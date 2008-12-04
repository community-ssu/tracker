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

#include <string.h>

#include <gio/gio.h>

#include <libtracker-common/tracker-file-utils.h>
#include <libtracker-common/tracker-type-utils.h>
#include <libtracker-common/tracker-os-dependant.h>
#include <libtracker-common/tracker-ontology.h>
#include <libtracker-common/tracker-thumbnailer.h>

#include "tracker-module-metadata-utils.h"

#define METADATA_FILE_NAME_DELIMITED "File:NameDelimited"
#define METADATA_FILE_EXT	     "File:Ext"
#define METADATA_FILE_PATH	     "File:Path"
#define METADATA_FILE_NAME	     "File:Name"
#define METADATA_FILE_LINK	     "File:Link"
#define METADATA_FILE_MIMETYPE	     "File:Mime"
#define METADATA_FILE_SIZE	     "File:Size"
#define METADATA_FILE_MODIFIED	     "File:Modified"
#define METADATA_FILE_ACCESSED	     "File:Accessed"

#undef	TRY_LOCALE_TO_UTF8_CONVERSION
#define TEXT_MAX_SIZE		     1048576  /* bytes */
#define TEXT_CHECK_SIZE		     65535    /* bytes */

typedef struct {
	GPid pid;
	guint stdout_watch_id;
	GIOChannel *stdin_channel;
	GIOChannel *stdout_channel;
	GMainLoop  *data_incoming_loop;
	gpointer data;
} ProcessContext;

static ProcessContext *metadata_context = NULL;

static void
process_context_invalidate (ProcessContext *context)
{
	if (context->stdin_channel) {
		g_io_channel_shutdown (context->stdin_channel, FALSE, NULL);
		g_io_channel_unref (context->stdin_channel);
		context->stdin_channel = NULL;
	}

	if (context->stdout_watch_id != 0) {
		g_source_remove (context->stdout_watch_id);
		context->stdout_watch_id = 0;
	}

	if (context->stdout_channel) {
		g_io_channel_shutdown (context->stdout_channel, FALSE, NULL);
		g_io_channel_unref (context->stdout_channel);
		context->stdout_channel = NULL;
	}

	if (context->data_incoming_loop) {
		if (g_main_loop_is_running (context->data_incoming_loop)) {
			g_main_loop_quit (context->data_incoming_loop);
		}

		g_main_loop_unref (context->data_incoming_loop);
		context->data_incoming_loop = NULL;
	}

	if (context->pid != 0) {
		g_spawn_close_pid (context->pid);
		context->pid = 0;
	}
}

static void
process_context_destroy (ProcessContext *context)
{
	process_context_invalidate (context);
	g_free (context);
}

static void
process_context_child_watch_cb (GPid	 pid,
				gint	 status,
				gpointer user_data)
{
	ProcessContext *context;

	g_debug ("Process '%d' exited with code %d",
		 pid,
		 status);

	context = (ProcessContext *) user_data;

	if (context == metadata_context) {
		metadata_context = NULL;
	}

	process_context_destroy (context);
}

static ProcessContext *
process_context_create (const gchar **argv,
			GIOFunc       stdout_watch_func)
{
	ProcessContext *context;
	GIOChannel *stdin_channel, *stdout_channel;
	GIOFlags flags;
	GPid pid;

	if (!tracker_spawn_async_with_channels (argv,
						10,
						&pid,
						&stdin_channel,
						&stdout_channel,
						NULL)) {
		return NULL;
	}

	g_debug ("Process '%d' spawned for command:'%s'",
		 pid,
		 argv[0]);

	context = g_new0 (ProcessContext, 1);
	context->pid = pid;
	context->stdin_channel = stdin_channel;
	context->stdout_channel = stdout_channel;
	context->data_incoming_loop = g_main_loop_new (NULL, FALSE);
	context->stdout_watch_id = g_io_add_watch (stdout_channel,
						   G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP,
						   stdout_watch_func,
						   context);

	flags = g_io_channel_get_flags (context->stdout_channel);
	flags |= G_IO_FLAG_NONBLOCK;

	g_io_channel_set_flags (context->stdout_channel, flags, NULL);

	g_child_watch_add (context->pid, process_context_child_watch_cb, context);

	return context;
}

static gboolean
metadata_read_cb (GIOChannel   *channel,
		  GIOCondition	condition,
		  gpointer	user_data)
{
	GPtrArray *array;
	GIOStatus status;
	gchar *line;

	if (!metadata_context) {
		return FALSE;
	}

	if ((condition & G_IO_HUP) || (condition & G_IO_ERR)) {
		return FALSE;
	}

	array = metadata_context->data;
	status = G_IO_STATUS_NORMAL;
	line = NULL;

	if (!array) {
		/* FIXME: What do we do here? This has happened to me
		 * before and we get warnings when we try to add to
		 * the empty array later.
		 */
		g_message ("EEEEEK!!!\n"
			   "\n"
			   "Expected metadata array to be non-NULL!\n"
			   "\n"
			   "This usually means we probably got '\\n' too many times "
			   "and closed the pipe when there is more content available "
			   "to read\n"
			   "\n"
			   "Stopping main loop and this callback");

		g_main_loop_quit (metadata_context->data_incoming_loop);

		return FALSE;
	}

	if ((condition & G_IO_IN) || (condition & G_IO_PRI)) {
		do {
			status = g_io_channel_read_line (metadata_context->stdout_channel,
							 &line,
							 NULL,
							 NULL,
							 NULL);

			if (status == G_IO_STATUS_NORMAL && line && *line) {
				g_strstrip (line);
				g_strdelimit (line, ";", '\0');
				g_ptr_array_add (array, line);
			}
		} while (status == G_IO_STATUS_NORMAL && line && *line);

		if (status == G_IO_STATUS_EOF ||
		    status == G_IO_STATUS_ERROR ||
		    (status == G_IO_STATUS_NORMAL && !*line)) {
			/* All extractor output has been processed */
			g_main_loop_quit (metadata_context->data_incoming_loop);
		}
	}

	return TRUE;
}

static gboolean
metadata_setup (void)
{
	const gchar *argv[2] = {
		LIBEXEC_PATH G_DIR_SEPARATOR_S "tracker-extract",
		NULL
	};

	if (metadata_context) {
		process_context_invalidate (metadata_context);
		metadata_context = NULL;
	}

	metadata_context = process_context_create (argv,
						   metadata_read_cb);
	if (!metadata_context) {
		return FALSE;
	}

	return TRUE;
}

static gchar **
metadata_query_file (const gchar *path,
		     const gchar *mimetype)
{
	gchar *utf_path, *str;
	GPtrArray *array;
	GIOStatus status;
	GError *error = NULL;

	if (!path || !mimetype) {
		return NULL;
	}

	if (!metadata_context && !metadata_setup ()) {
		return NULL;
	}

	utf_path = g_filename_from_utf8 (path, -1, NULL, NULL, NULL);

	if (!utf_path) {
		return NULL;
	}

	array = g_ptr_array_sized_new (10);
	metadata_context->data = array;

	str = g_strdup_printf ("%s\n%s\n", utf_path, mimetype);
	g_free (utf_path);

	/* Write path and mimetype */
	g_io_channel_write_chars (metadata_context->stdin_channel, str, -1, NULL, NULL);
	status = g_io_channel_flush (metadata_context->stdin_channel, &error);

	if (status == G_IO_STATUS_ERROR &&
	    error &&
	    g_error_matches (error, G_IO_CHANNEL_ERROR, G_IO_CHANNEL_ERROR_PIPE)) {
		/* Looks like the external extractor
		 * process has died before the child watch
		 * could handle it, try respawning.
		 */
		g_error_free (error);

		metadata_setup ();
		metadata_context->data = array;

		g_io_channel_write_chars (metadata_context->stdin_channel, str, -1, NULL, NULL);
		status = g_io_channel_flush (metadata_context->stdin_channel, NULL);

		if (status == G_IO_STATUS_ERROR) {
			/* No point in trying again */
			process_context_invalidate (metadata_context);
			metadata_context = NULL;
			g_free (str);
			return NULL;
		}
	}

	/* It will block here until all incoming
	 * metadata has been processed
	 */
	g_main_loop_run (metadata_context->data_incoming_loop);

	g_ptr_array_add (array, NULL);

	if (metadata_context) {
		metadata_context->data = NULL;
	}

	g_free (str);

	return (gchar **) g_ptr_array_free (array, FALSE);
}

static void
metadata_utils_get_embedded (const char            *path,
			     const char            *mime_type,
			     TrackerModuleMetadata *metadata)
{
	gchar **values;
	const gchar *service_type;
	gint i;
	TrackerField *field;

	service_type = tracker_ontology_get_service_by_mime (mime_type);
	if (!service_type) {
		return;
	}

	if (!tracker_ontology_service_has_metadata (service_type)) {
		return;
	}

	values = metadata_query_file (path, mime_type);

	if (!values) {
		return;
	}

	/* parse returned values and extract keys and associated metadata */
	for (i = 0; values[i]; i++) {
		gchar *meta_data, *sep;
		const gchar *name, *value;
		gchar *utf_value;

		meta_data = values[i];
		sep = strchr (meta_data, '=');

		if (!sep)
			continue;

		/* zero out the separator, so we get
		 * NULL-terminated name and value
		 */
		sep[0] = '\0';
		name = meta_data;
		value = sep + 1;

		if (!name || !value)
			continue;
		
		field = tracker_ontology_get_field_by_name (name);

		if (!field) {
			g_warning ("Field name '%s' isn't described in the ontology", name);
			continue;
		}
		
		if (tracker_field_get_multiple_values (field)) {
			GStrv arr;
			guint i;

			arr = g_strsplit (value, "|",-1);

			for (i = 0; arr[i]; i++) {
				if (!g_utf8_validate (arr[i], -1, NULL)) {
					utf_value = g_locale_to_utf8 (arr[i], -1, NULL, NULL, NULL);
				} else {
					utf_value = g_strdup (arr[i]);
				}

				if (!utf_value)
					continue;

				tracker_module_metadata_add_string (metadata, name, utf_value);
				g_free (utf_value);
			}

			g_strfreev (arr);
		} else {
			if (!g_utf8_validate (value, -1, NULL)) {
				utf_value = g_locale_to_utf8 (value, -1, NULL, NULL, NULL);
			} else {
				utf_value = g_strdup (value);
			}

			if (!utf_value) {
				continue;
			}

			tracker_module_metadata_add_string (metadata, name, utf_value);
			g_free (utf_value);
		}
	}

	g_strfreev (values);
}

static gboolean
get_file_content_read_cb (GIOChannel   *channel,
			  GIOCondition	condition,
			  gpointer	user_data)
{
	ProcessContext *context;
	GString *text;
	GIOStatus status;
	gchar *line;

	context = user_data;
	text = context->data;;
	status = G_IO_STATUS_NORMAL;

	if (condition & G_IO_IN || condition & G_IO_PRI) {
		do {
			GError *error = NULL;

			status = g_io_channel_read_line (channel, &line, NULL, NULL, &error);

			if (status == G_IO_STATUS_NORMAL) {
				g_string_append (text, line);
				g_free (line);
			} else if (error) {
				g_warning ("%s", error->message);
				g_error_free (error);
			}
		} while (status == G_IO_STATUS_NORMAL);

		if (status == G_IO_STATUS_EOF ||
		    status == G_IO_STATUS_ERROR) {
			g_main_loop_quit (context->data_incoming_loop);
			return FALSE;
		}
	}

	if (condition & G_IO_ERR || condition & G_IO_HUP) {
		g_main_loop_quit (context->data_incoming_loop);
		return FALSE;
	}

	return TRUE;
}

static gboolean
get_file_is_utf8 (GString *s,
		  gssize  *bytes_valid)
{
	const gchar *end;

	/* Check for UTF-8 validity, since we may
	 * have cut off the end.
	 */
	if (g_utf8_validate (s->str, s->len, &end)) {
		*bytes_valid = (gssize) s->len;
		return TRUE;
	}

	*bytes_valid = end - s->str;

	/* 4 is the maximum bytes for a UTF-8 character. */
	if (*bytes_valid > 4) {
		return FALSE;
	}

	if (g_utf8_get_char_validated (end, *bytes_valid) == (gunichar) -1) {
		return FALSE;
	}

	return TRUE;
}

#ifdef TRY_LOCALE_TO_UTF8_CONVERSION

static GString *
get_file_in_locale (GString *s)
{
	GError *error = NULL;
	gchar  *str;
	gsize	bytes_read;
	gsize	bytes_written;

	str = g_locale_to_utf8 (s->str,
				s->len,
				&bytes_read,
				&bytes_written,
				&error);
	if (error) {
		g_debug ("  Conversion to UTF-8 read %d bytes, wrote %d bytes",
			 bytes_read,
			 bytes_written);
		g_message ("Could not convert file from locale to UTF-8, %s",
			   error->message);
		g_error_free (error);
		g_free (str);
	} else {
		g_string_assign (s, str);
		g_free (str);
	}

	return s;
}

#endif /* TRY_LOCALE_TO_UTF8_CONVERSION */

static gchar *
get_file_content (const gchar *path)
{
	GFile		 *file;
	GFileInputStream *stream;
	GError		 *error = NULL;
	GString		 *s;
	gssize		  bytes;
	gssize		  bytes_valid;
	gssize		  bytes_read_total;
	gssize		  buf_size;
	gchar		  buf[TEXT_CHECK_SIZE];
	gboolean	  has_more_data;
	gboolean	  has_reached_max;
	gboolean	  is_utf8;

	file = g_file_new_for_path (path);
	stream = g_file_read (file, NULL, &error);

	if (error) {
		g_message ("Could not get read file:'%s', %s",
			   path,
			   error->message);
		g_error_free (error);
		g_object_unref (file);

		return NULL;
	}

	s = g_string_new ("");
	has_reached_max = FALSE;
	has_more_data = TRUE;
	bytes_read_total = 0;
	buf_size = TEXT_CHECK_SIZE - 1;

	g_debug ("  Starting read...");

	while (has_more_data && !has_reached_max && !error) {
		gssize bytes_read;
		gssize bytes_remaining;

		/* Leave space for NULL termination and make sure we
		 * add it at the end now.
		 */
		bytes_remaining = buf_size;
		bytes_read = 0;

		/* Loop until we hit the maximum */
		for (bytes = -1; bytes != 0 && !error; ) {
			bytes = g_input_stream_read (G_INPUT_STREAM (stream),
						     buf,
						     bytes_remaining,
						     NULL,
						     &error);

			bytes_read += bytes;
			bytes_remaining -= bytes;

			g_debug ("  Read %" G_GSSIZE_FORMAT " bytes", bytes);
		}

		/* Set the NULL termination after the last byte read */
		buf[TEXT_CHECK_SIZE - bytes_remaining] = '\0';

		/* First of all, check if this is the first time we
		 * have tried to read the file up to the TEXT_CHECK_SIZE
		 * limit. Then make sure that we read the maximum size
		 * of the buffer. If we don't do this, there is the
		 * case where we read 10 bytes in and it is just one
		 * line with no '\n'. Once we have confirmed this we
		 * check that the buffer has a '\n' to make sure the
		 * file is worth indexing. Similarly if the file has
		 * <= 3 bytes then we drop it.
		 */
		if (bytes_read_total == 0) {
			if (bytes_read == buf_size &&
			    strchr (buf, '\n') == NULL) {
				g_debug ("  No '\\n' in the first %" G_GSSIZE_FORMAT " bytes, not indexing file",
					 buf_size);
				break;
			} else if (bytes_read <= 2) {
				g_debug ("  File has less than 3 characters in it, not indexing file");
				break;
			}
		}

		/* Here we increment the bytes read total to evaluate
		 * the next states. We don't do this before the
		 * previous condition so we can know when we have
		 * iterated > 1.
		 */
		bytes_read_total += bytes_read;

		if (bytes_read != buf_size || bytes_read == 0) {
			has_more_data = FALSE;
		}

		if (bytes_read_total >= TEXT_MAX_SIZE) {
			has_reached_max = TRUE;
		}

		g_debug ("  Read "
			 "%" G_GSSIZE_FORMAT " bytes total, "
			 "%" G_GSSIZE_FORMAT " bytes this time, "
			 "more data:%s, reached max:%s",
			 bytes_read_total,
			 bytes_read,
			 has_more_data ? "yes" : "no",
			 has_reached_max ? "yes" : "no");

		/* The + 1 is for the NULL terminating byte */
		s = g_string_append_len (s, buf, bytes_read + 1);
	}

	if (has_reached_max) {
		g_debug ("  Maximum indexable limit reached");
	}

	if (error) {
		g_message ("Could not read input stream for:'%s', %s",
			   path,
			   error->message);
		g_error_free (error);
		g_string_free (s, TRUE);
		g_object_unref (stream);
		g_object_unref (file);

		return NULL;
	}

	/* Check for UTF-8 Validity, if not try to convert it to the
	 * locale we are in.
	 */
	is_utf8 = get_file_is_utf8 (s, &bytes_valid);

	/* Make sure the string is NULL terminated and in the case
	 * where the string is valid UTF-8 up to the last character
	 * which was cut off, NULL terminate to the last most valid
	 * character.
	 */
#ifdef TRY_LOCALE_TO_UTF8_CONVERSION
	if (!is_utf8) {
		s = get_file_in_locale (s);
	} else {
		g_debug ("  Truncating to last valid UTF-8 character (%d/%d bytes)",
			 bytes_valid,
			 s->len);
		s = g_string_truncate (s, bytes_valid);
	}
#else	/* TRY_LOCALE_TO_UTF8_CONVERSION */
	g_debug ("  Truncating to last valid UTF-8 character (%" G_GSSIZE_FORMAT "/%" G_GSSIZE_FORMAT " bytes)",
		 bytes_valid,
		 s->len);
	s = g_string_truncate (s, bytes_valid);
#endif	/* TRY_LOCALE_TO_UTF8_CONVERSION */

	g_object_unref (stream);
	g_object_unref (file);

	if (s->len < 1) {
		g_string_free (s, TRUE);
		s = NULL;
	}

	return s ? g_string_free (s, FALSE) : NULL;
}

static gchar *
get_file_content_by_filter (const gchar *path,
			    const gchar *mime)
{
	ProcessContext *context;
	gchar *str, *text_filter_file;
	gchar **argv;
	GString *text;

#ifdef G_OS_WIN32
	str = g_strconcat (mime, "_filter.bat", NULL);
#else
	str = g_strconcat (mime, "_filter", NULL);
#endif

	text_filter_file = g_build_filename (LIBDIR,
					     "tracker",
					     "filters",
					     str,
					     NULL);

	g_free (str);

	if (!g_file_test (text_filter_file, G_FILE_TEST_EXISTS)) {
		g_free (text_filter_file);
		return NULL;
	}

	argv = g_new0 (gchar *, 3);
	argv[0] = text_filter_file;
	argv[1] = (gchar *) path;

	g_message ("Extracting text for:'%s' using filter:'%s'", argv[1], argv[0]);

	context = process_context_create ((const gchar **) argv,
					  get_file_content_read_cb);

	g_free (text_filter_file);
	g_free (argv);

	if (!context) {
		return NULL;
	}

	text = g_string_new (NULL);
	context->data = text;

	/* It will block here until all incoming
	 * text has been processed
	 */
	g_main_loop_run (context->data_incoming_loop);

	return g_string_free (text, FALSE);
}

/**
 * tracker_module_metadata_utils_get_text:
 * @file: A #GFile
 *
 * Gets the text from @file, if the file is considered as
 * containing plain text, it will be extracted, else this function
 * will resort to the installed text filters for the file MIME type.
 *
 * Returns: A newly allocated string containing the file text, or %NULL.
 **/
gchar *
tracker_module_metadata_utils_get_text (GFile *file)
{
	const gchar *service_type;
	gchar *path, *mime_type, *text;

	path = g_file_get_path (file);

	mime_type = tracker_file_get_mime_type (path);
	service_type = tracker_ontology_get_service_by_mime (mime_type);

	/* No need to filter text based files - index them directly */
	if (service_type &&
	    (strcmp (service_type, "Text") == 0 ||
	     strcmp (service_type, "Development") == 0)) {
		text = get_file_content (path);
	} else {
		text = get_file_content_by_filter (path, mime_type);
	}

	g_free (mime_type);
	g_free (path);

	return text;
}

/**
 * tracker_module_metadata_utils_get_data:
 * @file: A #GFile
 *
 * Returns a #TrackerModuleMetadata filled in with all the
 * metadata that could be extracted for the given file.
 *
 * Returns: A newly created #TrackerModuleMetadata, or %NULL if the file is not found.
 **/
TrackerModuleMetadata *
tracker_module_metadata_utils_get_data (GFile *file)
{
	TrackerModuleMetadata *metadata;
	struct stat st;
	const gchar *ext;
	gchar *path, *mime_type;
	gchar *dirname, *basename, *path_delimited;

	path = g_file_get_path (file);

	if (g_lstat (path, &st) < 0) {
		g_free (path);
		return NULL;
	}

	metadata = tracker_module_metadata_new ();
	ext = strrchr (path, '.');

	if (ext) {
		ext++;
		tracker_module_metadata_add_string (metadata, METADATA_FILE_EXT, ext);
	}

	mime_type = tracker_file_get_mime_type (path);

	dirname = g_path_get_dirname (path);
	basename = g_filename_display_basename (path);
	path_delimited = g_filename_to_utf8 (path, -1, NULL, NULL, NULL);

	tracker_module_metadata_add_string (metadata, METADATA_FILE_NAME, basename);
	tracker_module_metadata_add_string (metadata, METADATA_FILE_PATH, dirname);
	tracker_module_metadata_add_string (metadata, METADATA_FILE_NAME_DELIMITED, path_delimited);
	tracker_module_metadata_add_string (metadata, METADATA_FILE_MIMETYPE, mime_type);

	g_free (path_delimited);
	g_free (basename);
	g_free (dirname);

	if (mime_type) {
		gchar *uri;

		uri = g_file_get_uri (file);
		tracker_thumbnailer_get_file_thumbnail (uri, mime_type);
		g_free (uri);
	}

	if (S_ISLNK (st.st_mode)) {
		gchar *link_path;
		gchar *link_path_delimited;

		link_path = g_file_read_link (path, NULL);
		link_path_delimited = g_filename_to_utf8 (link_path, -1, NULL, NULL, NULL);

		tracker_module_metadata_add_string (metadata, METADATA_FILE_LINK, link_path_delimited);

		g_free (link_path_delimited);
		g_free (link_path);
	}

	tracker_module_metadata_add_uint (metadata, METADATA_FILE_SIZE, st.st_size);
	tracker_module_metadata_add_date (metadata, METADATA_FILE_MODIFIED, st.st_mtime);
	tracker_module_metadata_add_date (metadata, METADATA_FILE_ACCESSED, st.st_atime);

	metadata_utils_get_embedded (path, mime_type, metadata);

	g_free (mime_type);
	g_free (path);

	return metadata;
}