AM_DEFAULT_SOURCE_EXT = .cpp

sbin_PROGRAMS = ibm-log-manager

ibm_log_manager_SOURCES = \
	callout.cpp \
	dbus.cpp \
	main.cpp \
	manager.cpp \
	policy_find.cpp \
	policy_table.cpp \
	xyz/openbmc_project/Common/ObjectPath/server.cpp

#TODO: remove, and also the line above
BUILT_SOURCES = \
	xyz/openbmc_project/Common/ObjectPath/server.cpp \
	xyz/openbmc_project/Common/ObjectPath/server.hpp

CLEANFILES=${BUILT_SOURCES}

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

#TODO: remove
xyz/openbmc_project/Common/ObjectPath/server.cpp: \
xyz/openbmc_project/Common/ObjectPath.interface.yaml \
xyz/openbmc_project/Common/ObjectPath/server.hpp
	@mkdir -p `dirname $@`
	$(SDBUSPLUSPLUS) -r $(srcdir) interface server-cpp \
xyz.openbmc_project.Common.ObjectPath > $@

xyz/openbmc_project/Common/ObjectPath/server.hpp: \
xyz/openbmc_project/Common/ObjectPath.interface.yaml
	@mkdir -p `dirname $@`
	$(SDBUSPLUSPLUS) -r $(srcdir) interface server-header \
xyz.openbmc_project.Common.ObjectPath > $@

SUBDIRS = . test
