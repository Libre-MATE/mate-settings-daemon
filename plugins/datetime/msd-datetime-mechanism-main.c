/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2007 David Zeuthen <david@fubar.dk>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib.h>
#include <errno.h>
#include <fcntl.h>
#include <glib-object.h>
#include <glib.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "msd-datetime-mechanism.h"

static DBusGProxy *get_bus_proxy(DBusGConnection *connection) {
  DBusGProxy *bus_proxy;

  bus_proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS,
                                        DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
  return bus_proxy;
}

#define BUS_NAME "org.mate.SettingsDaemon.DateTimeMechanism"

static gboolean acquire_name_on_proxy(DBusGProxy *bus_proxy) {
  GError *error;
  guint result;
  gboolean res;
  gboolean ret;

  ret = FALSE;

  if (bus_proxy == NULL) {
    goto out;
  }

  error = NULL;
  res = dbus_g_proxy_call(bus_proxy, "RequestName", &error, G_TYPE_STRING,
                          BUS_NAME, G_TYPE_UINT, 0, G_TYPE_INVALID, G_TYPE_UINT,
                          &result, G_TYPE_INVALID);
  if (!res) {
    if (error != NULL) {
      g_warning("Failed to acquire %s: %s", BUS_NAME, error->message);
      g_error_free(error);
    } else {
      g_warning("Failed to acquire %s", BUS_NAME);
    }
    goto out;
  }

  if (result != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
    if (error != NULL) {
      g_warning("Failed to acquire %s: %s", BUS_NAME, error->message);
      g_error_free(error);
    } else {
      g_warning("Failed to acquire %s", BUS_NAME);
    }
    goto out;
  }

  ret = TRUE;

out:
  return ret;
}

static DBusGConnection *get_system_bus(void) {
  GError *error;
  DBusGConnection *bus;

  error = NULL;
  bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
  if (bus == NULL) {
    g_warning("Couldn't connect to system bus: %s", error->message);
    g_error_free(error);
  }
  return bus;
}

int main(int argc, char **argv) {
  GMainLoop *loop;
  MsdDatetimeMechanism *mechanism;
  DBusGProxy *bus_proxy;
  DBusGConnection *connection;
  int ret;

  ret = 1;

  dbus_g_thread_init();

  connection = get_system_bus();
  if (connection == NULL) {
    goto out;
  }

  bus_proxy = get_bus_proxy(connection);
  if (bus_proxy == NULL) {
    g_warning("Could not construct bus_proxy object; bailing out");
    goto out;
  }

  if (!acquire_name_on_proxy(bus_proxy)) {
    g_warning("Could not acquire name; bailing out");
    goto out;
  }

  mechanism = msd_datetime_mechanism_new();

  if (mechanism == NULL) {
    goto out;
  }

  loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(loop);

  g_object_unref(mechanism);
  g_main_loop_unref(loop);
  ret = 0;

out:
  return ret;
}
