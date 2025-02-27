/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2007 William Jon McCann <mccann@jhu.edu>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "msd-a11y-settings-plugin.h"

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include "mate-settings-plugin.h"
#include "msd-a11y-settings-manager.h"

struct MsdA11ySettingsPluginPrivate {
  MsdA11ySettingsManager *manager;
};

MATE_SETTINGS_PLUGIN_REGISTER_WITH_PRIVATE(MsdA11ySettingsPlugin,
                                           msd_a11y_settings_plugin)

static void msd_a11y_settings_plugin_init(MsdA11ySettingsPlugin *plugin) {
  plugin->priv = msd_a11y_settings_plugin_get_instance_private(plugin);

  g_debug("MsdA11ySettingsPlugin initializing");

  plugin->priv->manager = msd_a11y_settings_manager_new();
}

static void msd_a11y_settings_plugin_finalize(GObject *object) {
  MsdA11ySettingsPlugin *plugin;

  g_return_if_fail(object != NULL);
  g_return_if_fail(MSD_IS_A11Y_SETTINGS_PLUGIN(object));

  g_debug("MsdA11ySettingsPlugin finalizing");

  plugin = MSD_A11Y_SETTINGS_PLUGIN(object);

  g_return_if_fail(plugin->priv != NULL);

  if (plugin->priv->manager != NULL) {
    g_object_unref(plugin->priv->manager);
  }

  G_OBJECT_CLASS(msd_a11y_settings_plugin_parent_class)->finalize(object);
}

static void impl_activate(MateSettingsPlugin *plugin) {
  gboolean res;
  GError *error;

  g_debug("Activating a11y-settings plugin");

  error = NULL;
  res = msd_a11y_settings_manager_start(
      MSD_A11Y_SETTINGS_PLUGIN(plugin)->priv->manager, &error);
  if (!res) {
    g_warning("Unable to start a11y-settings manager: %s", error->message);
    g_error_free(error);
  }
}

static void impl_deactivate(MateSettingsPlugin *plugin) {
  g_debug("Deactivating a11y-settings plugin");
  msd_a11y_settings_manager_stop(
      MSD_A11Y_SETTINGS_PLUGIN(plugin)->priv->manager);
}

static void msd_a11y_settings_plugin_class_init(
    MsdA11ySettingsPluginClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  MateSettingsPluginClass *plugin_class = MATE_SETTINGS_PLUGIN_CLASS(klass);

  object_class->finalize = msd_a11y_settings_plugin_finalize;

  plugin_class->activate = impl_activate;
  plugin_class->deactivate = impl_deactivate;
}

static void msd_a11y_settings_plugin_class_finalize(
    MsdA11ySettingsPluginClass *klass) {}
