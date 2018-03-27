AC_PREREQ([2.69])
AC_INIT([ibm-logging], [1.0], [https://github.com/openbmc/ibm-logging/issues])
AC_LANG([C++])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror foreign dist-xz])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET

PKG_CHECK_MODULES([PHOSPHOR_DBUS_INTERFACES], [phosphor-dbus-interfaces],,\
    AC_MSG_ERROR(["Requires phosphor-dbus-interfaces package."]))
PKG_CHECK_MODULES([SDBUSPLUS], [sdbusplus],,
    AC_MSG_ERROR(["Requires sdbusplus package."]))
PKG_CHECK_MODULES([PHOSPHOR_LOGGING], [phosphor-logging],,\
    AC_MSG_ERROR(["Requires phosphor-logging package."]))

# Checks for typedefs, structures, and compiler characteristics.
AX_CXX_COMPILE_STDCXX_14([noext])
AX_APPEND_COMPILE_FLAGS([-Wall -Werror], [CXXFLAGS])

LT_INIT

# Check/set gtest specific functions.
AX_PTHREAD([GTEST_CPPFLAGS="-DGTEST_HAS_PTHREAD=1"],[GTEST_CPPFLAGS="-DGTEST_HAS_PTHREAD=0"])
AC_SUBST(GTEST_CPPFLAGS)
AC_ARG_ENABLE([oe-sdk],
    AS_HELP_STRING([--enable-oe-sdk], [Link testcases absolutely against OE SDK so they can be ran within it.])
)
AC_ARG_VAR(OECORE_TARGET_SYSROOT,
    [Path to the OE SDK SYSROOT])
AS_IF([test "x$enable_oe_sdk" == "xyes"],
    AS_IF([test "x$OECORE_TARGET_SYSROOT" == "x"],
          AC_MSG_ERROR([OECORE_TARGET_SYSROOT must be set with --enable-oe-sdk])
    )
    AC_MSG_NOTICE([Enabling OE-SDK at $OECORE_TARGET_SYSROOT])
    [
        testcase_flags="-Wl,-rpath,\${OECORE_TARGET_SYSROOT}/lib"
        testcase_flags="${testcase_flags} -Wl,-rpath,\${OECORE_TARGET_SYSROOT}/usr/lib"
        testcase_flags="${testcase_flags} -Wl,-dynamic-linker,`find \${OECORE_TARGET_SYSROOT}/lib/ld-*.so | sort -r -n | head -n1`"
    ]
    AC_SUBST([OESDK_TESTCASE_FLAGS], [$testcase_flags])
)

#The policy data must have been defined by the service folks
#for it to be used, so allow its use to be configurable and
#default it to off.
AC_ARG_ENABLE([policy-interface],
              AS_HELP_STRING([--enable-policy-interface],
                             [Enable the Policy D-Bus interface])
)

AC_ARG_VAR(USE_POLICY_INTERFACE, [If the Policy D-Bus interface should be created])

AS_IF([test "x$enable_policy_interface" == "xyes"],
      [USE_POLICY_INTERFACE="yes"]
      AC_DEFINE_UNQUOTED([USE_POLICY_INTERFACE], ["$USE_POLICY_INTERFACE"],
                         [If the Policy D-Bus interface should be created])
)

AC_DEFINE(IBM_LOGGING_BUSNAME, "com.ibm.Logging", [The log manager DBus busname to own.])
AC_DEFINE(IBM_LOGGING_PATH, "/com/ibm/logging", [The log manager DBus object path.])

AC_DEFINE(DEFAULT_POLICY_EID, "None", [The default event ID to use])
AC_DEFINE(DEFAULT_POLICY_MSG, "An internal BMC error occurred", [The default policy message to use])

AC_ARG_VAR(POLICY_JSON_PATH, [The path to the policy json file])
AS_IF([test "x$POLICY_JSON_PATH" == "x"], [POLICY_JSON_PATH="/usr/share/ibm-logging/policy.json"])
AC_DEFINE_UNQUOTED([POLICY_JSON_PATH], ["$POLICY_JSON_PATH"], [The path to the policy json file on the BMC])

AC_CONFIG_FILES([Makefile test/Makefile])
AC_OUTPUT
