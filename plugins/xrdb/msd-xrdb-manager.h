/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2007 William Jon McCann <mccann@jhu.edu>
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
 *
 */

#ifndef __MSD_XRDB_MANAGER_H
#define __MSD_XRDB_MANAGER_H

#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define MSD_TYPE_XRDB_MANAGER (msd_xrdb_manager_get_type())
#define MSD_XRDB_MANAGER(o) \
  (G_TYPE_CHECK_INSTANCE_CAST((o), MSD_TYPE_XRDB_MANAGER, MsdXrdbManager))
#define MSD_XRDB_MANAGER_CLASS(k) \
  (G_TYPE_CHECK_CLASS_CAST((k), MSD_TYPE_XRDB_MANAGER, MsdXrdbManagerClass))
#define MSD_IS_XRDB_MANAGER(o) \
  (G_TYPE_CHECK_INSTANCE_TYPE((o), MSD_TYPE_XRDB_MANAGER))
#define MSD_IS_XRDB_MANAGER_CLASS(k) \
  (G_TYPE_CHECK_CLASS_TYPE((k), MSD_TYPE_XRDB_MANAGER))
#define MSD_XRDB_MANAGER_GET_CLASS(o) \
  (G_TYPE_INSTANCE_GET_CLASS((o), MSD_TYPE_XRDB_MANAGER, MsdXrdbManagerClass))

typedef struct MsdXrdbManagerPrivate MsdXrdbManagerPrivate;

typedef struct {
  GObject parent;
  MsdXrdbManagerPrivate *priv;
} MsdXrdbManager;

typedef struct {
  GObjectClass parent_class;
} MsdXrdbManagerClass;

GType msd_xrdb_manager_get_type(void);

MsdXrdbManager *msd_xrdb_manager_new(void);
gboolean msd_xrdb_manager_start(MsdXrdbManager *manager, GError **error);
void msd_xrdb_manager_stop(MsdXrdbManager *manager);

G_END_DECLS

#endif /* __MSD_XRDB_MANAGER_H */
