AM_CPPFLAGS = -I$(top_srcdir)

TESTS = $(check_PROGRAMS)

check_PROGRAMS = test_policy
test_policy_CPPFLAGS = -Igtest $(GTEST_CPPFLAGS) $(AM_CPPFLAGS)

test_policy_CXXFLAGS = $(PTHREAD_CFLAGS) $(SDBUSPLUS_CFLAGS)
test_policy_LDFLAGS = -lgtest_main -lgtest \
	$(PTHREAD_LIBS) $(OESDK_TESTCASE_FLAGS)

test_policy_SOURCES = test_policy.cpp
test_policy_LDADD = \
	$(top_builddir)/policy_table.o \
	$(top_builddir)/policy_find.o \
	-lstdc++fs \
	$(SDBUSPLUS_LIBS)
