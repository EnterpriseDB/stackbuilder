################################################################################
# Initial setup
################################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

IF(COMMAND cmake_policy)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(CMAKE_FIND_LIBRARY_PREFIXES "")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")

INCLUDE(CPack)

################################################################################
# Apple stuff
################################################################################
IF(APPLE)
    # Setup default values
    IF(NOT HAVE_CACHED_VALUES)
        IF(EXISTS /Developer/SDKs/MacOSX10.5.sdk)
            SET(CMAKE_OSX_ARCHITECTURES "ppc;i386" CACHE STRING "Build architectures for OSX" FORCE)
            SET(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.5.sdk" CACHE FILEPATH "isysroot used for universal binary support" FORCE)
        ELSE()
            IF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
                SET(CMAKE_OSX_ARCHITECTURES "ppc;i386" CACHE STRING "Build architectures for OSX" FORCE)
                SET(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.4u.sdk" CACHE FILEPATH "isysroot used for universal binary support" FORCE)
            ELSE()
                MESSAGE(FATAL_ERROR "No supported SDK could be found!")
            ENDIF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
        ENDIF(EXISTS /Developer/SDKs/MacOSX10.5.sdk)
    ENDIF(NOT HAVE_CACHED_VALUES)

    # Target Tiger
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.4")
ENDIF(APPLE)

################################################################################
# Project config
################################################################################
PROJECT(stackbuilder)

SET(VERSION 2.0.0)

# CPack stuff
SET(CPACK_PACKAGE_VERSION_MAJOR 2)
SET(CPACK_PACKAGE_VERSION_MINOR 0)
SET(CPACK_PACKAGE_VERSION_PATCH 0)
SET(CPACK_PACKAGE_NAME "StackBuilder")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "PostgreSQL StackBuilder")
SET(CPACK_PACKAGE_VENDOR "EnterpriseDB UK")
SET(CPACK_GENERATOR TGZ)
SET(CPACK_SOURCE_GENERATOR TGZ)
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION}")
SET(CPACK_SOURCE_IGNORE_FILES "${PROJECT_NAME}-${VERSION}.tar.gz" "_CPack_Packages")

# OS X bundle stuff
SET(MACOSX_BUNDLE_INFO_STRING "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
SET(MACOSX_BUNDLE_ICON_FILE "StackBuilder.icns")
SET(MACOSX_BUNDLE_GUI_IDENTIFIER "com.enterprisedb.stackbuilder")
SET(MACOSX_BUNDLE_VERSION "${VERSION}")
SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION}")
SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION}")
SET(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}")
SET(MACOSX_BUNDLE_COPYRIGHT "Copyright 2007 - 2008, EnterpriseDB UK")

################################################################################
# Find wxWidgets
################################################################################
IF(NOT HAVE_CACHED_VALUES)
    SET(wxWidgets_USE_UNICODE ON CACHE BOOL "Use unicode build?" FORCE)
    SET(wxWidgets_USE_DEBUG OFF CACHE BOOL "Use debug build?" FORCE)
    SET(wxWidgets_USE_STATIC ON CACHE BOOL "Link libraries statically?" FORCE)
ENDIF(NOT HAVE_CACHED_VALUES)

SET(wxWidgets_USE_LIBS adv base core net xml)
FIND_PACKAGE(wxWidgets)

IF(NOT wxWidgets_FOUND)
    MESSAGE(FATAL_ERROR "wxWidgets could not be found!")
ENDIF(NOT wxWidgets_FOUND)

INCLUDE(${wxWidgets_USE_FILE})

IF(NOT WIN32)
    # Get the wxWidgets version
    EXECUTE_PROCESS(
        COMMAND sh "${wxWidgets_CONFIG_EXECUTABLE}" --version
        OUTPUT_VARIABLE wxWidgets_VERSION
        RESULT_VARIABLE RET
        ERROR_QUIET
    )
    STRING(STRIP ${wxWidgets_VERSION} wxWidgets_VERSION)

    IF(NOT ${wxWidgets_VERSION} MATCHES "^2.8.")
        MESSAGE(FATAL_ERROR "wxWidgets must be version 2.8.x (${wxWidgets_VERSION} found)")
    ENDIF(NOT ${wxWidgets_VERSION} MATCHES "^2.8.")
ELSE(NOT WIN32)
    IF(NOT wxWidgets_LIBRARIES MATCHES "wxmsw28")
        MESSAGE(FATAL_ERROR "wxWidgets must be version 2.8.x")
    ELSE(NOT wxWidgets_LIBRARIES MATCHES "wxmsw28")
        SET(wxWidgets_VERSION "2.8.x")
    ENDIF(NOT wxWidgets_LIBRARIES MATCHES "wxmsw28")
ENDIF(NOT WIN32)

################################################################################
# Compile translations
################################################################################
FIND_PROGRAM(MSGFMT_EXECUTABLE msgfmt)

# Add a custom output for each po file we need to compile
IF(MSGFMT_EXECUTABLE)

    FILE(GLOB_RECURSE _pofiles i18n/??_??/StackBuilder.po)

    FOREACH(_pofile ${_pofiles})
        GET_FILENAME_COMPONENT(_dir ${_pofile} PATH)
        GET_FILENAME_COMPONENT(_lang ${_dir} NAME)

        SET(_out "${_dir}/StackBuilder.${_lang}.mo")

        IF(APPLE)
            ADD_CUSTOM_COMMAND(
                OUTPUT ${_out}
                COMMAND ${MSGFMT_EXECUTABLE} -o ${_out} ${_pofile}
                COMMAND mkdir -p stackbuilder.app/Contents/Resources/i18n/${_lang}
                COMMAND cp ${_out} stackbuilder.app/Contents/Resources/i18n/${_lang}/StackBuilder.mo
                COMMAND cp ${_dir}/wxstd.mo stackbuilder.app/Contents/Resources/i18n/${_lang}/wxstd.mo 
                DEPENDS ${_pofile}
            )
        ELSE(APPLE)
            ADD_CUSTOM_COMMAND(
                OUTPUT ${_out}
                COMMAND ${MSGFMT_EXECUTABLE} -o ${_out} ${_pofile}
                DEPENDS ${_pofile}
            )
        ENDIF(APPLE)

        INSTALL(
            FILES ${_out}
            DESTINATION share/i18n/${_lang}/
            RENAME StackBuilder.mo
        )

        IF(EXISTS "${_dir}/wxstd.mo")
            INSTALL(
                FILES "${_dir}/wxstd.mo"
                DESTINATION share/i18n/${_lang}/
            )
        ENDIF(EXISTS "${_dir}/wxstd.mo")

        SET(_translations ${_translations} ${_out})

    ENDFOREACH(_pofile)

ELSE(MSGFMT_EXECUTABLE)
    MESSAGE(STATUS "Could not find msgfmt - skipping message catalogue compilation")
ENDIF(MSGFMT_EXECUTABLE)

################################################################################
# Set the platform
################################################################################
SET(_platform "unknown")
IF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    SET(_platform "osx")
ELSE(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
        IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
            SET(_platform "linux-x64")
        ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
            SET(_platform "linux")
        ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    ELSE(CMAKE_SYSTEM_NAME MATCHES "Linux")
        IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
            SET(_platform "windows")
        ELSE(CMAKE_SYSTEM_NAME MATCHES "Windows")
            MESSAGE(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
        ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")

ADD_DEFINITIONS("-DSTACKBUILDER_PLATFORM=wxT(\"${_platform}\")")

################################################################################
# Let's rock!
################################################################################
INCLUDE_DIRECTORIES(${stackbuilder_SOURCE_DIR}
                    ${stackbuilder_SOURCE_DIR}/include
                    ${wxWidgets_INCLUDE_DIRS} )

FILE(GLOB _cpp_files *.cpp)
FILE(GLOB _h_files include/*.h)
FILE(GLOB _xpm_files include/images/*.xpm)

SET(_srcs ${_cpp_files} ${_h_files} ${_xpm_files} ${_translations})
 
IF(WIN32)
    SET(_srcs ${_srcs} stackbuilder.rc)
ENDIF(WIN32)
 
ADD_EXECUTABLE(stackbuilder WIN32 MACOSX_BUNDLE ${_srcs})
TARGET_LINK_LIBRARIES(stackbuilder ${wxWidgets_LIBRARIES} )

IF(APPLE)
    CONFIGURE_FILE(include/images/StackBuilder.icns stackbuilder.app/Contents/Resources/StackBuilder.icns COPYONLY)
ENDIF(APPLE)

INSTALL(TARGETS stackbuilder DESTINATION bin)

################################################################################
# Build summary
################################################################################
MESSAGE(STATUS " ")

MESSAGE(STATUS "================================================================================")
MESSAGE(STATUS "Configuration summary:")
IF(${CONFIG_IS_UPTODATE})
    MESSAGE(STATUS "(Reusing previous identical config to save compilation time)")
ENDIF()
MESSAGE(STATUS " ")
MESSAGE(STATUS "  Project                     : ${PROJECT_NAME}")
MESSAGE(STATUS "  Description                 : ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
MESSAGE(STATUS "  Version                     : ${VERSION}")
MESSAGE(STATUS " ")
MESSAGE(STATUS "  Installer platform          : ${_platform}")
MESSAGE(STATUS " ")
MESSAGE(STATUS "  wxWidgets version           : ${wxWidgets_VERSION}")
MESSAGE(STATUS "  wxWidgets config binary     : ${wxWidgets_CONFIG_EXECUTABLE}")
MESSAGE(STATUS "  wxWidgets Static linking    : ${wxWidgets_USE_STATIC}")
MESSAGE(STATUS "  wxWidgets Unicode?          : ${wxWidgets_USE_UNICODE}")
MESSAGE(STATUS "  wxWidgets Debug?            : ${wxWidgets_USE_DEBUG}")
MESSAGE(STATUS "================================================================================")
MESSAGE(STATUS " ")

################################################################################
# Give ourselves a hint that we have cached values - must be last!
################################################################################
IF(NOT HAVE_CACHED_VALUES)
    SET(HAVE_CACHED_VALUES 1 CACHE INTERNAL "Flag to indicate that we have cached values")
ENDIF(NOT HAVE_CACHED_VALUES)


