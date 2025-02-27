/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2007 William Jon McCann <mccann@jhu.edu>
 * Copyright (C) 2007 Red Hat, Inc.
 * Copyright (C) 2012 Jasmine Hassan <jasmine.aura@gmail.com>
 * Copyright (C) 2012-2021 MATE Developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MATE_DESKTOP_USE_UNSTABLE_API
#include <X11/Xatom.h>
#include <libmate-desktop/mate-bg.h>

#include "mate-settings-profile.h"
#include "msd-background-manager.h"

#define MATE_SESSION_MANAGER_DBUS_NAME "org.gnome.SessionManager"
#define MATE_SESSION_MANAGER_DBUS_PATH "/org/gnome/SessionManager"

struct _MsdBackgroundManager {
  GObject parent;

  GSettings *settings;
  MateBG *bg;
  cairo_surface_t *surface;
  MateBGCrossfade *fade;
  GList *scr_sizes;

  gboolean msd_can_draw;
  gboolean caja_can_draw;
  gboolean do_fade;
  gboolean draw_in_progress;

  guint timeout_id;

  GDBusProxy *proxy;
  gulong proxy_signal_id;
};

G_DEFINE_TYPE(MsdBackgroundManager, msd_background_manager, G_TYPE_OBJECT)

static gpointer manager_object = NULL;

/* Whether MSD is allowed to draw background */
static gboolean msd_can_draw_bg(MsdBackgroundManager *manager) {
  return g_settings_get_boolean(manager->settings, MATE_BG_KEY_DRAW_BACKGROUND);
}

/* Whether to change background with a fade effect */
static gboolean can_fade_bg(MsdBackgroundManager *manager) {
  return g_settings_get_boolean(manager->settings, MATE_BG_KEY_BACKGROUND_FADE);
}

/* Whether Caja is configured to draw desktop (show-desktop-icons) */
static gboolean caja_can_draw_bg(MsdBackgroundManager *manager) {
  return g_settings_get_boolean(manager->settings, MATE_BG_KEY_SHOW_DESKTOP);
}

static gboolean caja_is_drawing_bg(MsdBackgroundManager *manager) {
  Display *display = gdk_x11_get_default_xdisplay();
  Window window = gdk_x11_get_default_root_xwindow();
  Atom caja_prop, wmclass_prop, type;
  Window caja_window;
  int format;
  unsigned long nitems, after;
  unsigned char *data;
  GdkDisplay *gdk_display;
  gboolean running = FALSE;

  if (!manager->caja_can_draw) return FALSE;

  caja_prop = XInternAtom(display, "CAJA_DESKTOP_WINDOW_ID", True);
  if (caja_prop == None) return FALSE;

  XGetWindowProperty(display, window, caja_prop, 0, 1, False, XA_WINDOW, &type,
                     &format, &nitems, &after, &data);

  if (data == NULL) return FALSE;

  caja_window = *(Window *)data;
  XFree(data);

  if (type != XA_WINDOW || format != 32) return FALSE;

  wmclass_prop = XInternAtom(display, "WM_CLASS", True);
  if (wmclass_prop == None) return FALSE;

  gdk_display = gdk_display_get_default();
  gdk_x11_display_error_trap_push(gdk_display);

  XGetWindowProperty(display, caja_window, wmclass_prop, 0, 20, False,
                     XA_STRING, &type, &format, &nitems, &after, &data);

  XSync(display, False);

  if (gdk_x11_display_error_trap_pop(gdk_display) == BadWindow || data == NULL)
    return FALSE;

  /* See: caja_desktop_window_new(), in src/caja-desktop-window.c */
  if (nitems == 20 && after == 0 && format == 8 &&
      !strcmp((char *)data, "desktop_window") &&
      !strcmp((char *)data + strlen((char *)data) + 1, "Caja")) {
    running = TRUE;
  }
  XFree(data);

  return running;
}

static void free_fade(MsdBackgroundManager *manager) {
  if (manager->fade != NULL) {
    g_object_unref(manager->fade);
    manager->fade = NULL;
  }
}

static void free_bg_surface(MsdBackgroundManager *manager) {
  if (manager->surface != NULL) {
    cairo_surface_destroy(manager->surface);
    manager->surface = NULL;
  }
}

static void free_scr_sizes(MsdBackgroundManager *manager) {
  if (manager->scr_sizes != NULL) {
    g_list_free_full(manager->scr_sizes, g_free);
    manager->scr_sizes = NULL;
  }
}

static void real_draw_bg(MsdBackgroundManager *manager, GdkScreen *screen) {
  GdkWindow *window = gdk_screen_get_root_window(screen);
  gint scale = gdk_window_get_scale_factor(window);
  gint width = WidthOfScreen(gdk_x11_screen_get_xscreen(screen)) / scale;
  gint height = HeightOfScreen(gdk_x11_screen_get_xscreen(screen)) / scale;

  free_bg_surface(manager);
  manager->surface = mate_bg_create_surface_scale(manager->bg, window, width,
                                                  height, scale, TRUE);

  if (manager->do_fade) {
    free_fade(manager);
    manager->fade =
        mate_bg_set_surface_as_root_with_crossfade(screen, manager->surface);
    g_signal_connect_swapped(manager->fade, "finished", G_CALLBACK(free_fade),
                             manager);
  } else {
    mate_bg_set_surface_as_root(screen, manager->surface);
  }
  manager->scr_sizes = g_list_prepend(manager->scr_sizes,
                                      g_strdup_printf("%dx%d", width, height));
}

static void draw_background(MsdBackgroundManager *manager, gboolean may_fade) {
  if (!manager->msd_can_draw || manager->draw_in_progress ||
      caja_is_drawing_bg(manager))
    return;

  mate_settings_profile_start(NULL);

  GdkDisplay *display = gdk_display_get_default();

  manager->draw_in_progress = TRUE;
  manager->do_fade = may_fade && can_fade_bg(manager);
  free_scr_sizes(manager);

  g_debug("Drawing background on Screen");
  real_draw_bg(manager, gdk_display_get_default_screen(display));

  manager->scr_sizes = g_list_reverse(manager->scr_sizes);

  manager->draw_in_progress = FALSE;
  mate_settings_profile_end(NULL);
}

static void on_bg_changed(MateBG *bg, MsdBackgroundManager *manager) {
  g_debug("Background changed");
  draw_background(manager, TRUE);
}

static void on_bg_transitioned(MateBG *bg, MsdBackgroundManager *manager) {
  g_debug("Background transitioned");
  draw_background(manager, FALSE);
}

static void on_screen_size_changed(GdkScreen *screen,
                                   MsdBackgroundManager *manager) {
  if (!manager->msd_can_draw || manager->draw_in_progress ||
      caja_is_drawing_bg(manager))
    return;

  GdkWindow *window = gdk_screen_get_root_window(screen);
  gint scale = gdk_window_get_scale_factor(window);
  gint scr_num = gdk_x11_screen_get_screen_number(screen);
  gchar *old_size = g_list_nth_data(manager->scr_sizes, (guint)scr_num);
  gchar *new_size = g_strdup_printf(
      "%dx%d", WidthOfScreen(gdk_x11_screen_get_xscreen(screen)) / scale,
      HeightOfScreen(gdk_x11_screen_get_xscreen(screen)) / scale);
  if (g_strcmp0(old_size, new_size) != 0) {
    g_debug("Screen%d size changed: %s -> %s", scr_num, old_size, new_size);
    draw_background(manager, FALSE);
  } else {
    g_debug("Screen%d size unchanged (%s). Ignoring.", scr_num, old_size);
  }
  g_free(new_size);
}

static void disconnect_screen_signals(MsdBackgroundManager *manager) {
  GdkDisplay *display = gdk_display_get_default();

  g_signal_handlers_disconnect_by_func(gdk_display_get_default_screen(display),
                                       G_CALLBACK(on_screen_size_changed),
                                       manager);
}

static void connect_screen_signals(MsdBackgroundManager *manager) {
  GdkDisplay *display = gdk_display_get_default();

  GdkScreen *screen = gdk_display_get_default_screen(display);

  g_signal_connect(screen, "monitors-changed",
                   G_CALLBACK(on_screen_size_changed), manager);
  g_signal_connect(screen, "size-changed", G_CALLBACK(on_screen_size_changed),
                   manager);
}

static gboolean settings_change_event_idle_cb(MsdBackgroundManager *manager) {
  mate_settings_profile_start("settings_change_event_idle_cb");

  mate_bg_load_from_preferences(manager->bg);

  mate_settings_profile_end("settings_change_event_idle_cb");

  return FALSE; /* remove from the list of event sources */
}

static gboolean settings_change_event_cb(GSettings *settings, gpointer keys,
                                         gint n_keys,
                                         MsdBackgroundManager *manager) {
  /* Complements on_bg_handling_changed() */
  manager->msd_can_draw = msd_can_draw_bg(manager);
  manager->caja_can_draw = caja_can_draw_bg(manager);

  if (manager->msd_can_draw && manager->bg != NULL &&
      !caja_is_drawing_bg(manager)) {
    /* Defer signal processing to avoid making the dconf backend deadlock */
    g_idle_add((GSourceFunc)settings_change_event_idle_cb, manager);
  }

  return FALSE; /* let the event propagate further */
}

static void setup_background(MsdBackgroundManager *manager) {
  g_return_if_fail(manager->bg == NULL);

  manager->bg = mate_bg_new();

  manager->draw_in_progress = FALSE;

  g_signal_connect(manager->bg, "changed", G_CALLBACK(on_bg_changed), manager);

  g_signal_connect(manager->bg, "transitioned", G_CALLBACK(on_bg_transitioned),
                   manager);

  mate_bg_load_from_gsettings(manager->bg, manager->settings);

  connect_screen_signals(manager);

  g_signal_connect(manager->settings, "change-event",
                   G_CALLBACK(settings_change_event_cb), manager);
}

static void remove_background(MsdBackgroundManager *manager) {
  disconnect_screen_signals(manager);

  g_signal_handlers_disconnect_by_func(manager->settings,
                                       settings_change_event_cb, manager);

  if (manager->settings != NULL) {
    g_object_unref(manager->settings);
    manager->settings = NULL;
  }

  if (manager->bg != NULL) {
    g_object_unref(manager->bg);
    manager->bg = NULL;
  }

  free_scr_sizes(manager);
  free_bg_surface(manager);
  free_fade(manager);
}

static void on_bg_handling_changed(GSettings *settings, const char *key,
                                   MsdBackgroundManager *manager) {
  mate_settings_profile_start(NULL);

  if (caja_is_drawing_bg(manager)) {
    if (manager->bg != NULL) remove_background(manager);
  } else if (manager->msd_can_draw && manager->bg == NULL) {
    setup_background(manager);
  }

  mate_settings_profile_end(NULL);
}

static gboolean queue_setup_background(MsdBackgroundManager *manager) {
  manager->timeout_id = 0;

  setup_background(manager);

  return FALSE;
}

static void queue_timeout(MsdBackgroundManager *manager) {
  if (manager->timeout_id > 0) return;

  /* SessionRunning: now check if Caja is drawing background, and if not, set
   * it.
   *
   * FIXME: We wait a few seconds after the session is up because Caja tells the
   * session manager that its ready before it sets the background.
   * https://bugzilla.gnome.org/show_bug.cgi?id=568588
   */
  manager->timeout_id =
      g_timeout_add_seconds(8, (GSourceFunc)queue_setup_background, manager);
}

static void disconnect_session_manager_listener(MsdBackgroundManager *manager) {
  if (manager->proxy) {
    g_clear_signal_handler(&manager->proxy_signal_id, manager->proxy);
  }
}

static void on_session_manager_signal(GDBusProxy *proxy,
                                      const gchar *sender_name,
                                      const gchar *signal_name,
                                      GVariant *parameters,
                                      gpointer user_data) {
  MsdBackgroundManager *manager = MSD_BACKGROUND_MANAGER(user_data);

  if (g_strcmp0(signal_name, "SessionRunning") == 0) {
    queue_timeout(manager);
    disconnect_session_manager_listener(manager);
  }
}

static void draw_bg_after_session_loads(MsdBackgroundManager *manager) {
  GError *error = NULL;
  GDBusProxyFlags flags;

  flags = G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
          G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START;
  manager->proxy = g_dbus_proxy_new_for_bus_sync(
      G_BUS_TYPE_SESSION, flags, NULL, /* GDBusInterfaceInfo */
      MATE_SESSION_MANAGER_DBUS_NAME, MATE_SESSION_MANAGER_DBUS_PATH,
      MATE_SESSION_MANAGER_DBUS_NAME, NULL, /* GCancellable */
      &error);
  if (manager->proxy == NULL) {
    g_warning("Could not listen to session manager: %s", error->message);
    g_error_free(error);
    return;
  }

  manager->proxy_signal_id =
      g_signal_connect(manager->proxy, "g-signal",
                       G_CALLBACK(on_session_manager_signal), manager);
}

gboolean msd_background_manager_start(MsdBackgroundManager *manager,
                                      GError **error) {
  g_debug("Starting background manager");
  mate_settings_profile_start(NULL);

  manager->settings = g_settings_new(MATE_BG_SCHEMA);

  manager->msd_can_draw = msd_can_draw_bg(manager);
  manager->caja_can_draw = caja_can_draw_bg(manager);

  g_signal_connect(manager->settings, "changed::" MATE_BG_KEY_DRAW_BACKGROUND,
                   G_CALLBACK(on_bg_handling_changed), manager);
  g_signal_connect(manager->settings, "changed::" MATE_BG_KEY_SHOW_DESKTOP,
                   G_CALLBACK(on_bg_handling_changed), manager);

  /* If Caja is set to draw the background, it is very likely in our session.
   * But it might not be started yet, so caja_is_drawing_bg() would fail.
   * In this case, we wait till the session is loaded, to avoid double-draws.
   */
  if (manager->msd_can_draw) {
    if (manager->caja_can_draw)
      draw_bg_after_session_loads(manager);
    else
      setup_background(manager);
  }

  mate_settings_profile_end(NULL);

  return TRUE;
}

void msd_background_manager_stop(MsdBackgroundManager *manager) {
  g_debug("Stopping background manager");

  if (manager->proxy) {
    disconnect_session_manager_listener(manager);
    g_object_unref(manager->proxy);
  }

  if (manager->timeout_id != 0) {
    g_source_remove(manager->timeout_id);
    manager->timeout_id = 0;
  }

  remove_background(manager);
}

static GObject *msd_background_manager_constructor(
    GType type, guint n_construct_properties,
    GObjectConstructParam *construct_properties) {
  MsdBackgroundManager *manager = MSD_BACKGROUND_MANAGER(
      G_OBJECT_CLASS(msd_background_manager_parent_class)
          ->constructor(type, n_construct_properties, construct_properties));

  return G_OBJECT(manager);
}

static void msd_background_manager_finalize(GObject *object) {
  g_return_if_fail(object != NULL);
  g_return_if_fail(MSD_IS_BACKGROUND_MANAGER(object));

  G_OBJECT_CLASS(msd_background_manager_parent_class)->finalize(object);
}

static void msd_background_manager_init(MsdBackgroundManager *manager) {}

static void msd_background_manager_class_init(
    MsdBackgroundManagerClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->constructor = msd_background_manager_constructor;
  object_class->finalize = msd_background_manager_finalize;
}

MsdBackgroundManager *msd_background_manager_new(void) {
  if (manager_object != NULL) {
    g_object_ref(manager_object);
  } else {
    manager_object = g_object_new(MSD_TYPE_BACKGROUND_MANAGER, NULL);

    g_object_add_weak_pointer(manager_object, (gpointer *)&manager_object);
  }

  return MSD_BACKGROUND_MANAGER(manager_object);
}
