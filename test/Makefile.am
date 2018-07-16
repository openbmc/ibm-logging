AM_CPPFLAGS = -I$(top_srcdir)

TESTS = $(check_PROGRAMS)

check_PROGRAMS = test_policy test_callout

test_cppflags = \
	-Igtest \
	$(GTEST_CPPFLAGS) \
	$(AM_CPPFLAGS)

test_cxxflags = \
	$(PTHREAD_CFLAGS) \
	$(PHOSPHOR_DBUS_INTERFACES_CFLAGS) \
	$(IBM_DBUS_INTERFACES_CFLAGS) \
	$(SDBUSPLUS_CFLAGS) \
	$(PHOSPHOR_LOGGING_CFLAGS)

test_ldflags = \
	-lgtest_main -lgtest \
	-lstdc++fs \
	$(PTHREAD_LIBS) \
	$(OESDK_TESTCASE_FLAGS) \
	$(PHOSPHOR_DBUS_INTERFACES_LIBS) \
	$(IBM_DBUS_INTERFACES_LIBS) \
	$(SDBUSPLUS_LIBS)

test_policy_CPPFLAGS = $(test_cppflags)
test_policy_CXXFLAGS = $(test_cxxflags)
test_policy_LDFLAGS = $(test_ldflags)
test_policy_SOURCES = test_policy.cpp
test_policy_LDADD = \
	$(top_builddir)/policy_table.o \
	$(top_builddir)/policy_find.o

test_callout_CPPFLAGS = $(test_cppflags)
test_callout_CXXFLAGS = $(test_cxxflags)
test_callout_LDFLAGS = $(test_ldflags)
test_callout_SOURCES = test_callout.cpp

test_callout_LDADD = \
	$(top_builddir)/callout.o


