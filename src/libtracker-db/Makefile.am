include $(top_srcdir)/Makefile.decl

INCLUDES =								\
	-DSHAREDIR=\""$(datadir)"\"					\
	-DG_LOG_DOMAIN=\"Tracker\"					\
	-DTRACKER_COMPILATION						\
	-I$(top_srcdir)/src						\
	$(WARN_CFLAGS)							\
	$(GLIB2_CFLAGS)							\
	$(GCOV_CFLAGS)							\
	$(DBUS_CFLAGS)							\
	$(SQLITE3_CFLAGS)						\
	$(QDBM_CFLAGS)

libtracker_dbdir = $(libdir)/tracker
libtracker_db_LTLIBRARIES = libtracker-db.la

libtracker_db_la_SOURCES = 						\
	tracker-db-action.c						\
	tracker-db-dbus.c						\
	tracker-db-index.c						\
	tracker-db-index-item.c						\
	tracker-db-index-manager.c					\
	tracker-db-interface.c						\
	tracker-db-interface-sqlite.c					\
	tracker-db-manager.c

noinst_HEADERS =							\
	tracker-db-action.h						\
	tracker-db-dbus.h						\
	tracker-db-index.h						\
	tracker-db-index-item.h						\
	tracker-db-index-manager.h					\
	tracker-db-interface.h						\
	tracker-db-interface-sqlite.h					\
	tracker-db-manager.h 

libtracker_db_la_LDFLAGS =						\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

libtracker_db_la_LIBADD = 						\
	$(top_builddir)/src/libtracker-common/libtracker-common.la	\
	$(SQLITE3_LIBS)							\
	$(QDBM_LIBS)							\
	$(DBUS_LIBS)							\
	$(GCOV_LIBS)							\
	$(GLIB2_LIBS)							\
	-lz