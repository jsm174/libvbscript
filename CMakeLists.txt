cmake_minimum_required(VERSION 3.25)

set(PLATFORM "win" CACHE STRING "Platform")
set(ARCH "x64" CACHE STRING "Arch")

option(BUILD_SHARED "Option to build shared library" ON)
option(BUILD_STATIC "Option to build static library" ON)

message(STATUS "PLATFORM: ${PLATFORM}")
message(STATUS "ARCH: ${ARCH}")

message(STATUS "BUILD_SHARED: ${BUILD_SHARED}")
message(STATUS "BUILD_STATIC: ${BUILD_STATIC}")

if(PLATFORM STREQUAL "macos")
   set(CMAKE_OSX_DEPLOYMENT_TARGET 14.0)
elseif(PLATFORM STREQUAL "ios" OR PLATFORM STREQUAL "ios-simulator")
   set(CMAKE_SYSTEM_NAME iOS)
   if (PLATFORM STREQUAL "ios-simulator")
      set(CMAKE_OSX_SYSROOT iphonesimulator)
   endif()
   set(CMAKE_OSX_DEPLOYMENT_TARGET 17.0)
elseif(PLATFORM STREQUAL "tvos")
   set(CMAKE_SYSTEM_NAME tvOS)
   set(CMAKE_OSX_DEPLOYMENT_TARGET 17.0)
elseif(PLATFORM STREQUAL "android")
   set(CMAKE_SYSTEM_NAME Android)
   set(CMAKE_SYSTEM_VERSION 30)
   set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
endif()

project(libvbscript VERSION 0.1.0 LANGUAGES C)

if(PLATFORM STREQUAL "win")
   if(ARCH STREQUAL "x86")
      add_compile_definitions(WIN32)
   endif()
elseif(PLATFORM STREQUAL "macos")
   if (ARCH STREQUAL "arm64")
      set(CMAKE_OSX_ARCHITECTURES arm64)
   elseif(ARCH STREQUAL "x64")
      set(CMAKE_OSX_ARCHITECTURES x86_64)
   endif()
   set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
   set(CMAKE_INSTALL_RPATH "@executable_path")
elseif(PLATFORM STREQUAL "ios" OR PLATFORM STREQUAL "ios-simulator" OR PLATFORM STREQUAL "tvos")
   set(CMAKE_OSX_ARCHITECTURES arm64)
elseif(PLATFORM STREQUAL "linux" OR PLATFORM STREQUAL "android")
   set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
   set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_VISIBILITY_PRESET hidden)

if(MINGW)
   find_program(GENDEF gendef)
   find_program(DLLTOOL dlltool)
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

   wine/dlls/atl/atl.c

   wine/dlls/kernelbase/locale.c
)

set(LIBVBSCRIPT_INCLUDE_DIRS
   ${CMAKE_SOURCE_DIR}/include
   ${CMAKE_SOURCE_DIR}/wine/include
   ${CMAKE_SOURCE_DIR}/atl/include
   ${CMAKE_SOURCE_DIR}/atlmfc/include
)

set(LIBVBSCRIPT_PRIVATE_INCLUDE_DIRS
   ${CMAKE_SOURCE_DIR}/wine/dlls/vbscript
   ${CMAKE_SOURCE_DIR}/wine/dlls/oleaut32
   ${CMAKE_SOURCE_DIR}/wine/dlls/kernelbase
   ${CMAKE_SOURCE_DIR}/wine/dlls/scrrun
   ${CMAKE_SOURCE_DIR}/wine/dlls/atl
)

set(LIBVBSCRIPT_COMPILE_DEFINITIONS
   __LIBVBSCRIPT__
   LIBVBSCRIPT_EXPORT
   __WINESRC__
   WINE_UNICODE_NATIVE
   WINE_NO_TRACE_MSGS
   WINE_NO_DEBUG_MSGS
   "__forceinline=__attribute__((always_inline)) inline"
)

set(LIBVBSCRIPT_COMPILE_OPTIONS
   -Wno-format
   -fvisibility=default
)

if(PLATFORM STREQUAL "win" AND ARCH STREQUAL "x64")
   set(LIBVBSCRIPT_OUTPUT_NAME "vbscript64")
else()
   set(LIBVBSCRIPT_OUTPUT_NAME "vbscript")
endif()

if(BUILD_SHARED)
   add_library(vbscript_shared SHARED ${LIBVBSCRIPT_SOURCES})

   target_include_directories(vbscript_shared PUBLIC ${LIBVBSCRIPT_INCLUDE_DIRS} PRIVATE ${LIBVBSCRIPT_PRIVATE_INCLUDE_DIRS})
   target_compile_definitions(vbscript_shared PRIVATE ${LIBVBSCRIPT_COMPILE_DEFINITIONS})
   target_compile_options(vbscript_shared PRIVATE ${LIBVBSCRIPT_COMPILE_OPTIONS})

   if(UNIX AND NOT APPLE)
      target_link_libraries(vbscript_shared PRIVATE m)
   endif()

   if(MINGW)
      target_link_options(vbscript_shared PRIVATE -Wl,--allow-multiple-definition)
      if(GENDEF AND DLLTOOL)
         add_custom_command(TARGET vbscript_shared POST_BUILD
            COMMAND ${GENDEF} "$<TARGET_FILE:vbscript_shared>"
            COMMAND ${DLLTOOL} -d "${LIBVBSCRIPT_OUTPUT_NAME}.def" -l "${LIBVBSCRIPT_OUTPUT_NAME}.lib" -D "$<TARGET_FILE_NAME:vbscript_shared>"
            WORKING_DIRECTORY "$<TARGET_FILE_DIR:vbscript_shared>"
         )
      endif()
   endif()

   if(WIN32)
      set_target_properties(vbscript_shared PROPERTIES PREFIX "")
   endif()

   set_target_properties(vbscript_shared PROPERTIES
      OUTPUT_NAME ${LIBVBSCRIPT_OUTPUT_NAME}
      VERSION ${PROJECT_VERSION}
      SOVERSION 0
   )

   install(TARGETS vbscript_shared
      LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
   )
   install(FILES ${CMAKE_SOURCE_DIR}/include/libvbscript.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
   install(DIRECTORY ${CMAKE_SOURCE_DIR}/wine/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

   if(PLATFORM STREQUAL "win" OR PLATFORM STREQUAL "macos" OR PLATFORM STREQUAL "linux")
      add_executable(vbscript_test src/vbscript_test.c)
      target_link_libraries(vbscript_test PRIVATE vbscript_shared)
      target_compile_definitions(vbscript_test PRIVATE
         __LIBVBSCRIPT__
         __WINESRC__
         WINE_UNICODE_NATIVE
      )
   endif()
endif()

if(BUILD_STATIC)
   add_library(vbscript_static STATIC ${LIBVBSCRIPT_SOURCES})

   target_include_directories(vbscript_static PUBLIC ${LIBVBSCRIPT_INCLUDE_DIRS} PRIVATE ${LIBVBSCRIPT_PRIVATE_INCLUDE_DIRS})
   target_compile_definitions(vbscript_static PRIVATE ${LIBVBSCRIPT_COMPILE_DEFINITIONS})
   target_compile_options(vbscript_static PRIVATE ${LIBVBSCRIPT_COMPILE_OPTIONS})

   if(PLATFORM STREQUAL "win")
      set_target_properties(vbscript_static PROPERTIES
         OUTPUT_NAME "vbscript_static"
      )
   else()
      set_target_properties(vbscript_static PROPERTIES
         OUTPUT_NAME "vbscript"
      )
   endif()

   install(TARGETS vbscript_static
      LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
   )
   install(FILES ${CMAKE_SOURCE_DIR}/include/libvbscript.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
   install(DIRECTORY ${CMAKE_SOURCE_DIR}/wine/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

endif()
