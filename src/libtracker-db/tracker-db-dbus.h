/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2006, Mr Jamie McCracken (jamiemcc@gnome.org)
 * Copyright (C) 2008, Nokia
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef __LIBTRACKER_DB_DBUS_H__
#define __LIBTRACKER_DB_DBUS_H__

G_BEGIN_DECLS

#include <glib.h>

#include "tracker-db-interface.h"

gchar **    tracker_dbus_query_result_to_strv            (TrackerDBResultSet *result_set,
							  gint                column,
							  gint               *count);
gchar **    tracker_dbus_query_result_numeric_to_strv    (TrackerDBResultSet *result_set,
							  gint                column,
							  gint               *count);
gchar **    tracker_dbus_query_result_columns_to_strv    (TrackerDBResultSet *result_set,
							  gint                offset_column,
							  gint                until_column,
							  gboolean            rewind);
GHashTable *tracker_dbus_query_result_to_hash_table      (TrackerDBResultSet *result_set);
GPtrArray * tracker_dbus_query_result_to_ptr_array       (TrackerDBResultSet *result_set);
GPtrArray * tracker_dbus_query_result_multi_to_ptr_array (TrackerDBResultSet *result_set);

G_END_DECLS

#endif /* __LIBTRACKER_DB_DBUS_H__ */
