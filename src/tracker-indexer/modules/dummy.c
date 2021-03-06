/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2006, Mr Jamie McCracken (jamiemcc@gnome.org)
 * Copyright (C) 2008, Nokia

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

#include "config.h"

#include <libtracker-data/tracker-data-metadata.h>

#include <tracker-indexer/tracker-module-file.h>
#include <tracker-indexer/tracker-module-iteratable.h>

#define DUMMY_TYPE_FILE    (dummy_file_get_type ())
#define DUMMY_FILE(module) (G_TYPE_CHECK_INSTANCE_CAST ((module), DUMMY_TYPE_FILE, DummyFile))

#define MODULE_IMPLEMENT_INTERFACE(TYPE_IFACE, iface_init)		   \
	{								   \
		const GInterfaceInfo g_implement_interface_info = {	   \
			(GInterfaceInitFunc) iface_init, NULL, NULL	   \
		};							   \
									   \
		g_type_module_add_interface (type_module,		   \
					     g_define_type_id,		   \
					     TYPE_IFACE,		   \
					     &g_implement_interface_info); \
	}

typedef struct DummyFile DummyFile;
typedef struct DummyFileClass DummyFileClass;

struct DummyFile {
        TrackerModuleFile parent_instance;
};

struct DummyFileClass {
        TrackerModuleFileClass parent_class;
};

static void          dummy_file_iteratable_init  (TrackerModuleIteratableIface *iface);

static void          dummy_file_finalize         (GObject           *object);

static void          dummy_file_initialize       (TrackerModuleFile *file);
static const gchar * dummy_file_get_service_type (TrackerModuleFile *file);
static gchar *       dummy_file_get_uri          (TrackerModuleFile *file);
static gchar *       dummy_file_get_text         (TrackerModuleFile *file);
static TrackerModuleMetadata *
                     dummy_file_get_metadata     (TrackerModuleFile *file);

static gboolean      dummy_file_iter_contents    (TrackerModuleIteratable *iteratable);
static guint         dummy_file_get_count        (TrackerModuleIteratable *iteratable);


G_DEFINE_DYNAMIC_TYPE_EXTENDED (DummyFile, dummy_file, TRACKER_TYPE_MODULE_FILE, 0,
                                MODULE_IMPLEMENT_INTERFACE (TRACKER_TYPE_MODULE_ITERATABLE,
                                                            dummy_file_iteratable_init))

static void
dummy_file_class_init (DummyFileClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        TrackerModuleFileClass *file_class = TRACKER_MODULE_FILE_CLASS (klass);

        object_class->finalize = dummy_file_finalize;

        file_class->initialize = dummy_file_initialize;
        file_class->get_service_type = dummy_file_get_service_type;
        file_class->get_uri = dummy_file_get_uri;
        file_class->get_text = dummy_file_get_text;
        file_class->get_metadata = dummy_file_get_metadata;
}

static void
dummy_file_class_finalize (DummyFileClass *klass)
{
}

static void
dummy_file_init (DummyFile *file)
{
}

static void
dummy_file_iteratable_init (TrackerModuleIteratableIface *iface)
{
        iface->iter_contents = dummy_file_iter_contents;
        iface->get_count = dummy_file_get_count;
}

static void
dummy_file_finalize (GObject *object)
{
        /* Free here all resources allocated by the object, if any */

        /* Chain up to parent implementation */
        G_OBJECT_CLASS (dummy_file_parent_class)->finalize (object);
}

static void
dummy_file_initialize (TrackerModuleFile *file)
{
        /* Allocate here all resources for the file, if any */
}

static const gchar *
dummy_file_get_service_type (TrackerModuleFile *file)
{
        /* Implementing this function is optional.
         *
         * Return the service type for the given file.
         *
         * If this function is not implemented, the indexer will use
         * whatever service name is specified in the module configuration
         * file.
         */
        return NULL;
}

static gchar *
dummy_file_get_uri (TrackerModuleFile *file)
{
        /* Implementing this function is optional
         *
         * Return URI for the current item, with this method
         * modules can specify different URIs for different
         * elements contained in the file. See also
         * TrackerModuleIteratable.
         */
        return NULL;
}

static gchar *
dummy_file_get_text (TrackerModuleFile *file)
{
	/* Implementing this function is optional
	 *
	 * Return here full text for file, given the current state,
	 * see also TrackerModuleIteratable.
	 */
	return NULL;
}

static TrackerModuleMetadata *
dummy_file_get_metadata (TrackerModuleFile *file)
{
	/* Return a TrackerModuleMetadata filled with metadata for file,
         * given the current state. Also see TrackerModuleIteratable.
	 */
	return NULL;
}

static gboolean
dummy_file_iter_contents (TrackerModuleIteratable *iteratable)
{
	/* This function is meant to iterate the internal state,
	 * so it points to the next entity inside the file.
	 * In case there is such next entity, this function must
	 * return TRUE, else, returning FALSE will make the indexer
	 * think it is done with this file and move on to the next one.
	 *
	 * What an "entity" is considered is left to the module
	 * implementation.
	 */
        return FALSE;
}

static guint
dummy_file_get_count (TrackerModuleIteratable *iteratable)
{
        /* This function is meant to return the number of entities
         * contained in the file, what an "entity" is considered is
         * left to the module implementation.
         */
        return 0;
}

void
indexer_module_initialize (GTypeModule *module)
{
        dummy_file_register_type (module);
}

void
indexer_module_shutdown (void)
{
}

TrackerModuleFile *
indexer_module_create_file (GFile *file)
{
        return g_object_new (DUMMY_TYPE_FILE,
                             "file", file,
                             NULL);
}
