#define COBJMACROS

#include <ntstatus.h>
#define WIN32_NO_STATUS

#include <oleauto.h>
#include <winternl.h>
#include <winuser.h>
#include <rpcproxy.h>
#include "wine/debug.h"

#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "vbscript.h"

#undef wcsncpy

HRESULT external_open_storage(const OLECHAR* pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage **ppstgOpen);

HINSTANCE hProxyDll = 0;

const WCHAR* system_dir = L".";

const ExtendedProxyFileInfo oleaut32_oaidl_ProxyFileInfo = { 0 };

struct debug_info
{
   unsigned int str_pos;      /* current position in strings buffer */
   unsigned int out_pos;      /* current position in output buffer */
   char       strings[1020]; /* buffer for temporary strings */
   char       output[1020];  /* current output line */
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
   //if (!strcmp(channel->name, "vbscript"))
   //{
   //   return (1 << __WINE_DBCL_ERR) | (1 << __WINE_DBCL_TRACE) | (1 << __WINE_DBCL_FIXME);
   //}

   //if (!strcmp(channel->name, "variant"))
   //{
   //   return (1 << __WINE_DBCL_ERR) | (1 << __WINE_DBCL_TRACE) | (1 << __WINE_DBCL_FIXME);
   //}

   //if (!strcmp(channel->name, "scrrun"))
   //{
   //   return (1 << __WINE_DBCL_ERR) | (1 << __WINE_DBCL_TRACE) | (1 << __WINE_DBCL_FIXME);
   //}

   return 0;
}

HRESULT WINAPI CLSIDFromProgID(LPCOLESTR progid, CLSID *clsid)
{
   return 0;
}

HRESULT WINAPI CreateBindCtx(DWORD reserved, IBindCtx **bind_context)
{
   return 0;
}

BOOL WINAPI DisableThreadLibraryCalls(HMODULE module)
{
   return FALSE;
}

void WINAPI GetLocalTime(SYSTEMTIME *systime)
{
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    long milliseconds = current_time.tv_usec / 1000;

    time_t current_seconds = current_time.tv_sec;
    struct tm* local_time = localtime(&current_seconds);

    systime->wYear = local_time->tm_year + 1900;
    systime->wMonth = local_time->tm_mon + 1;
    systime->wDayOfWeek = local_time->tm_wday;
    systime->wDay = local_time->tm_mday;
    systime->wHour = local_time->tm_hour;
    systime->wMinute = local_time->tm_min;
    systime->wSecond = local_time->tm_sec;
    systime->wMilliseconds = milliseconds;
}

BOOL WINAPI IsBadStringPtrA(LPCSTR str, UINT_PTR max)
{
   return FALSE;
}

BOOL WINAPI IsBadStringPtrW(LPCWSTR str, UINT_PTR max)
{
   return FALSE;
}

HRESULT WINAPI LoadRegTypeLib(REFGUID rguid, WORD wVerMajor, WORD wVerMinor, LCID lcid, ITypeLib **ppTLib)
{
   return 0;
}

HGLOBAL WINAPI LoadResource(HINSTANCE module, HRSRC rsrc)
{
   return 0;
}

LPVOID WINAPI LockResource(HGLOBAL handle)
{
   return NULL;
}

HRSRC WINAPI FindResourceExW(HMODULE module, LPCWSTR type, LPCWSTR name, WORD lang)
{
   return 0;
}

INT WINAPI LoadStringW(HINSTANCE instance, UINT resource_id, LPWSTR buffer, INT buflen)
{
   return 0;
}

HRESULT WINAPI LoadTypeLib(const OLECHAR *szFile, ITypeLib * *pptLib)
{
   return 0;
}

INT WINAPI MessageBoxW(HWND hwnd, LPCWSTR text, LPCWSTR title, UINT type)
{
   return 0;
}

INT WINAPI ShowCursor(BOOL bShow)
{
   static int ref = 0;

   if (bShow)
      ref++;
   else
      ref--;

   return ref;
}

HRESULT WINAPI MkParseDisplayName(LPBC pbc, LPCOLESTR szDisplayName, LPDWORD pchEaten, LPMONIKER *ppmk)
{
   return 0;
}

void WINAPI GetSystemTime(SYSTEMTIME *systime)
{
}

DWORD WINAPI GetVersion(void)
{
   return 0;
}

HGLOBAL WINAPI GlobalAlloc(UINT flags, SIZE_T size)
{
   return malloc(size);
}

HGLOBAL WINAPI GlobalFree(HGLOBAL hMem)
{
   free(hMem);
   return NULL;
}

LPVOID WINAPI GlobalLock(HGLOBAL handle)
{
   return handle;
}

BOOL WINAPI GlobalUnlock(HGLOBAL handle)
{
   return TRUE;
}

HRESULT WINAPI CreateStreamOnHGlobal(HGLOBAL hGlobal, BOOL fDeleteOnRelease, LPSTREAM* ppstm)
{
   return S_OK;
}

void* WINAPI HeapAlloc(HANDLE heap, ULONG flags, SIZE_T size)
{
   return flags & HEAP_ZERO_MEMORY ? calloc(size, sizeof(char*)) : malloc(size);
}

void* WINAPI HeapReAlloc(HANDLE heap, ULONG flags, void *ptr, SIZE_T size)
{
   return realloc(ptr, size);
}

BOOL WINAPI HeapFree(HANDLE heap, ULONG flags, void *ptr)
{
   free(ptr);

   return TRUE;
}

LPVOID WINAPI CoTaskMemAlloc(SIZE_T size)
{
   return malloc(size);
}

LPVOID WINAPI CoTaskMemRealloc(LPVOID ptr, SIZE_T size)
{
   return NULL;
}

void WINAPI CoTaskMemFree(LPVOID ptr)
{
   free(ptr);
}

HRESULT WINAPI CoGetMalloc(DWORD dwMemContext, LPMALLOC* lpMalloc)
{
   return 0;
}

HRESULT WINAPI CoGetClassObject(REFCLSID rclsid, DWORD clscontext, COSERVERINFO *server_info, REFIID riid, void **obj)
{
   return 0;
}

HRESULT WINAPI CoCreateInstance(REFCLSID rclsid, IUnknown *outer, DWORD cls_context, REFIID riid, void **obj)
{
   MULTI_QI multi_qi = { .pIID = riid };
   HRESULT hr;

   if (!obj)
      return E_POINTER;

   hr = CoCreateInstanceEx(rclsid, outer, cls_context, NULL, 1, &multi_qi);
   *obj = multi_qi.pItf;
   return hr;
}

static void init_multi_qi(DWORD count, MULTI_QI *mqi, HRESULT hr)
{
   ULONG i;

   for (i = 0; i < count; i++)
   {
      mqi[i].pItf = NULL;
      mqi[i].hr = hr;
   }
}

static HRESULT return_multi_qi(IUnknown *unk, DWORD count, MULTI_QI *mqi, BOOL include_unk)
{
   ULONG index = 0, fetched = 0;

   if (include_unk) {
      mqi[0].hr = S_OK;
      mqi[0].pItf = unk;
      index = fetched = 1;
   }

   for (; index < count; index++) {
      mqi[index].hr = IUnknown_QueryInterface(unk, mqi[index].pIID, (void **)&mqi[index].pItf);
      if (mqi[index].hr == S_OK)
         fetched++;
   }

   if (!include_unk)
      IUnknown_Release(unk);

   if (fetched == 0)
      return E_NOINTERFACE;

   return fetched == count ? S_OK : CO_S_NOTALLINTERFACES;
}

static HRESULT com_get_class_object(REFCLSID rclsid, DWORD clscontext, COSERVERINFO *server_info, REFIID riid, void **obj)
{
   return 0;
}

HRESULT WINAPI CoCreateInstanceEx(REFCLSID rclsid, IUnknown *outer, DWORD cls_context, COSERVERINFO *server_info, ULONG count, MULTI_QI *results)
{
   IClassFactory *factory;
   IUnknown *unk = NULL;
   HRESULT hr;

   if (!count || !results)
      return E_INVALIDARG;

   init_multi_qi(count, results, E_NOINTERFACE);

   hr = VBScriptFactory_CreateInstance(factory, outer, results[0].pIID, (void **)&unk);

   if (FAILED(hr))
      return hr;

   return return_multi_qi(unk, count, results, TRUE);
}

BOOL WINAPI FileTimeToSystemTime(const FILETIME *ft, SYSTEMTIME *systime)
{
   return FALSE;
}

DWORD WINAPI GetEnvironmentVariableW(LPCWSTR name, LPWSTR val, DWORD size)
{
   return 0;
}

HMODULE WINAPI GetModuleHandleW(LPCWSTR module)
{
   return 0;
}

HLOCAL WINAPI LocalAlloc(UINT flags, SIZE_T size)
{
   return 0;
}

BOOL WINAPI SystemTimeToFileTime(const SYSTEMTIME *systime, FILETIME *ft)
{
   if (systime == NULL || ft == NULL)
      return FALSE;

   int a = (14 - systime->wMonth) / 12;
   int y = systime->wYear + 4800 - a;
   int m = systime->wMonth + 12 * a - 3;
   int jdn = systime->wDay + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
   int daysSince1601 = jdn - 2305813;

   ULONGLONG intervals = daysSince1601 * 24LL * 60 * 60 * 10000000
      + systime->wHour * 60LL * 60 * 10000000
      + systime->wMinute * 60LL * 10000000
      + systime->wSecond * 10000000LL
      + systime->wMilliseconds * 10000LL;

   ft->dwLowDateTime = (DWORD)(intervals & 0xFFFFFFFF);
   ft->dwHighDateTime = (DWORD)(intervals >> 32);

   return TRUE;
}

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

LSTATUS WINAPI RegCloseKey(HKEY hkey)
{
   return 0;
}

LSTATUS WINAPI RegCreateKeyExW(HKEY hkey, LPCWSTR name, DWORD reserved, LPWSTR class,
                      DWORD options, REGSAM access, SECURITY_ATTRIBUTES *sa,
                      PHKEY retkey, LPDWORD dispos)
{
   return 0;
}

LSTATUS WINAPI RegEnumKeyExW(HKEY hkey, DWORD index, LPWSTR name, LPDWORD name_len,
                     LPDWORD reserved, LPWSTR class, LPDWORD class_len, FILETIME *ft)
{
   return 0;
}

LSTATUS WINAPI RegEnumValueW(HKEY hkey, DWORD index, LPWSTR value, LPDWORD val_count,
                       LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD count)
{
   return 0;
}

LSTATUS WINAPI RegOpenKeyExW(HKEY hkey, LPCWSTR name, DWORD options, REGSAM access, PHKEY retkey)
{
   return 0;
}

LSTATUS WINAPI RegQueryValueExW(HKEY hkey, LPCWSTR name, LPDWORD reserved, LPDWORD type,
                                       LPBYTE data, LPDWORD count)
{
   return 0;
}

LONG WINAPI RegSetKeyValueW(HKEY hkey, LPCWSTR subkey, LPCWSTR name, DWORD type, const void *data, DWORD len)
{
   return 0;
}

LSTATUS WINAPI RegSetValueExW(HKEY hkey, LPCWSTR name, DWORD reserved,
                                     DWORD type, const BYTE *data, DWORD count)
{
   return 0;
}

void *WINAPI RtlAllocateHeap(HANDLE heap, ULONG flags, SIZE_T size)
{
   return NULL;
}

LONG WINAPI RtlCompareUnicodeStrings(const WCHAR *s1, SIZE_T len1, const WCHAR *s2, SIZE_T len2, BOOLEAN case_insensitive)
{
   WCHAR wz1[len1 + 1];
   memset(wz1, 0, sizeof(wz1));
   wcsncpy(wz1, s1, len1);

   WCHAR wz2[len2 + 1];
   memset(wz2, 0, sizeof(wz2));
   wcsncpy(wz2, s2, len2);

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

BOOLEAN WINAPI RtlFreeHeap(HANDLE heap, ULONG flags, void *ptr)
{
   return FALSE;
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

__forceinline unsigned int decode_utf8_char(unsigned char ch, const char **str, const char *strend)
{
   /* number of following bytes in sequence based on first byte value (for bytes above 0x7f) */
   static const char utf8_length[128] =
   {
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x80-0x8f */
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x90-0x9f */
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xa0-0xaf */
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xb0-0xbf */
      0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0xc0-0xcf */
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0xd0-0xdf */
      2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* 0xe0-0xef */
      3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0  /* 0xf0-0xff */
   };

   /* first byte mask depending on UTF-8 sequence length */
   static const unsigned char utf8_mask[4] = { 0x7f, 0x1f, 0x0f, 0x07 };

   unsigned int len = utf8_length[ch - 0x80];
   unsigned int res = ch & utf8_mask[len];
   const char *end = *str + len;

   if (end > strend)
   {
      *str = end;
      return ~0;
   }
   switch (len)
   {
   case 3:
      if ((ch = end[-3] ^ 0x80) >= 0x40) break;
      res = (res << 6) | ch;
      (*str)++;
      if (res < 0x10) break;
   case 2:
      if ((ch = end[-2] ^ 0x80) >= 0x40) break;
      res = (res << 6) | ch;
      if (res >= 0x110000 >> 6) break;
      (*str)++;
      if (res < 0x20) break;
      if (res >= 0xd800 >> 6 && res <= 0xdfff >> 6) break;
   case 1:
      if ((ch = end[-1] ^ 0x80) >= 0x40) break;
      res = (res << 6) | ch;
      (*str)++;
      if (res < 0x80) break;
      return res;
   }
   return ~0;
}

__forceinline NTSTATUS utf8_mbstowcs(WCHAR *dst, unsigned int dstlen, unsigned int *reslen,
                             const char *src, unsigned int srclen)
{
   unsigned int res;
   NTSTATUS status = STATUS_SUCCESS;
   const char *srcend = src + srclen;
   WCHAR *dstend = dst + dstlen;

   while ((dst < dstend) && (src < srcend))
   {
      unsigned char ch = *src++;
      if (ch < 0x80)  /* special fast case for 7-bit ASCII */
      {
         *dst++ = ch;
         continue;
      }
      if ((res = decode_utf8_char(ch, &src, srcend)) <= 0xffff)
      {
         *dst++ = res;
      }
      else if (res <= 0x10ffff)  /* we need surrogates */
      {
         res -= 0x10000;
         *dst++ = 0xd800 | (res >> 10);
         if (dst == dstend) break;
         *dst++ = 0xdc00 | (res & 0x3ff);
      }
      else
      {
         *dst++ = 0xfffd;
         status = STATUS_SOME_NOT_MAPPED;
      }
   }
   if (src < srcend) status = STATUS_BUFFER_TOO_SMALL;  /* overflow */
   *reslen = dstlen - (dstend - dst);
   return status;
}

__forceinline NTSTATUS utf8_mbstowcs_size(const char *src, unsigned int srclen, unsigned int *reslen)
{
   unsigned int res, len;
   NTSTATUS status = STATUS_SUCCESS;
   const char *srcend = src + srclen;

   for (len = 0; src < srcend; len++)
   {
      unsigned char ch = *src++;
      if (ch < 0x80) continue;
      if ((res = decode_utf8_char(ch, &src, srcend)) > 0x10ffff)
         status = STATUS_SOME_NOT_MAPPED;
      else
         if (res > 0xffff) len++;
   }
   *reslen = len;
   return status;
}

__forceinline int get_utf16(const WCHAR *src, unsigned int srclen, unsigned int *ch)
{
   if (IS_HIGH_SURROGATE(src[0]))
   {
      if (srclen <= 1) return 0;
      if (!IS_LOW_SURROGATE(src[1])) return 0;
      *ch = 0x10000 + ((src[0] & 0x3ff) << 10) + (src[1] & 0x3ff);
      return 2;
   }
   if (IS_LOW_SURROGATE(src[0])) return 0;
   *ch = src[0];
   return 1;
}

__forceinline NTSTATUS utf8_wcstombs_size(const WCHAR *src, unsigned int srclen, unsigned int *reslen)
{
   unsigned int val, len;
   NTSTATUS status = STATUS_SUCCESS;

   for (len = 0; srclen; srclen--, src++)
   {
      if (*src < 0x80) len++;  /* 0x00-0x7f: 1 byte */
      else if (*src < 0x800) len += 2;  /* 0x80-0x7ff: 2 bytes */
      else
      {
         if (!get_utf16(src, srclen, &val))
         {
            val = 0xfffd;
            status = STATUS_SOME_NOT_MAPPED;
         }
         if (val < 0x10000) len += 3; /* 0x800-0xffff: 3 bytes */
         else   /* 0x10000-0x10ffff: 4 bytes */
         {
            len += 4;
            src++;
            srclen--;
         }
      }
   }
   *reslen = len;
   return status;
}

__forceinline NTSTATUS utf8_wcstombs(char *dst, unsigned int dstlen, unsigned int *reslen,
                             const WCHAR *src, unsigned int srclen)
{
   char *end;
   unsigned int val;
   NTSTATUS status = STATUS_SUCCESS;

   for (end = dst + dstlen; srclen; srclen--, src++)
   {
      WCHAR ch = *src;

      if (ch < 0x80)  /* 0x00-0x7f: 1 byte */
      {
         if (dst > end - 1) break;
         *dst++ = ch;
         continue;
      }
      if (ch < 0x800)  /* 0x80-0x7ff: 2 bytes */
      {
         if (dst > end - 2) break;
         dst[1] = 0x80 | (ch & 0x3f);
         ch >>= 6;
         dst[0] = 0xc0 | ch;
         dst += 2;
         continue;
      }
      if (!get_utf16(src, srclen, &val))
      {
         val = 0xfffd;
         status = STATUS_SOME_NOT_MAPPED;
      }
      if (val < 0x10000)  /* 0x800-0xffff: 3 bytes */
      {
         if (dst > end - 3) break;
         dst[2] = 0x80 | (val & 0x3f);
         val >>= 6;
         dst[1] = 0x80 | (val & 0x3f);
         val >>= 6;
         dst[0] = 0xe0 | val;
         dst += 3;
      }
      else   /* 0x10000-0x10ffff: 4 bytes */
      {
         if (dst > end - 4) break;
         dst[3] = 0x80 | (val & 0x3f);
         val >>= 6;
         dst[2] = 0x80 | (val & 0x3f);
         val >>= 6;
         dst[1] = 0x80 | (val & 0x3f);
         val >>= 6;
         dst[0] = 0xf0 | val;
         dst += 4;
         src++;
         srclen--;
      }
   }
   if (srclen) status = STATUS_BUFFER_TOO_SMALL;
   *reslen = dstlen - (end - dst);
   return status;
}

NTSTATUS WINAPI RtlUTF8ToUnicodeN(WCHAR *dst, DWORD dstlen, DWORD *reslen, const char *src, DWORD srclen)
{
   unsigned int ret;
   NTSTATUS status;

   if (!dst)
      status = utf8_mbstowcs_size(src, srclen, &ret);
   else
      status = utf8_mbstowcs(dst, dstlen / sizeof(WCHAR), &ret, src, srclen);

   *reslen = ret * sizeof(WCHAR);
   return status;
}

NTSTATUS WINAPI RtlUnicodeToUTF8N(char *dst, DWORD dstlen, DWORD *reslen, const WCHAR *src, DWORD srclen)
{
   unsigned int ret;
   NTSTATUS status;

   if (!dst)
      status = utf8_wcstombs_size(src, srclen / sizeof(WCHAR), &ret);
   else
      status = utf8_wcstombs(dst, dstlen, &ret, src, srclen / sizeof(WCHAR));

   *reslen = ret;
   return status;
}

UINT WINAPI GetPaletteEntries(HPALETTE palette, UINT start, UINT count, PALETTEENTRY *entries)
{
   return 0;
}

COLORREF WINAPI GetSysColor(INT index)
{
   return 0;
}

__forceinline int wcsnicmp(LPCWSTR str1, LPCWSTR str2, size_t n)
{
   int ret = 0;
   for (; n > 0; n--, str1++, str2++)
   {
      WCHAR ch1 = (*str1 >= 'A' && *str1 <= 'Z') ? *str1 + 32 : *str1;
      WCHAR ch2 = (*str2 >= 'A' && *str2 <= 'Z') ? *str2 + 32 : *str2;
      if ((ret = ch1 - ch2) ||  !*str1) break;
   }
   return ret;
}

__forceinline int wcsicmp(LPCWSTR str1, LPCWSTR str2)
{
   for (;;)
   {
      WCHAR ch1 = (*str1 >= 'A' && *str1 <= 'Z') ? *str1 + 32 : *str1;
      WCHAR ch2 = (*str2 >= 'A' && *str2 <= 'Z') ? *str2 + 32 : *str2;
      if (ch1 != ch2 || !*str1) return ch1 - ch2;
      str1++;
      str2++;
   }
}

__forceinline INT WINAPI lstrcmpA(LPCSTR str1, LPCSTR str2)
{
   return strcmp(str1, str2);
}

__forceinline INT WINAPI lstrcmpiA(LPCSTR str1, LPCSTR str2)
{
   return strcasecmp(str1, str2);
}

INT WINAPI GetSystemMetrics(INT index)
{
   return 0;
}

BOOL WINAPI ScreenToClient(HWND hwnd, POINT *pt)
{
   return TRUE;
}

BOOL WINAPI GetCursorPos(POINT *pt)
{
   return TRUE;
}

UINT WINAPI GetCurrentDirectoryA(UINT buflen, LPSTR buf)
{
   getcwd(buf, buflen);
   return 0;
}

BOOL WINAPI SetCurrentDirectoryA(LPCSTR buf)
{
   return chdir(buf) == 0;
}

DWORD WINAPI GetModuleFileNameA(HMODULE module, LPSTR filename, DWORD size)
{
   memset(filename, 0, size);
   return 0;
}

INT WINAPI MessageBoxA(HWND hWnd, LPCSTR text, LPCSTR title, UINT type)
{
   external_log_info("MessageBoxA(): title=%s, text=%s", title, text);

   return 0;
}

HRESULT WINAPI StgOpenStorage(const OLECHAR* pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage **ppstgOpen)
{
   return external_open_storage(pwcsName, pstgPriority, grfMode, snbExclude, reserved, ppstgOpen);
}

VOID WINAPI OutputDebugStringA(LPCSTR text)
{
   external_log_info("OutputDebugStringA(): text=%s", text);
}

__forceinline DWORD WINAPI CharLowerBuffW(WCHAR *str, DWORD len)
{
   if (!str) return 0; /* YES */
   return LCMapStringW(LOCALE_USER_DEFAULT, LCMAP_LOWERCASE, str, len, str, len);
}

__forceinline DWORD WINAPI CharLowerBuffA(char *str, DWORD len)
{
   DWORD lenW;
   WCHAR buffer[32];
   WCHAR *strW = buffer;

   if (!str) return 0; /* YES */

   lenW = MultiByteToWideChar(CP_ACP, 0, str, len, NULL, 0);
   if (lenW > ARRAY_SIZE(buffer)) {
      strW = HeapAlloc(GetProcessHeap(), 0, lenW * sizeof(WCHAR));
      if (!strW) return 0;
   }
   MultiByteToWideChar(CP_ACP, 0, str, len, strW, lenW);
   CharLowerBuffW(strW, lenW);
   len = WideCharToMultiByte(CP_ACP, 0, strW, lenW, str, len, NULL, NULL);
   if (strW != buffer) HeapFree(GetProcessHeap(), 0, strW);
   return len;
}

void WINAPI Sleep(DWORD timeout)
{
}

WINBASEAPI void WINAPI InitializeCriticalSection(CRITICAL_SECTION *lpCrit)
{
}

WINBASEAPI void WINAPI LeaveCriticalSection(CRITICAL_SECTION *lpCrit)
{
}

WINBASEAPI void WINAPI EnterCriticalSection(CRITICAL_SECTION *lpCrit)
{
}

WINBASEAPI void WINAPI DeleteCriticalSection(CRITICAL_SECTION *lpCrit)
{
}

WINBASEAPI BOOL WINAPI TryEnterCriticalSection(CRITICAL_SECTION *lpCrit)
{
   return TRUE;
}

HRESULT WINAPI CreateProxyFromTypeInfo(ITypeInfo *typeinfo, IUnknown *outer, REFIID iid, IRpcProxyBuffer **proxy, void **obj)
{
   return S_OK;
}

HRESULT WINAPI CreateStubFromTypeInfo(ITypeInfo *typeinfo, REFIID iid, IUnknown *server, IRpcStubBuffer **stub)
{
   return S_OK;
}

BOOL WINAPI FindActCtxSectionGuid(DWORD flags, const GUID *ext_guid, ULONG id, const GUID *guid, PACTCTX_SECTION_KEYED_DATA info)
{
   return FALSE;
}

BOOL WINAPI IsWow64Process(HANDLE process, PBOOL wow64)
{
   return FALSE;
}

LSTATUS WINAPI RegOpenKeyExA(HKEY hkey, LPCSTR name, DWORD options, REGSAM access, PHKEY retkey)
{
   return STATUS_SUCCESS;
}

LSTATUS WINAPI RegQueryValueA(HKEY hkey, LPCSTR name, LPSTR data, LPLONG count)
{
   return STATUS_SUCCESS;
}

LSTATUS WINAPI RegQueryValueExA(HKEY hkey, LPCSTR name, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD count)
{
   return STATUS_SUCCESS;
}

BOOL WINAPI CloseHandle(HANDLE handle)
{
   if (handle)
   {
      fclose(handle);
      return TRUE;
   }
   return FALSE;
}

BOOL WINAPI CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)
{
   return 0;
}

BOOL WINAPI CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
   return 0;
}

HANDLE WINAPI CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
   CHAR szFileName[MAX_PATH];
   int len = wcslen(lpFileName);
   WideCharToMultiByte(CP_ACP, 0, lpFileName, len, szFileName, len, NULL, NULL);
   szFileName[len] = '\0';

   for (int i = 0; i < len; ++i) {
      if (szFileName[i] == '\\')
         szFileName[i] = '/';
   }

   FILE* fp = NULL;
   char mode[3] = { 0 };

   if (dwDesiredAccess == GENERIC_READ)
      mode[0] = 'r';
   else if (dwDesiredAccess == GENERIC_WRITE)
      mode[0] = 'w';
   else {
      mode[0] = 'r';
      mode[1] = '+';
   }

   switch(dwCreationDisposition) {
      case CREATE_NEW:
      {
         FILE* file_tmp = fopen(szFileName, "r");
         if (file_tmp)
            fclose(file_tmp);
         else
            fp = fopen(szFileName, "w");
      }
      break;
      case CREATE_ALWAYS:
         fp = fopen(szFileName, "w");
         break;
      case OPEN_EXISTING:
      {
         FILE* file_tmp = fopen(szFileName, "r");
         if (file_tmp) {
            fclose(file_tmp);
            fp = fopen(szFileName, mode);
         }
      }
      break;
      case OPEN_ALWAYS:
         fp = fopen(szFileName, "a+");
         break;
      case TRUNCATE_EXISTING:
      {
         FILE* file_tmp = fopen(szFileName, "r");
         if (file_tmp) {
            fclose(file_tmp);
            fp = fopen(szFileName, "w");
         }
      }
      break;
      default:
         break;
    }

   if (!fp)
      fp = INVALID_HANDLE_VALUE;

   return fp;
}

BOOL WINAPI DeleteFileW(LPCWSTR lpFileName)
{
   return 0;
}

BOOL WINAPI FileTimeToLocalFileTime(const FILETIME *lpFileTime, LPFILETIME lpLocalFileTime)
{
   return 0;
}

BOOL WINAPI FindClose(HANDLE hFindFile)
{
   return 0;
}

HANDLE WINAPI FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData)
{
   return 0;
}

BOOL WINAPI FindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData)
{
   return 0;
}

BOOL WINAPI GetDiskFreeSpaceExW(LPCWSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailableToCaller, PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes)
{
   return 0;
}

UINT WINAPI GetDriveTypeW(LPCWSTR lpRootPathName)
{
   return 0;
}

BOOL WINAPI GetFileAttributesExW(LPCWSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation)
{
   return 0;
}

DWORD WINAPI GetFileAttributesW(LPCWSTR lpFileName)
{
   DWORD status = INVALID_FILE_ATTRIBUTES;
   CHAR szFileName[MAX_PATH];
   int len = wcslen(lpFileName);
   WideCharToMultiByte(CP_ACP, 0, lpFileName, len, szFileName, len, NULL, NULL);
   szFileName[len] = '\0';

   for (int i = 0; i < len; ++i) {
      if (szFileName[i] == '\\')
         szFileName[i] = '/';
   }

   if (szFileName[len-1] == '/')
      szFileName[len-1] = '\0';

   struct stat statbuf;
   if (!stat(szFileName, &statbuf)) {
      status = 0;
      if (S_ISDIR(statbuf.st_mode))
         status |= FILE_ATTRIBUTE_DIRECTORY;
      if (S_ISREG(statbuf.st_mode))
         status |= FILE_ATTRIBUTE_NORMAL;
   }
   return status;
}

BOOL WINAPI GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize)
{
   struct stat buffer;
   if (!fstat(fileno(hFile), &buffer)) {
      lpFileSize->QuadPart = buffer.st_size;
      return TRUE;
   }
   return FALSE;
}

DWORD WINAPI GetFullPathNameW(LPCWSTR lpFileName, DWORD nBufferLength, LPWSTR lpBuffer, LPWSTR *lpFilePart)
{
   int len = wcslen(lpFileName) + 1;
   if (lpBuffer) {
      wcsncpy(lpBuffer, lpFileName, nBufferLength);
      WCHAR* ptrBackslash = wcsrchr(lpBuffer, L'\\');
      WCHAR* ptrSlash = wcsrchr(lpBuffer, L'/');
      WCHAR* ptr = NULL;
      if (ptrBackslash && ptrSlash)
         ptr = (ptrBackslash > ptrSlash) ? ptrBackslash : ptrSlash;
      else if (ptrBackslash)
         ptr = ptrBackslash;
      else if (ptrSlash)
         ptr = ptrSlash;
      if (ptr) {
         ptr++;
         lpFilePart = (LPWSTR*)ptr;
      }
   }
   return len;
}

DWORD WINAPI GetLogicalDrives(void)
{
   return 0;
}

UINT WINAPI GetSystemDirectoryW(LPWSTR lpBuffer, UINT uSize)
{
   return 0;
}

DWORD WINAPI GetTempPathW(DWORD nBufferLength, LPWSTR lpBuffer)
{
   return 0;
}

BOOL WINAPI GetVolumeInformationW(LPCWSTR lpRootPathName, LPWSTR lpVolumeNameBuffer, DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, LPWSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize)
{
   return 0;
}

UINT WINAPI GetWindowsDirectoryW(LPWSTR lpBuffer, UINT uSize)
{
   return 0;
}

BOOL WINAPI IsTextUnicode(const void *lpv, int iSize, LPINT lpiResult)
{
   return 0;
}

BOOL WINAPI MoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
{
   return 0;
}

BOOL WINAPI ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
   char* pBuffer = (char*)lpBuffer;
   *lpNumberOfBytesRead = 0;
   while (*lpNumberOfBytesRead < nNumberOfBytesToRead)
   {
      if(feof(hFile)) break;
      *pBuffer++ = fgetc(hFile);
      (*lpNumberOfBytesRead)++;
   }
   return TRUE;
}

BOOL WINAPI RemoveDirectoryW(LPCWSTR lpPathName)
{
   return 0;
}

BOOL WINAPI SetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes)
{
   return 0;
}

DWORD WINAPI SetFilePointer(HANDLE hFile, LONG lDistanceToMove, LPLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
   return 0;
}

BOOL WINAPI SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod)
{
   return 0;
}

BOOL WINAPI WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
   char* pBuffer = (char*)lpBuffer;
   *lpNumberOfBytesWritten = 0;
   while (*lpNumberOfBytesWritten < nNumberOfBytesToWrite) {
      fputc(*pBuffer++, hFile);
      if(feof(hFile)) break;
      (*lpNumberOfBytesWritten)++;
   }
   return TRUE;
}

BOOL WINAPI VerQueryValueW(LPCVOID pBlock, LPCWSTR lpSubBlock,
                        LPVOID * lplpBuffer, PUINT puLen)
{
   return 0;
}

DWORD WINAPI GetFileVersionInfoSizeW(LPCWSTR filename, LPDWORD handle)
{
   return 0;
}

BOOL WINAPI GetFileVersionInfoW(LPCWSTR filename, DWORD handle, DWORD datasize, LPVOID data)
{
   return 0;
}

DWORD WINAPI GetTickCount()
{
   struct timespec ts;
   unsigned theTick = 0U;
   clock_gettime(CLOCK_REALTIME, &ts);
   theTick  = ts.tv_nsec / 1000000;
   theTick += ts.tv_sec * 1000;
   return theTick;
}

LCID WINAPI GetThreadLocale(void)
{
   return 0;
}

NTSTATUS WINAPI NtQuerySystemTime(PLARGE_INTEGER now)
{
   return 0;
}

LSTATUS WINAPI RegLoadMUIStringW(HKEY p1, LPCWSTR p2, LPWSTR p3, DWORD p4, LPDWORD p5, DWORD p6, LPCWSTR p7)
{
   return 0;
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

HICON WINAPI CopyIcon(HICON p1)
{
   return 0;
}

HRESULT WINAPI CreateItemMoniker(LPCOLESTR lpszDelim, LPCOLESTR  lpszItem, LPMONIKER* ppmk)
{
   return 0;
}

HRESULT WINAPI GetRunningObjectTable(DWORD reserved, LPRUNNINGOBJECTTABLE *pprot)
{
   return 0;
}

HRESULT WINAPI OLEAUTPS_DllRegisterServer()
{
   return 0;
}

HRESULT WINAPI OLEAUTPS_DllUnregisterServer()
{
   return 0;
}

INT WINAPI StringFromGUID2(REFGUID id, LPOLESTR str, INT cmax)
{
   return 0;
}

HRESULT WINAPI CoRegisterClassObject(REFCLSID rclsid,LPUNKNOWN pUnk,DWORD dwClsContext,DWORD flags,LPDWORD lpdwRegister)
{
   return 0;
}

HRESULT WINAPI CoRevokeClassObject(DWORD dwRegister)
{
   return 0;
}

HDC WINAPI CreateDCW(LPCWSTR p1, LPCWSTR p2, LPCWSTR p3, const DEVMODEW* p4)
{
   return 0;
}

LRESULT WINAPI DispatchMessageW(const MSG* p1)
{
   return 0;
}

HDC WINAPI GetDC(HWND p1)
{
   return 0;
}

INT WINAPI GetDeviceCaps(HDC p1, INT p2)
{
   return 0;
}

BOOL WINAPI GetMessageW(LPMSG p1, HWND p2, UINT p3, UINT p4)
{
   return 0;
}

DWORD WINAPI GetModuleFileNameW(HMODULE p1, LPWSTR p2, DWORD p3)
{
   return 0;
}

DWORD WINAPI MsgWaitForMultipleObjects(DWORD p1, const HANDLE* p2, BOOL p3, DWORD p4, DWORD p5)
{
   return 0;
}

LSTATUS WINAPI RegDeleteKeyW(HKEY p1, LPCWSTR p2)
{
   return 0;
}

INT WINAPI ReleaseDC(HWND p1, HDC p2)
{
   return 0;
}

BOOL WINAPI TranslateMessage(const MSG* p1)
{
   return 0;
}

HRESULT WINAPI UnRegisterTypeLib(REFGUID p1, WORD p2, WORD p3, LCID p4, SYSKIND p5)
{
   return 0;
}

ULONG WINAPI CStdStubBuffer_AddRef(IRpcStubBuffer* This)
{
   return 0;
}

HRESULT WINAPI CStdStubBuffer_QueryInterface(IRpcStubBuffer* This, REFIID riid, void **ppvObject)
{
   return 0;
}

HRESULT WINAPI CStdStubBuffer_Connect(IRpcStubBuffer* This, IUnknown *pUnkServer)
{
   return 0;
}

void WINAPI CStdStubBuffer_Disconnect(IRpcStubBuffer* This)
{
}

HRESULT WINAPI CStdStubBuffer_Invoke(IRpcStubBuffer* This, RPCOLEMESSAGE* pRpcMsg, IRpcChannelBuffer* pRpcChannelBuffer)
{
   return 0;
}

IRpcStubBuffer* WINAPI CStdStubBuffer_IsIIDSupported(IRpcStubBuffer* This, REFIID riid)
{
   return NULL;
}

ULONG WINAPI CStdStubBuffer_CountRefs(IRpcStubBuffer* This)
{
   return 0;
}

HRESULT WINAPI CStdStubBuffer_DebugServerQueryInterface(IRpcStubBuffer* This, void **ppv)
{
   return 0;
}

void WINAPI CStdStubBuffer_DebugServerRelease(IRpcStubBuffer* This, void * pv)
{
}

HRESULT WINAPI OLEAUTPS_DllGetClassObject(REFCLSID p1, REFIID p2, LPVOID * p3)
{
   return 0;
}

#include <initguid.h>

DEFINE_GUID(CLSID_StdComponentCategoriesMgr, 0x0002e005, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
DEFINE_GUID(IID_ICatRegister, 0x0002e012, 0x0000, 0x0000, 0xc0,0x00, 0x00,0x00,0x00,0x00,0x00,0x46);
