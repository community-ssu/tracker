include $(top_srcdir)/Makefile.decl

noinst_PROGRAMS = $(TEST_PROGS)

#
# NOTE: Some of these tests have been disabled (by Martyn) temporarily
# 	because make distcheck fails. The reason for this is that it
# 	depends files being installed to $prefix/share which have not
# 	been installed at this point.
# 
#       These tests include:
#
#         tracker-parser
#	  tracker-field
#

TEST_PROGS += 								\
	tracker-ontology 						\
	tracker-dbus 							\
	tracker-type-utils 						\
	tracker-file-utils						\
	tracker-albumart						\
	tracker-thumbnailer

INCLUDES = 								\
	-DG_LOG_DOMAIN=\"Tracker\"					\
	-DTRACKER_COMPILATION						\
	-I$(top_srcdir)/src						\
	-I$(top_srcdir)/tests/common					\
	$(WARN_CFLAGS)							\
	$(GLIB2_CFLAGS)							\
	$(GCOV_CFLAGS)							\
	$(GMODULE_CFLAGS)						\
	$(GTHREAD_CFLAGS)						\
	$(PANGO_CFLAGS)							\
	$(DBUS_CFLAGS)

tracker_ontology_SOURCES = 						\
	tracker-ontology-test.c 

tracker_ontology_LDADD =						\
	$(top_builddir)/src/libtracker-common/libtracker-common.la 	\
	$(top_builddir)/tests/common/libtracker-testcommon.la 		\
	$(GMODULE_LIBS)							\
	$(GCOV_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GLIB2_LIBS)							

tracker_dbus_SOURCES = 							\
	tracker-dbus-test.c

tracker_dbus_LDADD =							\
	$(top_builddir)/src/libtracker-common/libtracker-common.la 	\
	$(top_builddir)/tests/common/libtracker-testcommon.la 		\
	$(DBUS_LIBS)							\
	$(GCOV_LIBS)							\
	$(GMODULE_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GLIB2_LIBS)							

tracker_type_utils_SOURCES = \
	tracker-type-utils-test.c 

tracker_type_utils_LDADD =						\
	$(top_builddir)/src/libtracker-common/libtracker-common.la 	\
	$(top_builddir)/tests/common/libtracker-testcommon.la 		\
	$(GMODULE_LIBS)							\
	$(GCOV_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GLIB2_LIBS)							

tracker_file_utils_SOURCES = 						\
	tracker-file-utils-test.c 

tracker_file_utils_LDADD =						\
	$(top_builddir)/src/libtracker-common/libtracker-common.la 	\
	$(top_builddir)/tests/common/libtracker-testcommon.la 		\
	$(GMODULE_LIBS)							\
	$(GCOV_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GLIB2_LIBS)							

# tracker_parser_SOURCES = 						\
# 	tracker-parser-test.c 
#
# tracker_parser_LDADD =						\
# 	$(top_builddir)/src/libtracker-common/libtracker-common.la 	\
# 	$(top_builddir)/tests/common/libtracker-testcommon.la 		\
# 	$(top_builddir)/src/libstemmer/libstemmer.la	 		\
# 	$(GMODULE_LIBS)							\
# 	$(GTHREAD_LIBS)							\
#	$(GCOV_LIBS)							\
# 	$(PANGO_LIBS)							\
# 	$(GLIB2_LIBS)							
#
# tracker_field_SOURCES = 						\
# 	tracker-field-test.c 
#
# tracker_field_LDADD =							\
# 	$(top_builddir)/src/libtracker-common/libtracker-common.la 	\
# 	$(top_builddir)/tests/common/libtracker-testcommon.la 		\
# 	$(GMODULE_LIBS)							\
#	$(GCOV_LIBS)							\
# 	$(GTHREAD_LIBS)							\
# 	$(PANGO_LIBS)							\
# 	$(GLIB2_LIBS)							

tracker_albumart_SOURCES = 						\
	tracker-albumart-test.c 

tracker_albumart_LDADD =						\
	$(top_builddir)/src/libtracker-common/libtracker-common.la 	\
	$(top_builddir)/tests/common/libtracker-testcommon.la 		\
	$(GMODULE_LIBS)							\
	$(GCOV_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GLIB2_LIBS)							

tracker_thumbnailer_SOURCES = 						\
	tracker-thumbnailer-test.c					\
	mock-dbus-gproxy.c						\
	mock-thumbnailer-dbus.c

tracker_thumbnailer_LDADD =						\
	$(top_builddir)/src/libtracker-common/libtracker-common.la 	\
	$(top_builddir)/tests/common/libtracker-testcommon.la 		\
	$(GMODULE_LIBS)							\
	$(GCOV_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GLIB2_LIBS)							

EXTRA_DIST = non-utf8.txt
