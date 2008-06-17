################################################################################
# Initial setup
################################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

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

SET(VERSION 1.0.0)
SET(CPACK_PACKAGE_VERSION_MAJOR 1)
SET(CPACK_PACKAGE_VERSION_MINOR 0)
SET(CPACK_PACKAGE_VERSION_PATCH 0)
SET(CPACK_PACKAGE_NAME "StackBuilder")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "PostgreSQL StackBuilder")
SET(CPACK_PACKAGE_VENDOR "EnterpriseDB UK")
SET(CPACK_GENERATOR TGZ)
SET(CPACK_SOURCE_GENERATOR TGZ)
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION}")
SET(CPACK_SOURCE_IGNORE_FILES "${PROJECT_NAME}-${VERSION}.tar.gz" "_CPack_Packages")

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

# Get the wxWidgets version
EXECUTE_PROCESS(
    COMMAND sh "${wxWidgets_CONFIG_EXECUTABLE}" --version
    OUTPUT_VARIABLE wxWidgets_VERSION
    RESULT_VARIABLE RET
    ERROR_QUIET
)
STRING(STRIP ${wxWidgets_VERSION} wxWidgets_VERSION)

################################################################################
# Let's rock!
################################################################################
INCLUDE_DIRECTORIES(${stackbuilder_SOURCE_DIR}
                    ${stackbuilder_SOURCE_DIR}/include
                    ${wxWidgets_INCLUDE_DIRS} )

FILE(GLOB _cpp_files *.cpp)
FILE(GLOB _h_files include/*.h)
FILE(GLOB _xpm_files include/images/*.xpm)

SET(_srcs ${_cpp_files} ${_h_files} ${_xpm_files})
 
IF(WIN32)
    SET(_srcs ${_srcs} stackbuilder.rc)
ENDIF(WIN32)
 
ADD_EXECUTABLE(stackbuilder WIN32 MACOSX_BUNDLE ${_srcs})
TARGET_LINK_LIBRARIES(stackbuilder ${wxWidgets_LIBRARIES} )

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


