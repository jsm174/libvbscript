#include "libvbscript.h"

static libvbscript_callbacks_t g_callbacks = { 0 };

void libvbscript_init(const libvbscript_callbacks_t* callbacks)
{
   if (callbacks)
      g_callbacks = *callbacks;
}

void libvbscript_shutdown(void)
{
   memset(&g_callbacks, 0, sizeof(g_callbacks));
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
