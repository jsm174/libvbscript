cmake_minimum_required(VERSION 3.25)

project(libvbscript VERSION 0.1.0 LANGUAGES C)

set(PLATFORM "" CACHE STRING "Platform")
set(ARCH "" CACHE STRING "Arch")

option(BUILD_SHARED "Build shared library" ON)

if(BUILD_SHARED)
   set(BUILD_SHARED_LIBS ON)
else()
   set(BUILD_SHARED_LIBS OFF)
endif()

set(CMAKE_C_STANDARD 99)

if(MINGW)
   find_program(GENDEF gendef)
   find_program(DLLTOOL dlltool)
endif()

if(APPLE)
   set(CMAKE_OSX_DEPLOYMENT_TARGET 14.0)
endif()

find_package(BISON 3.8.2 REQUIRED)
bison_target(vbsparser
   ${CMAKE_SOURCE_DIR}/wine/dlls/vbscript/parser.y
   ${CMAKE_SOURCE_DIR}/wine/dlls/vbscript/parser.tab.c
   DEFINES_FILE ${CMAKE_SOURCE_DIR}/wine/dlls/vbscript/parser.tab.h
)

set(LIBVBSCRIPT_SOURCES
   src/libvbscript.c
   src/wine.c

   wine/dlls/atl/atl.c

   wine/dlls/oleaut32/oleaut.c
   wine/dlls/oleaut32/variant.c
   wine/dlls/oleaut32/varformat.c
   wine/dlls/oleaut32/vartype.c
   wine/dlls/oleaut32/safearray.c

   wine/dlls/vbscript/compile.c
   wine/dlls/vbscript/global.c
   wine/dlls/vbscript/interp.c
   wine/dlls/vbscript/lex.c
   wine/dlls/vbscript/regexp.c
   wine/dlls/vbscript/utils.c
   wine/dlls/vbscript/vbdisp.c
   wine/dlls/vbscript/vbregexp.c
   wine/dlls/vbscript/vbscript.c
   wine/dlls/vbscript/vbscript_main.c
   wine/dlls/vbscript/parser.tab.c

   wine/dlls/scrrun/dictionary.c
   wine/dlls/scrrun/filesystem.c
   wine/dlls/scrrun/scrrun.c

   wine/dlls/kernelbase/locale.c
)

add_library(vbscript ${LIBVBSCRIPT_SOURCES})

target_include_directories(vbscript
   PUBLIC
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/wine/include>
      $<INSTALL_INTERFACE:include/libvbscript>
   PRIVATE
      ${CMAKE_SOURCE_DIR}/wine/dlls/atl
      ${CMAKE_SOURCE_DIR}/wine/dlls/vbscript
      ${CMAKE_SOURCE_DIR}/wine/dlls/oleaut32
      ${CMAKE_SOURCE_DIR}/wine/dlls/kernelbase
      ${CMAKE_SOURCE_DIR}/wine/dlls/scrrun
)

target_compile_definitions(vbscript PRIVATE
   __LIBVBSCRIPT__
   LIBVBSCRIPT_EXPORT

   __WINESRC__
   WINE_UNICODE_NATIVE

   WINE_NO_TRACE_MSGS
   WINE_NO_DEBUG_MSGS

   "__forceinline=__attribute__((always_inline)) inline"
)

target_compile_options(vbscript PRIVATE
   -Wno-format
   -fvisibility=default
)

if(APPLE)
   set_target_properties(vbscript PROPERTIES
      INSTALL_NAME_DIR "@rpath"
      BUILD_WITH_INSTALL_RPATH TRUE
   )
endif()

if(PLATFORM STREQUAL "win" AND ARCH STREQUAL "x64")
   set(LIBVBSCRIPT_OUTPUT_NAME "vbscript64")
else()
   set(LIBVBSCRIPT_OUTPUT_NAME "vbscript")
endif()

if(MINGW)
   target_link_options(vbscript PRIVATE -Wl,--allow-multiple-definition)
   if(GENDEF AND DLLTOOL)
      add_custom_command(TARGET vbscript POST_BUILD
         COMMAND ${GENDEF} "$<TARGET_FILE:vbscript>"
         COMMAND ${DLLTOOL} -d "${LIBVBSCRIPT_OUTPUT_NAME}.def" -l "${LIBVBSCRIPT_OUTPUT_NAME}.lib" -D "$<TARGET_FILE_NAME:vbscript>"
         WORKING_DIRECTORY "$<TARGET_FILE_DIR:vbscript>"
      )
   endif()
endif()

if(WIN32)
   set_target_properties(vbscript PROPERTIES PREFIX "")
endif()

set_target_properties(vbscript PROPERTIES
   OUTPUT_NAME ${LIBVBSCRIPT_OUTPUT_NAME}
   VERSION ${PROJECT_VERSION}
   SOVERSION 0
   PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/libvbscript.h"
)

include(GNUInstallDirs)

install(TARGETS vbscript
   EXPORT libvbscript-targets
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libvbscript
)

install(DIRECTORY wine/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libvbscript)

install(EXPORT libvbscript-targets
   FILE libvbscript-config.cmake
   NAMESPACE libvbscript::
   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libvbscript
)
