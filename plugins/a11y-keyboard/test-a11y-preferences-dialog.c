/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2007 William Jon McCann <mccann@jhu.edu>
 * Copyright (C) 2012-2021 MATE Developers
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <stdlib.h>

#include "msd-a11y-preferences-dialog.h"

static void test_window(void) {
  GtkWidget *window;

  window = msd_a11y_preferences_dialog_new();
  gtk_dialog_run(GTK_DIALOG(window));
}

int main(int argc, char **argv) {
  GError *error = NULL;

#ifdef ENABLE_NLS
  bindtextdomain(GETTEXT_PACKAGE, MATE_SETTINGS_LOCALEDIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif
  textdomain(GETTEXT_PACKAGE);
#endif

  if (!gtk_init_with_args(&argc, &argv, NULL, NULL, NULL, &error)) {
    fprintf(stderr, "%s", error->message);
    g_error_free(error);
    exit(1);
  }

  test_window();

  return 0;
}
