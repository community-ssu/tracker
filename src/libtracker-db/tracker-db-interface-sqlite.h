/* Tracker - Sqlite implementation
 * Copyright (C) 2008 Nokia
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

#ifndef __TRACKER_DB_INTERFACE_SQLITE_H__
#define __TRACKER_DB_INTERFACE_SQLITE_H__

#include "tracker-db-interface.h"

G_BEGIN_DECLS

#define TRACKER_TYPE_DB_INTERFACE_SQLITE	 (tracker_db_interface_sqlite_get_type ())
#define TRACKER_DB_INTERFACE_SQLITE(o)		 (G_TYPE_CHECK_INSTANCE_CAST ((o), TRACKER_TYPE_DB_INTERFACE_SQLITE, TrackerDBInterfaceSqlite))
#define TRACKER_DB_INTERFACE_SQLITE_CLASS(c)	 (G_TYPE_CHECK_CLASS_CAST ((c),    TRACKER_TYPE_DB_INTERFACE_SQLITE, TrackerDBInterfaceSqliteClass))
#define TRACKER_IS_DB_INTERFACE_SQLITE(o)	 (G_TYPE_CHECK_INSTANCE_TYPE ((o), TRACKER_TYPE_DB_INTERFACE_SQLITE))
#define TRACKER_IS_DB_INTERFACE_SQLITE_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE ((o),    TRACKER_TYPE_DB_INTERFACE_SQLITE))
#define TRACKER_DB_INTERFACE_SQLITE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o),  TRACKER_TYPE_DB_INTERFACE_SQLITE, TrackerDBInterfaceSqliteClass))

typedef struct TrackerDBInterfaceSqlite      TrackerDBInterfaceSqlite;
typedef struct TrackerDBInterfaceSqliteClass TrackerDBInterfaceSqliteClass;

typedef gint (* TrackerDBCollationFunc) (gchar *str1,
					 gint	len1,
					 gchar *str2,
					 gint	len2);
typedef GValue (* TrackerDBFunc) (TrackerDBInterface *interface,
				  gint		      argc,
				  GValue	      argv[]);

typedef void (* TrackerDBFuncStep) (TrackerDBInterface *interface,
				    void               *aggregate_context,
				    gint		  argc,
				    GValue	          argv[]);

typedef GValue (* TrackerDBFuncFinal) (TrackerDBInterface *interface,
				       void               *aggregate_context);

struct TrackerDBInterfaceSqlite {
	GObject parent_instance;
};

struct TrackerDBInterfaceSqliteClass {
	GObjectClass parent_class;
};

GType tracker_db_interface_sqlite_get_type (void);

TrackerDBInterface * tracker_db_interface_sqlite_new (const gchar *filename);
TrackerDBInterface * tracker_db_interface_sqlite_new_ro (const gchar *filename);

void		     tracker_db_interface_sqlite_create_function	(TrackerDBInterface	  *interface,
									 const gchar		  *name,
									 TrackerDBFunc		   func,
									 gint			   n_args);
void                 tracker_db_interface_sqlite_create_aggregate       (TrackerDBInterface       *interface,
									 const gchar	          *name,
									 TrackerDBFuncStep         step,
									 gint		           n_args,
									 TrackerDBFuncFinal        final,
									 guint                     context_size);
gboolean	     tracker_db_interface_sqlite_set_collation_function (TrackerDBInterfaceSqlite *interface,
									 const gchar		  *name,
									 TrackerDBCollationFunc    func);

gint64		     tracker_db_interface_sqlite_get_last_insert_id	(TrackerDBInterfaceSqlite *interface);

void		     tracker_db_interface_sqlite_enable_shared_cache (void);

G_END_DECLS

#endif /* __TRACKER_DB_INTERFACE_SQLITE_H__ */
