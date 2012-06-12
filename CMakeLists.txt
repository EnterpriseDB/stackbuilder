################################################################################
# Initial setup
################################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

IF(COMMAND cmake_policy)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

SET(CMAKE_FIND_LIBRARY_PREFIXES "")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")

################################################################################
# Apple stuff
################################################################################
IF(APPLE)
    # Setup default values
    IF(NOT HAVE_CACHED_VALUES)
        IF(EXISTS /Developer/SDKs/MacOSX10.6.sdk)
	    SET(CMAKE_OSX_ARCHITECTURES "i386" CACHE STRING "Build architectures for OSX" FORCE)
            SET(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.6.sdk" CACHE FILEPATH "isysroot used for universal binary support" FORCE)
	ELSE()
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
        ENDIF(EXISTS /Developer/SDKs/MacOSX10.6.sdk)
    ENDIF(NOT HAVE_CACHED_VALUES)

    # Target Tiger
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.4")
ENDIF(APPLE)

################################################################################
# Project config
################################################################################
PROJECT(stackbuilder)

SET(VERSION 3.1.0)

# CPack stuff
SET(CPACK_PACKAGE_VERSION_MAJOR 3)
SET(CPACK_PACKAGE_VERSION_MINOR 1)
SET(CPACK_PACKAGE_VERSION_PATCH 0)
SET(CPACK_PACKAGE_NAME "StackBuilder")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "PostgreSQL StackBuilder")
SET(CPACK_PACKAGE_VENDOR "EnterpriseDB")
SET(CPACK_GENERATOR TGZ)
SET(CPACK_SOURCE_GENERATOR TGZ)
SET(CPACK_SOURCE_IGNORE_FILES "\\\\.DS_Store;/CVS/;/\\\\.svn/;\\\\.swp$;\\\\.#;/#;.*~;cscope.*")

# This must come after we set the CPACK variables!!
INCLUDE(CPack)

# OS X bundle stuff
SET(MACOSX_BUNDLE_INFO_STRING "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
SET(MACOSX_BUNDLE_ICON_FILE "StackBuilder.icns")
SET(MACOSX_BUNDLE_GUI_IDENTIFIER "com.enterprisedb.stackbuilder")
SET(MACOSX_BUNDLE_VERSION "${VERSION}")
SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION}")
SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION}")
SET(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}")
SET(MACOSX_BUNDLE_COPYRIGHT "Copyright 2007 - 2011, EnterpriseDB")

################################################################################
# Find wxWidgets
################################################################################
SET(WX_DEBUG NO CACHE BOOL "Use the debug build of wxWidgets?")
SET(WX_STATIC YES CACHE BOOL "Use the statically linked build of wxWidgets?")
SET(WX_VERSION "2.8")
SET(WX_UNICODE YES)
SET(WX_MODULES "adv;core;base;net;xml;")

FIND_PACKAGE(WX REQUIRED)

INCLUDE_DIRECTORIES(${WX_INCLUDE_DIRS} ${WX_ROOT_DIR}/include/msvc)
ADD_DEFINITIONS(${WX_DEFINITIONS})
LINK_DIRECTORIES(${WX_LIBRARY_DIRS})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WX_CXX_FLAGS}")

################################################################################
# Compile translations
################################################################################
FIND_PROGRAM(MSGFMT_EXECUTABLE msgfmt)

# Add a custom output for each po file we need to compile
IF(MSGFMT_EXECUTABLE)

    FILE(GLOB_RECURSE _pofiles ${stackbuilder_SOURCE_DIR}/i18n/??_??/StackBuilder.po)

    FOREACH(_pofile ${_pofiles})
        GET_FILENAME_COMPONENT(_dir ${_pofile} PATH)
        GET_FILENAME_COMPONENT(_lang ${_dir} NAME)

        SET(_out "${stackbuilder_BINARY_DIR}/CMakeFiles/stackbuilder.dir/StackBuilder.${_lang}.mo")

        IF(APPLE)

            # Install the translations into the app bundle
            ADD_CUSTOM_COMMAND(
                OUTPUT ${_out}
                COMMAND ${MSGFMT_EXECUTABLE} -o ${_out} ${_pofile}
                COMMAND mkdir -p ${stackbuilder_BINARY_DIR}/stackbuilder.app/Contents/Resources/i18n/${_lang}
                COMMAND cp ${_out} ${stackbuilder_BINARY_DIR}/stackbuilder.app/Contents/Resources/i18n/${_lang}/StackBuilder.mo
                COMMAND cp ${_dir}/wxstd.mo ${stackbuilder_BINARY_DIR}/stackbuilder.app/Contents/Resources/i18n/${_lang}/wxstd.mo 
                DEPENDS ${_pofile}
            )

        ELSE(APPLE)

            # Install everything the 'normal' way
            ADD_CUSTOM_COMMAND(
                OUTPUT ${_out}
                COMMAND ${MSGFMT_EXECUTABLE} -o ${_out} ${_pofile}
                DEPENDS ${_pofile}
            )

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

        ENDIF(APPLE)

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
            IF(CMAKE_CL_64)
                SET(_platform "windows-x64")
            ELSE(CMAKE_CL_64)
                SET(_platform "windows")
            ENDIF(CMAKE_CL_64)
        ELSE(CMAKE_SYSTEM_NAME MATCHES "Windows")
            MESSAGE(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
        ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")

ADD_DEFINITIONS("-DSTACKBUILDER_PLATFORM=wxT(\"${_platform}\")")

################################################################################
# Let's rock!
################################################################################
ADD_DEFINITIONS("-DSTACKBUILDER_VERSION=wxT(\"${VERSION}\")")
INCLUDE_DIRECTORIES(${stackbuilder_SOURCE_DIR} ${stackbuilder_SOURCE_DIR}/include)

FILE(GLOB _cpp_files *.cpp)
FILE(GLOB _h_files include/*.h)
FILE(GLOB _xpm_files include/images/*.xpm)

SET(_srcs ${_cpp_files} ${_h_files} ${_xpm_files} ${_translations})
 
IF(WIN32)
    SET(_srcs ${_srcs} stackbuilder.rc)
ENDIF(WIN32)
 
ADD_EXECUTABLE(stackbuilder WIN32 MACOSX_BUNDLE ${_srcs})
TARGET_LINK_LIBRARIES(stackbuilder ${WX_LIBRARIES})

IF(WIN32)
    IF (MS_VS_10)
        SET_TARGET_PROPERTIES(stackbuilder PROPERTIES LINK_FLAGS "/level='requireAdministrator' /uiAccess='false'")
    ELSE()
        SET_TARGET_PROPERTIES(stackbuilder PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
    ENDIF(MS_VS_10)
ENDIF(WIN32)

IF(APPLE)
    CONFIGURE_FILE(${stackbuilder_SOURCE_DIR}/include/images/StackBuilder.icns ${stackbuilder_BINARY_DIR}/stackbuilder.app/Contents/Resources/StackBuilder.icns COPYONLY)
ENDIF(APPLE)

INSTALL(TARGETS stackbuilder DESTINATION bin)

################################################################################
# Build summary
################################################################################
MESSAGE(STATUS " ")

MESSAGE(STATUS "================================================================================")
MESSAGE(STATUS "Configuration summary:")
MESSAGE(STATUS " ")
MESSAGE(STATUS "  Project                     : ${PROJECT_NAME}")
MESSAGE(STATUS "  Description                 : ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
MESSAGE(STATUS "  Version                     : ${VERSION}")
MESSAGE(STATUS " ")
MESSAGE(STATUS "  Installer platform          : ${_platform}")
MESSAGE(STATUS " ")
MESSAGE(STATUS "  wxWidgets version           : ${WX_VERSION}")
MESSAGE(STATUS "  wxWidgets directory         : ${WX_ROOT_DIR}")
MESSAGE(STATUS "  wxWidgets static linking    : ${WX_STATIC}")
MESSAGE(STATUS "  wxWidgets unicode?          : ${WX_UNICODE}")
MESSAGE(STATUS "  wxWidgets debug?            : ${WX_DEBUG}")
MESSAGE(STATUS "================================================================================")
MESSAGE(STATUS " ")

################################################################################
# Give ourselves a hint that we have cached values - must be last!
################################################################################
IF(NOT HAVE_CACHED_VALUES)
    SET(HAVE_CACHED_VALUES 1 CACHE INTERNAL "Flag to indicate that we have cached values")
ENDIF(NOT HAVE_CACHED_VALUES)


