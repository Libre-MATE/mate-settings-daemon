/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2005 William Jon McCann <mccann@jhu.edu>
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
 * Authors: William Jon McCann <mccann@jhu.edu>
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "mate-settings-profile.h"

#include <glib.h>
#include <glib/gstdio.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void _mate_settings_profile_log(const char *func, const char *note,
                                const char *format, ...) {
  char *str;
  char *formatted;

  if (format == NULL) {
    formatted = g_strdup("");
  } else {
    va_list args;

    va_start(args, format);
    formatted = g_strdup_vprintf(format, args);
    va_end(args);
  }

  if (func != NULL) {
    str = g_strdup_printf("MARK: %s %s: %s %s", g_get_prgname(), func,
                          note ? note : "", formatted);
  } else {
    str = g_strdup_printf("MARK: %s: %s %s", g_get_prgname(), note ? note : "",
                          formatted);
  }

  g_free(formatted);

  g_access(str, F_OK);
  g_free(str);
}
