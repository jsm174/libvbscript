#ifndef LIBVBSCRIPT_H
#define LIBVBSCRIPT_H

#ifndef __LIBVBSCRIPT__
#define __LIBVBSCRIPT__
#endif

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "ole2.h"
#include "activscp.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
   #ifdef LIBVBSCRIPT_EXPORT
      #define LIBVBSCRIPT_API __declspec(dllexport)
   #else
      #define LIBVBSCRIPT_API __declspec(dllimport)
   #endif
#else
   #ifdef LIBVBSCRIPT_EXPORT
      #define LIBVBSCRIPT_API __attribute__((visibility("default")))
   #else
      #define LIBVBSCRIPT_API
   #endif
#endif

typedef enum {
   LIBVBSCRIPT_LOG_INFO,
   LIBVBSCRIPT_LOG_DEBUG,
   LIBVBSCRIPT_LOG_ERROR
} libvbscript_log_level_t;

typedef struct {
   void (*log)(libvbscript_log_level_t level, const char* format, va_list args);
   HRESULT (*create_object)(const WCHAR* progid, IClassFactory* cf, IUnknown* obj);
} libvbscript_callbacks_t;

LIBVBSCRIPT_API void libvbscript_init(const libvbscript_callbacks_t* callbacks);
LIBVBSCRIPT_API void libvbscript_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif
