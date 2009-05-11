/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Tracker Extract - extracts embedded metadata from files
 * Copyright (C) 2006, Mr Jamie McCracken (jamiemcc@gnome.org)
 * Copyright (C) 2008, Nokia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "config.h"

#include <glib.h>
#include <glib/gstdio.h>

#include <tiff.h>
#include <tiffio.h>

#include <libtracker-common/tracker-type-utils.h>
#include <libtracker-common/tracker-file-utils.h>

#include "tracker-main.h"
#include "tracker-xmp.h"
#include "tracker-iptc.h"

#define EXIF_DATE_FORMAT     "%Y:%m:%d %H:%M:%S"

typedef gchar * (*PostProcessor) (gchar *);

typedef enum {
	TIFF_TAGTYPE_UNDEFINED = 0,
	TIFF_TAGTYPE_STRING,
	TIFF_TAGTYPE_UINT16,
	TIFF_TAGTYPE_UINT32,
	TIFF_TAGTYPE_DOUBLE,
	TIFF_TAGTYPE_C16_UINT16
} TagType;

typedef struct {
	guint	       tag;
	gchar	      *name;
	TagType        type;
	PostProcessor  post;
} TiffTag;
 
static void   extract_tiff    (const gchar *filename,
			       GHashTable  *metadata);
static gchar *date_to_iso8601 (gchar       *date);

static TrackerExtractData data[] = {
	{ "image/tiff", extract_tiff },
	{ NULL, NULL }
};

/* FIXME: We are missing some */
static TiffTag tags[] = {
	{ TIFFTAG_ARTIST, "Image:Creator", TIFF_TAGTYPE_STRING, NULL },
	{ TIFFTAG_COPYRIGHT, "File:Copyright", TIFF_TAGTYPE_STRING, NULL },
	{ TIFFTAG_DATETIME, "Image:Date", TIFF_TAGTYPE_STRING, NULL },
	{ TIFFTAG_DOCUMENTNAME, "Image:Title", TIFF_TAGTYPE_STRING, NULL },
	{ TIFFTAG_IMAGEDESCRIPTION, "Image:Comments", TIFF_TAGTYPE_STRING, NULL },
	{ TIFFTAG_IMAGEWIDTH, "Image:Width", TIFF_TAGTYPE_UINT32, NULL },
	{ TIFFTAG_IMAGELENGTH, "Image:Height", TIFF_TAGTYPE_UINT32, NULL },
	{ TIFFTAG_MAKE, "Image:CameraMake", TIFF_TAGTYPE_STRING, NULL },
	{ TIFFTAG_MODEL, "Image:CameraModel", TIFF_TAGTYPE_STRING, NULL },
	{ TIFFTAG_ORIENTATION, "Image:Orientation", TIFF_TAGTYPE_UINT16, NULL },
	{ TIFFTAG_SOFTWARE, "Image:Software", TIFF_TAGTYPE_STRING, NULL },
	{ -1, NULL, TIFF_TAGTYPE_UNDEFINED, NULL }
};

static TiffTag exiftags[] = {
	{ EXIFTAG_EXPOSURETIME, "Image:ExposureTime", TIFF_TAGTYPE_DOUBLE, NULL},
	{ EXIFTAG_FNUMBER, "Image:FNumber", TIFF_TAGTYPE_DOUBLE, NULL},
	{ EXIFTAG_EXPOSUREPROGRAM, "Image:ExposureProgram", TIFF_TAGTYPE_UINT16, NULL },
	{ EXIFTAG_ISOSPEEDRATINGS, "Image:ISOSpeed", TIFF_TAGTYPE_C16_UINT16, NULL},
	{ EXIFTAG_DATETIMEORIGINAL, "Image:Date", TIFF_TAGTYPE_STRING, date_to_iso8601 },
	{ EXIFTAG_METERINGMODE, "Image:MeteringMode", TIFF_TAGTYPE_UINT16, NULL},
	{ EXIFTAG_FLASH, "Image:Flash", TIFF_TAGTYPE_UINT16, NULL},
	{ EXIFTAG_FOCALLENGTH, "Image:FocalLength", TIFF_TAGTYPE_DOUBLE, NULL},
	{ EXIFTAG_PIXELXDIMENSION, "Image:Width", TIFF_TAGTYPE_UINT32, NULL},
	{ EXIFTAG_PIXELYDIMENSION, "Image:Height", TIFF_TAGTYPE_UINT32, NULL},
	{ EXIFTAG_WHITEBALANCE, "Image:WhiteBalance", TIFF_TAGTYPE_UINT16, NULL},
	{ -1, NULL, TIFF_TAGTYPE_UNDEFINED, NULL }
};

static gchar *
date_to_iso8601 (gchar *date)
{
	/* ex; date "2007:04:15 15:35:58"
	 * To
	 * ex. "2007-04-15T17:35:58+0200 where +0200 is localtime
	 */
	return tracker_date_format_to_iso8601 (date, EXIF_DATE_FORMAT);
}

static void
extract_tiff (const gchar *filename, 
	      GHashTable  *metadata)
{
	TIFF *image;
	glong exifOffset;

	TiffTag *tag;

	gchar buffer[1024];
	gchar *text;
	guint16 varui16 = 0;
	guint32 varui32 = 0;
	
	void *data;
	guint16 count16;

	gfloat vardouble;

#ifdef HAVE_LIBIPTCDATA
	gchar   *iptcOffset;
	guint32  iptcSize;
#endif

#ifdef HAVE_EXEMPI
	gchar *xmpOffset;
	guint32 size;
#endif /* HAVE_EXEMPI */

	if ((image = TIFFOpen (filename, "r")) == NULL){
		g_critical ("Could not open image:'%s'\n", filename);
		goto fail;
	}

#ifdef HAVE_LIBIPTCDATA
	if (TIFFGetField (image, TIFFTAG_RICHTIFFIPTC, &iptcSize, &iptcOffset)) {
		if (TIFFIsByteSwapped(image) != 0) 
			TIFFSwabArrayOfLong((uint32 *) iptcOffset,(unsigned long) iptcSize);
		tracker_read_iptc (iptcOffset,
				   4*iptcSize,
				   metadata);
	}
#endif /* HAVE_LIBIPTCDATA */

	/* FIXME There are problems between XMP data embedded with different tools
	   due to bugs in the original spec (type) */
#ifdef HAVE_EXEMPI
	if (TIFFGetField (image, TIFFTAG_XMLPACKET, &size, &xmpOffset)) {
		tracker_read_xmp (xmpOffset,
				  size,
				  metadata);
	}
#endif /* HAVE_EXEMPI */

	if (TIFFGetField (image, TIFFTAG_EXIFIFD, &exifOffset)) {
		if (TIFFReadEXIFDirectory (image, exifOffset)) {
			for (tag = exiftags; tag->name; ++tag) {
				switch (tag->type) {
				case TIFF_TAGTYPE_STRING:
					if (!TIFFGetField (image, tag->tag, &text)) {
						continue;
					}

					sprintf (buffer,"%s",text);
					break;
				case TIFF_TAGTYPE_UINT16:						
					if (!TIFFGetField (image, tag->tag, &varui16)) {
						continue;
					}

					sprintf (buffer,"%i",varui16);
					break;
				case TIFF_TAGTYPE_UINT32:
					if (!TIFFGetField (image, tag->tag, &varui32)) {
						continue;
					}

					sprintf(buffer,"%i",varui32);
					break;
				case TIFF_TAGTYPE_DOUBLE:
					if (!TIFFGetField (image, tag->tag, &vardouble)) {
						continue;
					}

					sprintf (buffer,"%f",vardouble);
					break;
				case TIFF_TAGTYPE_C16_UINT16:						
					if (!TIFFGetField (image, tag->tag, &count16, &data)) {
						continue;
					}

					/* We only take only the first for now */
					sprintf (buffer,"%i",*(guint16 *)data);
					break;	

				default:
					continue;
					break;
				}

				if (tag->post) {
					g_hash_table_insert (metadata,
							     g_strdup (tag->name),
							     tracker_escape_metadata ((*tag->post) (buffer)));
				} else {
					g_hash_table_insert (metadata, 
							     g_strdup (tag->name),
							     tracker_escape_metadata (buffer));
				}
			}
		}
	}

	/* We want to give native tags priority over XMP/Exif */
	for (tag = tags; tag->name; ++tag) {
		switch (tag->type) {
			case TIFF_TAGTYPE_STRING:
				if (!TIFFGetField (image, tag->tag, &text)) {
					continue;
				}

				sprintf (buffer,"%s", text);
				break;
			case TIFF_TAGTYPE_UINT16:
				if (!TIFFGetField (image, tag->tag, &varui16)) {
					continue;
				}

				sprintf (buffer,"%i",varui16);
				break;
			case TIFF_TAGTYPE_UINT32:
				if (!TIFFGetField (image, tag->tag, &varui32)) {
					continue;
				}

				sprintf(buffer,"%i",varui32);
				break;
			case TIFF_TAGTYPE_DOUBLE:
				if (!TIFFGetField (image, tag->tag, &vardouble)) {
					continue;
				}

				sprintf (buffer,"%f",vardouble);
				break;
			default:
				continue;
				break;
			}

		if (tag->post) {
			g_hash_table_insert (metadata, 
					     g_strdup (tag->name),
					     tracker_escape_metadata ((*tag->post) (buffer)));
		} else {
			g_hash_table_insert (metadata, 
					     g_strdup (tag->name),
					     tracker_escape_metadata (buffer));
		}
	}

	TIFFClose (image);

fail:
	/* We fallback to the file's modified time for the
	 * "Image:Date" metadata if it doesn't exist.
	 *
	 * FIXME: This shouldn't be necessary.
	 */
	if (!g_hash_table_lookup (metadata, "Image:Date")) {
		gchar *date;
		guint64 mtime;
		
		mtime = tracker_file_get_mtime (filename);
		date = tracker_date_to_string ((time_t) mtime);
		
		g_hash_table_insert (metadata,
				     g_strdup ("Image:Date"),
				     tracker_escape_metadata (date));
		g_free (date);
	}
}

TrackerExtractData *
tracker_get_extract_data (void)
{
	return data;
}
