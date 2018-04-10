AM_DEFAULT_SOURCE_EXT = .cpp
AM_CPPFLAGS = -I$(top_srcdir)

noinst_HEADERS = \
	esel_to_pel.hpp \
	pel.hpp

sbin_PROGRAMS = openpower-esel-to-pel

openpower_esel_to_pel_SOURCES = \
	esel_to_pel.cpp \
	pel.cpp \
	main.cpp



BUILT_SOURCES =


CLEANFILES =

openpower_esel_to_pel_LDFLAGS = \
	$(SYSTEMD_LIBS) \
	$(PHOSPHOR_LOGGING_LIBS) \
	$(PHOSPHOR_DBUS_INTERFACES_LIBS) \
	$(OPENPOWER_DBUS_INTERFACES_LIBS) \
	$(SDBUSPLUS_LIBS)

openpower_esel_to_pel_CXXFLAGS = \
	$(SYSTEMD_CFLAGS) \
	$(PHOSPHOR_LOGGING_CFLAGS) \
	$(PHOSPHOR_DBUS_INTERFACES_CFLAGS) \
	$(OPENPOWER_DBUS_INTERFACES_CFLAGS) \
	$(SDBUSPLUS_CFLAGS)

SUBDIRS = 
