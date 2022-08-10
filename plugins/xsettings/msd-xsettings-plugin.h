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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef __MATE_XSETTINGS_PLUGIN_H__
#define __MATE_XSETTINGS_PLUGIN_H__

#include <glib-object.h>
#include <glib.h>
#include <gmodule.h>

#include "mate-settings-plugin.h"

G_BEGIN_DECLS

#define MATE_TYPE_XSETTINGS_PLUGIN (mate_xsettings_plugin_get_type())
#define MATE_XSETTINGS_PLUGIN(o)                               \
  (G_TYPE_CHECK_INSTANCE_CAST((o), MATE_TYPE_XSETTINGS_PLUGIN, \
                              MateXSettingsPlugin))
#define MATE_XSETTINGS_PLUGIN_CLASS(k)                      \
  (G_TYPE_CHECK_CLASS_CAST((k), MATE_TYPE_XSETTINGS_PLUGIN, \
                           MateXSettingsPluginClass))
#define MATE_IS_XSETTINGS_PLUGIN(o) \
  (G_TYPE_CHECK_INSTANCE_TYPE((o), MATE_TYPE_XSETTINGS_PLUGIN))
#define MATE_IS_XSETTINGS_PLUGIN_CLASS(k) \
  (G_TYPE_CHECK_CLASS_TYPE((k), MATE_TYPE_XSETTINGS_PLUGIN))
#define MATE_XSETTINGS_PLUGIN_GET_CLASS(o)                    \
  (G_TYPE_INSTANCE_GET_CLASS((o), MATE_TYPE_XSETTINGS_PLUGIN, \
                             MateXSettingsPluginClass))

typedef struct MateXSettingsPluginPrivate MateXSettingsPluginPrivate;

typedef struct {
  MateSettingsPlugin parent;
  MateXSettingsPluginPrivate *priv;
} MateXSettingsPlugin;

typedef struct {
  MateSettingsPluginClass parent_class;
} MateXSettingsPluginClass;

GType mate_xsettings_plugin_get_type(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_mate_settings_plugin(GTypeModule *module);

G_END_DECLS

#endif /* __MATE_XSETTINGS_PLUGIN_H__ */
