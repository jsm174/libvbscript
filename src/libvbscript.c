/*
 * libvbscript.c uses Wine headers internally (it's part of the DLL),
 * but the public API in libvbscript.h conditionally uses real Windows
 * headers on MinGW. Since this file is compiled with Wine's include
 * dirs in the private include path, it gets Wine's versions.
 */

/* Force Wine headers for internal compilation */
#include "windef.h"
#include "winbase.h"
#include "ole2.h"
#include "activscp.h"

#include <stdarg.h>
#include <string.h>

/* Re-declare the public types/API manually to avoid header conflicts */

typedef enum {
   LIBVBSCRIPT_LOG_INFO,
   LIBVBSCRIPT_LOG_DEBUG,
   LIBVBSCRIPT_LOG_ERROR
} libvbscript_log_level_t;

typedef struct {
   void (*log)(libvbscript_log_level_t level, const char* format, va_list args);
   HRESULT (*create_object)(const WCHAR* progid, IClassFactory* cf, IUnknown* obj);
} libvbscript_callbacks_t;

#ifdef _WIN32
   #define LIBVBSCRIPT_API __declspec(dllexport)
#else
   #define LIBVBSCRIPT_API __attribute__((visibility("default")))
#endif

HRESULT WINAPI VBScriptFactory_CreateInstance(IClassFactory*,IUnknown*,REFIID,void**);

static libvbscript_callbacks_t g_callbacks = { 0 };

LIBVBSCRIPT_API void libvbscript_init(const libvbscript_callbacks_t* callbacks)
{
   if (callbacks)
      g_callbacks = *callbacks;
}

LIBVBSCRIPT_API void libvbscript_shutdown(void)
{
   memset(&g_callbacks, 0, sizeof(g_callbacks));
}

LIBVBSCRIPT_API HRESULT libvbscript_create_engine(REFIID riid, void** ppv)
{
   if (!ppv)
      return E_POINTER;

   return VBScriptFactory_CreateInstance(NULL, NULL, riid, ppv);
}

HRESULT external_create_object(const WCHAR* progid, IClassFactory* cf, IUnknown* obj)
{
   if (g_callbacks.create_object)
      return g_callbacks.create_object(progid, cf, obj);

   return CLASS_E_CLASSNOTAVAILABLE;
}

void external_log_info(const char* format, ...)
{
   if (g_callbacks.log) {
      va_list args;
      va_start(args, format);
      g_callbacks.log(LIBVBSCRIPT_LOG_INFO, format, args);
      va_end(args);
   }
}

void external_log_debug(const char* format, ...)
{
   if (g_callbacks.log) {
      va_list args;
      va_start(args, format);
      g_callbacks.log(LIBVBSCRIPT_LOG_DEBUG, format, args);
      va_end(args);
   }
}

void external_log_error(const char* format, ...)
{
   if (g_callbacks.log) {
      va_list args;
      va_start(args, format);
      g_callbacks.log(LIBVBSCRIPT_LOG_ERROR, format, args);
      va_end(args);
   }
}
