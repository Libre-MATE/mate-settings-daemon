/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2007 William Jon McCann <mccann@jhu.edu>
 * Copyright (C) 2010 Red Hat, Inc.
 * Copyright (C) 2012-2021 MATE Developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "msd-rfkill-plugin.h"

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include "mate-settings-plugin.h"
#include "msd-rfkill-manager.h"

struct _MsdRfkillPluginPrivate {
  MsdRfkillManager *manager;
};

MATE_SETTINGS_PLUGIN_REGISTER_WITH_PRIVATE(MsdRfkillPlugin, msd_rfkill_plugin)

static void msd_rfkill_plugin_init(MsdRfkillPlugin *plugin) {
  plugin->priv = msd_rfkill_plugin_get_instance_private(plugin);

  g_debug("MsdRfkillPlugin initializing");

  plugin->priv->manager = msd_rfkill_manager_new();
}

static void msd_rfkill_plugin_finalize(GObject *object) {
  MsdRfkillPlugin *plugin;

  g_return_if_fail(object != NULL);
  g_return_if_fail(MSD_IS_RFKILL_PLUGIN(object));

  g_debug("MsdRfkillPlugin finalizing");

  plugin = MSD_RFKILL_PLUGIN(object);

  g_return_if_fail(plugin->priv != NULL);

  if (plugin->priv->manager != NULL) {
    g_object_unref(plugin->priv->manager);
  }

  G_OBJECT_CLASS(msd_rfkill_plugin_parent_class)->finalize(object);
}

static void impl_activate(MateSettingsPlugin *plugin) {
  gboolean res;
  GError *error;

  g_debug("Activating rfkill plugin");

  error = NULL;
  res = msd_rfkill_manager_start(MSD_RFKILL_PLUGIN(plugin)->priv->manager,
                                 &error);
  if (!res) {
    g_warning("Unable to start rfkill manager: %s", error->message);
    g_error_free(error);
  }
}

static void impl_deactivate(MateSettingsPlugin *plugin) {
  g_debug("Deactivating rfkill plugin");
  msd_rfkill_manager_stop(MSD_RFKILL_PLUGIN(plugin)->priv->manager);
}

static void msd_rfkill_plugin_class_init(MsdRfkillPluginClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  MateSettingsPluginClass *plugin_class = MATE_SETTINGS_PLUGIN_CLASS(klass);

  object_class->finalize = msd_rfkill_plugin_finalize;

  plugin_class->activate = impl_activate;
  plugin_class->deactivate = impl_deactivate;
}

static void msd_rfkill_plugin_class_finalize(MsdRfkillPluginClass *klass) {}
