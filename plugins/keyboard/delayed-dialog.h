/*
 * Copyright © 2006 Novell, Inc.
 * Copyright (C) 2012-2021 MATE Developers
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef __DELAYED_DIALOG_H
#define __DELAYED_DIALOG_H

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

void msd_delayed_show_dialog(GtkWidget *dialog);

G_END_DECLS

#endif /* __DELAYED_DIALOG_H */
