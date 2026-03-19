/*
 * Minimal Wine compatibility layer for win-mingw builds.
 *
 * On real Windows, most of wine.c's stubs are unnecessary because the
 * real Windows APIs exist. This file only provides:
 *   - Wine debug infrastructure (__wine_dbg_*)
 *   - A few stubs for Wine-internal APIs that don't exist on Windows
 */

#define COBJMACROS

#include <ntstatus.h>
#define WIN32_NO_STATUS

#include <oleauto.h>
#include <winternl.h>
#include <rpcproxy.h>
#include "wine/debug.h"

void external_log_info(const char* format, ...);
void external_log_debug(const char* format, ...);
HRESULT WINAPI VBScriptFactory_CreateInstance(IClassFactory*,IUnknown*,REFIID,void**);

HINSTANCE hProxyDll = 0;

const WCHAR* system_dir = L".";

const ExtendedProxyFileInfo oleaut32_oaidl_ProxyFileInfo = { 0 };

/* --- Wine debug infrastructure --- */

struct debug_info
{
   unsigned int str_pos;
   unsigned int out_pos;
   char       strings[1020];
   char       output[1020];
} g_debug_info;

char dbg_buffer[4096] = { 0 };

int WINAPI __wine_dbg_write(const char *str, unsigned int len)
{
   size_t dest_len = strlen(dbg_buffer);
   size_t remaining_len = sizeof(dbg_buffer) - dest_len - 1;

   if (len < remaining_len)
      strncat(dbg_buffer, str, len);
   else {
      strncat(dbg_buffer, str, remaining_len);
      dbg_buffer[sizeof(dbg_buffer) - 1] = '\n';
   }

   char* end = strrchr(dbg_buffer, '\n');

   if (end) {
      *end = '\0';

#ifdef _DEBUG
      external_log_debug("%s", dbg_buffer);
#endif

      *dbg_buffer = '\0';
   }

   return 0;
}

static int append_output(struct debug_info *info, const char *str, size_t len)
{
   if (len >= sizeof(info->output) - info->out_pos) {
      __wine_dbg_write(info->output, info->out_pos);
      info->out_pos = 0;
      __wine_dbg_write(str, len);
   }
   memcpy(info->output + info->out_pos, str, len);
   info->out_pos += len;
   return len;
}

const char * __cdecl __wine_dbg_strdup(const char *str)
{
   struct debug_info *info = &g_debug_info;
   unsigned int pos = info->str_pos;
   size_t n = strlen(str) + 1;

   if (pos + n > sizeof(info->strings)) pos = 0;
   info->str_pos = pos + n;
   return memcpy(info->strings + pos, str, n);
}

int __cdecl __wine_dbg_output(const char *str)
{
   struct debug_info *info = &g_debug_info;
   const char *end = strrchr(str, '\n');
   int ret = 0;

   if (end) {
      ret += append_output(info, str, end + 1 - str);
      __wine_dbg_write(info->output, info->out_pos);
      info->out_pos = 0;
      str = end + 1;
   }
   if (*str) ret += append_output(info, str, strlen(str));
   return ret;
}

int __cdecl __wine_dbg_header(enum __wine_debug_class cls, struct __wine_debug_channel *channel,
                        const char *function)
{
   static const char * const classes[] = { "fixme", "err", "warn", "trace" };
   struct debug_info *info = &g_debug_info;
   char *pos = info->output;

   if (!(__wine_dbg_get_channel_flags(channel) & (1 << cls))) return -1;

   if (info->out_pos) return 0;

   if (function && cls < ARRAY_SIZE(classes))
      pos += snprintf(pos, sizeof(info->output) - (pos - info->output), "%s:%s:%s ",
                   classes[cls], channel->name, function);
   info->out_pos = pos - info->output;
   return info->out_pos;
}

unsigned char __cdecl __wine_dbg_get_channel_flags(struct __wine_debug_channel *channel)
{
   return 0;
}

/* --- Stubs for Wine-internal / unused APIs --- */

HRESULT WINAPI CLSIDFromProgID(LPCOLESTR progid, CLSID *clsid)
{
   /* Must succeed so create_object() reaches the __LIBVBSCRIPT__ dispatch
      block that routes to internal factories (FileSystemObject, Dictionary)
      or the external_create_object callback. The CLSID value is unused. */
   if (clsid)
      memset(clsid, 0, sizeof(*clsid));
   return S_OK;
}

HRESULT WINAPI CoGetClassObject(REFCLSID rclsid, DWORD clscontext, COSERVERINFO *server_info, REFIID riid, void **obj)
{
   /* Stub: the __LIBVBSCRIPT__ code path in create_object() doesn't use the
      class factory for known objects, and external_create_object handles the
      rest. We just need this to not fail so the dispatch logic is reached. */
   if (obj)
      *obj = NULL;
   return S_OK;
}

HRESULT WINAPI LoadRegTypeLib(REFGUID rguid, WORD wVerMajor, WORD wVerMinor, LCID lcid, ITypeLib **ppTLib)
{
   return E_NOTIMPL;
}

HRESULT WINAPI LoadTypeLib(const OLECHAR *szFile, ITypeLib * *pptLib)
{
   return E_NOTIMPL;
}

HRESULT WINAPI MkParseDisplayName(LPBC pbc, LPCOLESTR szDisplayName, LPDWORD pchEaten, LPMONIKER *ppmk)
{
   return E_NOTIMPL;
}

HRESULT WINAPI CreateStreamOnHGlobal(HGLOBAL hGlobal, BOOL fDeleteOnRelease, LPSTREAM* ppstm)
{
   return E_NOTIMPL;
}

HRESULT WINAPI CreateProxyFromTypeInfo(ITypeInfo *typeinfo, IUnknown *outer, REFIID iid, IRpcProxyBuffer **proxy, void **obj)
{
   return E_NOTIMPL;
}

HRESULT WINAPI CreateStubFromTypeInfo(ITypeInfo *typeinfo, REFIID iid, IUnknown *server, IRpcStubBuffer **stub)
{
   return E_NOTIMPL;
}

HRESULT WINAPI CreateItemMoniker(LPCOLESTR lpszDelim, LPCOLESTR lpszItem, LPMONIKER* ppmk)
{
   return E_NOTIMPL;
}

HRESULT WINAPI GetRunningObjectTable(DWORD reserved, LPRUNNINGOBJECTTABLE *pprot)
{
   return E_NOTIMPL;
}

HRESULT WINAPI OLEAUTPS_DllRegisterServer()
{
   return E_NOTIMPL;
}

HRESULT WINAPI OLEAUTPS_DllUnregisterServer()
{
   return E_NOTIMPL;
}

HRESULT WINAPI OLEAUTPS_DllGetClassObject(REFCLSID p1, REFIID p2, LPVOID * p3)
{
   return E_NOTIMPL;
}

INT WINAPI StringFromGUID2(REFGUID id, LPOLESTR str, INT cmax)
{
   return 0;
}

HRESULT WINAPI CoRegisterClassObject(REFCLSID rclsid,LPUNKNOWN pUnk,DWORD dwClsContext,DWORD flags,LPDWORD lpdwRegister)
{
   return E_NOTIMPL;
}

HRESULT WINAPI CoRevokeClassObject(DWORD dwRegister)
{
   return E_NOTIMPL;
}

HRESULT WINAPI UnRegisterTypeLib(REFGUID p1, WORD p2, WORD p3, LCID p4, SYSKIND p5)
{
   return E_NOTIMPL;
}

/* CStdStubBuffer stubs */

ULONG WINAPI CStdStubBuffer_AddRef(IRpcStubBuffer* This) { return 0; }
HRESULT WINAPI CStdStubBuffer_QueryInterface(IRpcStubBuffer* This, REFIID riid, void **ppvObject) { return E_NOTIMPL; }
HRESULT WINAPI CStdStubBuffer_Connect(IRpcStubBuffer* This, IUnknown *pUnkServer) { return E_NOTIMPL; }
void WINAPI CStdStubBuffer_Disconnect(IRpcStubBuffer* This) { }
HRESULT WINAPI CStdStubBuffer_Invoke(IRpcStubBuffer* This, RPCOLEMESSAGE* pRpcMsg, IRpcChannelBuffer* pRpcChannelBuffer) { return E_NOTIMPL; }
IRpcStubBuffer* WINAPI CStdStubBuffer_IsIIDSupported(IRpcStubBuffer* This, REFIID riid) { return NULL; }
ULONG WINAPI CStdStubBuffer_CountRefs(IRpcStubBuffer* This) { return 0; }
HRESULT WINAPI CStdStubBuffer_DebugServerQueryInterface(IRpcStubBuffer* This, void **ppv) { return E_NOTIMPL; }
void WINAPI CStdStubBuffer_DebugServerRelease(IRpcStubBuffer* This, void * pv) { }

/* Ntdll stubs for Wine's kernelbase/locale.c */

NTSTATUS WINAPI NtGetNlsSectionPtr(ULONG type, ULONG id, void *unknown, void **ptr, SIZE_T *size)
{
   return 0;
}

NTSTATUS WINAPI NtQueryDefaultLocale(BOOLEAN user, LCID *lcid)
{
   return 0;
}

NTSTATUS WINAPI NtQueryInstallUILanguage(LANGID *lang)
{
   return 0;
}

NTSTATUS WINAPI NtQuerySystemTime(PLARGE_INTEGER now)
{
   return 0;
}

NTSTATUS WINAPI RtlCompareUnicodeStrings(const WCHAR *s1, SIZE_T len1, const WCHAR *s2, SIZE_T len2, BOOLEAN case_insensitive)
{
   WCHAR wz1[len1 + 1];
   memset(wz1, 0, sizeof(wz1));
   memcpy(wz1, s1, len1 * sizeof(WCHAR));

   WCHAR wz2[len2 + 1];
   memset(wz2, 0, sizeof(wz2));
   memcpy(wz2, s2, len2 * sizeof(WCHAR));

   return case_insensitive ? wcsicmp(wz1, wz2) : wcscmp(wz1, wz2);
}

NTSTATUS WINAPI RtlEnterCriticalSection(RTL_CRITICAL_SECTION *crit)
{
   return 0;
}

NTSTATUS WINAPI RtlFindMessage(HMODULE hmod, ULONG type, ULONG lang, ULONG msg_id, const MESSAGE_RESOURCE_ENTRY **ret)
{
   return 0;
}

NTSTATUS WINAPI RtlFormatMessage(const WCHAR *src, ULONG width, BOOLEAN ignore_inserts,
                          BOOLEAN ansi, BOOLEAN is_array, __ms_va_list *args,
                          WCHAR *buffer, ULONG size, ULONG *retsize)
{
   return 0;
}

NTSTATUS WINAPI RtlGUIDFromString(PUNICODE_STRING str, GUID* guid)
{
   return 0;
}

NTSTATUS WINAPI RtlGetLocaleFileMappingAddress(void **ptr, LCID *lcid, LARGE_INTEGER *size)
{
   return 0;
}

NTSTATUS WINAPI RtlGetSystemPreferredUILanguages(DWORD flags, ULONG unknown, ULONG *count,
                                      WCHAR *buffer, ULONG *size)
{
   return 0;
}

NTSTATUS WINAPI RtlGetProcessPreferredUILanguages(DWORD flags, ULONG *count, WCHAR *buffer, ULONG *size)
{
   return 0;
}

NTSTATUS WINAPI RtlGetThreadPreferredUILanguages(DWORD flags, ULONG *count, WCHAR *buffer, ULONG *size)
{
   return 0;
}

NTSTATUS WINAPI RtlGetUserPreferredUILanguages(DWORD flags, ULONG unknown, ULONG *count,
                                    WCHAR *buffer, ULONG *size)
{
   return 0;
}

NTSTATUS WINAPI RtlIdnToAscii(DWORD flags, const WCHAR *src, INT srclen, WCHAR *dst, INT *dstlen)
{
   return 0;
}

NTSTATUS WINAPI RtlIdnToNameprepUnicode(DWORD flags, const WCHAR *src, INT srclen,
                               WCHAR *dst, INT *dstlen)
{
   return 0;
}

NTSTATUS WINAPI RtlIdnToUnicode(DWORD flags, const WCHAR *src, INT srclen, WCHAR *dst, INT *dstlen)
{
   return 0;
}

void WINAPI RtlInitCodePageTable(USHORT *ptr, CPTABLEINFO *info)
{
}

void WINAPI RtlInitUnicodeString(UNICODE_STRING *str, const WCHAR *data)
{
}

NTSTATUS WINAPI RtlIsNormalizedString(ULONG form, const WCHAR *str, INT len, BOOLEAN *res)
{
   return 0;
}

NTSTATUS WINAPI RtlLeaveCriticalSection(RTL_CRITICAL_SECTION *crit)
{
   return 0;
}

NTSTATUS WINAPI RtlMultiByteToUnicodeN(WCHAR *dst, DWORD dstlen, DWORD *reslen,
                              const char *src, DWORD srclen)
{
   return 0;
}

NTSTATUS WINAPI RtlNormalizeString(ULONG form, const WCHAR *src, INT src_len, WCHAR *dst, INT *dst_len)
{
   return 0;
}

ULONG WINAPI RtlNtStatusToDosError(NTSTATUS status)
{
   return 0;
}

NTSTATUS WINAPI RtlQueryTimeZoneInformation(RTL_TIME_ZONE_INFORMATION *ret)
{
   return 0;
}

NTSTATUS WINAPI RtlSetProcessPreferredUILanguages(DWORD flags, PCZZWSTR buffer, ULONG *count)
{
   return 0;
}

NTSTATUS WINAPI RtlSetThreadPreferredUILanguages(DWORD flags, PCZZWSTR buffer, ULONG *count)
{
   return 0;
}

NTSTATUS WINAPI RtlSetTimeZoneInformation(const RTL_TIME_ZONE_INFORMATION *tzinfo)
{
   return 0;
}

VOID WINAPI RtlTimeToTimeFields(const LARGE_INTEGER *liTime, PTIME_FIELDS TimeFields)
{
}

NTSTATUS WINAPI RtlQueryDynamicTimeZoneInformation(RTL_DYNAMIC_TIME_ZONE_INFORMATION* p1)
{
   return 0;
}

PVOID WINAPI RtlReAllocateHeap(HANDLE p1, ULONG p2, PVOID p3, SIZE_T p4)
{
   return 0;
}

/* RtlGenRandom */
BOOLEAN WINAPI SystemFunction036(PVOID p1, ULONG p2)
{
   return 0;
}

LSTATUS WINAPI RegLoadMUIStringW(HKEY p1, LPCWSTR p2, LPWSTR p3, DWORD p4, LPDWORD p5, DWORD p6, LPCWSTR p7)
{
   return 0;
}

#include <initguid.h>

DEFINE_GUID(CLSID_StdComponentCategoriesMgr, 0x0002e005, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
DEFINE_GUID(IID_ICatRegister, 0x0002e012, 0x0000, 0x0000, 0xc0,0x00, 0x00,0x00,0x00,0x00,0x00,0x46);
