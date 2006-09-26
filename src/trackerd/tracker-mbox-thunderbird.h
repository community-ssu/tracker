/* Tracker
 * mbox routines
 * Copyright (C) 2005, Mr Jamie McCracken
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _TRACKER_MBOX_THUNDERBIRD_H_
#define _TRACKER_MBOX_THUNDERBIRD_H_

#include <glib.h>
#include <gmime/gmime.h>

#include "tracker-mbox.h"

#include "config.h"

#ifdef USING_SQLITE
#   include "tracker-db-sqlite.h"
#else
#   include "tracker-db-mysql.h"
#endif


/*
 * These functions are supposed to be used only with tracker-mbox.c
 *
 */

GSList *	watch_emails_of_thunderbird		(DBConnection *db_con);
void		tracker_get_status_of_thunderbird_email	(GMimeMessage *g_m_message, MailMessage *msg);
gboolean	is_in_a_thunderbird_mail_dir		(const char *uri);

#endif
