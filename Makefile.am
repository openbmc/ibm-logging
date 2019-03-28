AM_DEFAULT_SOURCE_EXT = .cpp

bin_PROGRAMS = ibm-log-manager

ibm_log_manager_SOURCES = \
	callout.cpp \
	dbus.cpp \
	main.cpp \
	manager.cpp \
	policy_find.cpp \
	policy_table.cpp

ibm_log_manager_CXX_FLAGS =  \
	$(PHOSPHOR_DBUS_INTERFACES_CFLAGS) \
	$(IBM_DBUS_INTERFACES_CFLAGS) \
	$(SDBUSPLUS_CFLAGS) \
	$(PHOSPHOR_LOGGING_CFLAGS)

ibm_log_manager_LDFLAGS = \
	-lstdc++fs \
	$(PHOSPHOR_DBUS_INTERFACES_LIBS) \
	$(IBM_DBUS_INTERFACES_LIBS) \
	$(SDBUSPLUS_LIBS) \
	$(PHOSPHOR_LOGGING_LIBS)

SUBDIRS = . test
