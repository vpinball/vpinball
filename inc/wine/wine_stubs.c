#define COBJMACROS

#include "ntstatus.h"
#define WIN32_NO_STATUS

#include "oleauto.h"
#include "winternl.h"
#include "winuser.h"
#include "wine/debug.h"

#undef GetCurrentDirectory
#define GetCurrentDirectory GetCurrentDirectoryA

#undef GetModuleFileName
#define GetModuleFileName GetModuleFileNameA

#undef MessageBox
#define MessageBox MessageBoxA

#undef OutputDebugString
#define OutputDebugString OutputDebugStringA

#ifdef __STANDALONE__
#include <unistd.h>
#include <sys/stat.h>
#include "vbscript.h"
#endif

enum class_reg_data_origin
{
    CLASS_REG_ACTCTX,
    CLASS_REG_REGISTRY,
};

struct class_reg_data
{
    enum class_reg_data_origin origin;
    union
    {
        struct
        {
            const WCHAR *module_name;
            DWORD threading_model;
            HANDLE hactctx;
        } actctx;
        HKEY hkey;
    } u;
};

const char * __cdecl __wine_dbg_strdup( const char *str ) 
{
    return strdup(str);
}

int __cdecl __wine_dbg_output( const char *str )
{
    printf("%s", str); 

    return 0;
}

int __cdecl __wine_dbg_header( enum __wine_debug_class cls, struct __wine_debug_channel *channel,
                               const char *function )                               
{
/*#ifdef __WINE_DEBUG__
    if (!strcmp(channel->name, "variant") || 
        !strcmp(channel->name, "vbscript")) {
        printf("[%s][%s]: ", channel->name, function); 
        return 0;
    }
#endif*/
    return -1;
}

unsigned char __cdecl __wine_dbg_get_channel_flags( struct __wine_debug_channel *channel )
{
    return '\0';
}

void winetest_set_location( const char* file, int line )
{
}

void winetest_ok( int condition, const char *msg, ... )
{
    va_list valist;

    va_start(valist, msg);

    if (!condition) {
        vprintf( msg, valist );
    }

    va_end(valist);
}

HRESULT WINAPI DECLSPEC_HOTPATCH CLSIDFromProgID(LPCOLESTR progid, CLSID *clsid)
{
    return 0;
}

HRESULT WINAPI DECLSPEC_HOTPATCH CoGetClassObject(REFCLSID rclsid, DWORD clscontext,
        COSERVERINFO *server_info, REFIID riid, void **obj)
{
    return 0;
}

HRESULT WINAPI CreateBindCtx(DWORD reserved, IBindCtx **bind_context) {
    return 0;
}

BOOL WINAPI DECLSPEC_HOTPATCH DisableThreadLibraryCalls( HMODULE module ) {
    return FALSE;
}

void WINAPI DECLSPEC_HOTPATCH GetLocalTime( SYSTEMTIME *systime )
{
}

BOOL WINAPI IsBadStringPtrA( LPCSTR str, UINT_PTR max ) {
    return FALSE;
}

BOOL WINAPI IsBadStringPtrW( LPCWSTR str, UINT_PTR max ) {
    return FALSE;
}

struct _TEB g_TEB = { 0 };

struct _TEB * WINAPI NtCurrentTeb() {
    return &g_TEB;
}

HRESULT WINAPI LoadRegTypeLib(
	REFGUID rguid,
	WORD wVerMajor,
	WORD wVerMinor,
	LCID lcid,
	ITypeLib **ppTLib)
{
    return 0;
}

HGLOBAL WINAPI DECLSPEC_HOTPATCH LoadResource( HINSTANCE module, HRSRC rsrc )
{
    return 0;
}

LPVOID WINAPI DECLSPEC_HOTPATCH LockResource( HGLOBAL handle )
{
    return NULL;
}

HRSRC WINAPI DECLSPEC_HOTPATCH FindResourceExW( HMODULE module, LPCWSTR type, LPCWSTR name, WORD lang )
{
    return 0;
}

INT WINAPI DECLSPEC_HOTPATCH LoadStringW(HINSTANCE instance, UINT resource_id, LPWSTR buffer, INT buflen)
{
    return 0;
}

HRESULT WINAPI LoadTypeLib(const OLECHAR *szFile, ITypeLib * *pptLib) {
    return 0;
}

INT WINAPI MessageBoxW( HWND hwnd, LPCWSTR text, LPCWSTR title, UINT type )
{
    return 0;
}

HRESULT WINAPI MkParseDisplayName(LPBC pbc, LPCOLESTR szDisplayName,
				LPDWORD pchEaten, LPMONIKER *ppmk) {
    return 0;
}

void WINAPI DECLSPEC_HOTPATCH GetSystemTime( SYSTEMTIME *systime )
{
}

DWORD WINAPI GetVersion(void) {
    return 0;
}

void *WINAPI DECLSPEC_HOTPATCH HeapAlloc( HANDLE heap, ULONG flags, SIZE_T size )
{
    return flags & HEAP_ZERO_MEMORY ? calloc(size, sizeof(char*)) : malloc(size);
}

void *WINAPI HeapReAlloc( HANDLE heap, ULONG flags, void *ptr, SIZE_T size ) 
{
    return realloc(ptr, size);
}

BOOL WINAPI DECLSPEC_HOTPATCH HeapFree( HANDLE heap, ULONG flags, void *ptr )
{
    free(ptr);

    return TRUE;
}

void * WINAPI CoTaskMemAlloc(SIZE_T size)
{
    return malloc(size);
}

LPVOID WINAPI CoTaskMemRealloc(LPVOID ptr, SIZE_T size)
{
    return NULL;
}

void WINAPI CoTaskMemFree(void *ptr)
{
    free(ptr);
}

HINSTANCE hProxyDll;

HRESULT WINAPI DECLSPEC_HOTPATCH CoCreateInstance(REFCLSID rclsid, IUnknown *outer, DWORD cls_context,
        REFIID riid, void **obj)
{
    MULTI_QI multi_qi = { .pIID = riid };
    HRESULT hr;

    //TRACE("%s, %p, %#lx, %s, %p.\n", debugstr_guid(rclsid), outer, cls_context, debugstr_guid(riid), obj);

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

    if (include_unk)
    {
        mqi[0].hr = S_OK;
        mqi[0].pItf = unk;
        index = fetched = 1;
    }

    for (; index < count; index++)
    {
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

static HRESULT com_get_class_object(REFCLSID rclsid, DWORD clscontext,
        COSERVERINFO *server_info, REFIID riid, void **obj)
{
    struct class_reg_data clsreg = { 0 };
    HRESULT hr = E_UNEXPECTED;
    IUnknown *registered_obj;
    struct apartment *apt;

    if (!obj)
        return E_INVALIDARG;

    *obj = NULL;

#ifndef __STANDALONE__
    if (!(apt = apartment_get_current_or_mta()))
    {
        ERR("apartment not initialised\n");
        return CO_E_NOTINITIALIZED;
    }

    if (server_info)
        FIXME("server_info name %s, authinfo %p\n", debugstr_w(server_info->pwszName), server_info->pAuthInfo);

    if (clscontext & CLSCTX_INPROC_SERVER)
    {
        if (IsEqualCLSID(rclsid, &CLSID_InProcFreeMarshaler) ||
                IsEqualCLSID(rclsid, &CLSID_GlobalOptions) ||
                (!(clscontext & CLSCTX_APPCONTAINER) && IsEqualCLSID(rclsid, &CLSID_ManualResetEvent)) ||
                IsEqualCLSID(rclsid, &CLSID_StdGlobalInterfaceTable))
        {
            apartment_release(apt);

            if (IsEqualCLSID(rclsid, &CLSID_GlobalOptions))
                return get_builtin_class_factory(rclsid, riid, obj);
            else
                return Ole32DllGetClassObject(rclsid, riid, obj);
        }
    }

    if (clscontext & CLSCTX_INPROC)
    {
        ACTCTX_SECTION_KEYED_DATA data;

        data.cbSize = sizeof(data);
        /* search activation context first */
        if (FindActCtxSectionGuid(FIND_ACTCTX_SECTION_KEY_RETURN_HACTCTX, NULL,
                ACTIVATION_CONTEXT_SECTION_COM_SERVER_REDIRECTION, rclsid, &data))
        {
            struct comclassredirect_data *comclass = (struct comclassredirect_data *)data.lpData;

            clsreg.u.actctx.module_name = (WCHAR *)((BYTE *)data.lpSectionBase + comclass->name_offset);
            clsreg.u.actctx.hactctx = data.hActCtx;
            clsreg.u.actctx.threading_model = comclass->model;
            clsreg.origin = CLASS_REG_ACTCTX;

            hr = apartment_get_inproc_class_object(apt, &clsreg, &comclass->clsid, riid, clscontext, obj);
            ReleaseActCtx(data.hActCtx);
            apartment_release(apt);
            return hr;
        }
    }

    /*
     * First, try and see if we can't match the class ID with one of the
     * registered classes.
     */
    if (!(clscontext & CLSCTX_APPCONTAINER) && (registered_obj = com_get_registered_class_object(apt, rclsid, clscontext)))
    {
        hr = IUnknown_QueryInterface(registered_obj, riid, obj);
        IUnknown_Release(registered_obj);
        apartment_release(apt);
        return hr;
    }

    /* First try in-process server */
    if (clscontext & CLSCTX_INPROC_SERVER)
    {
        HKEY hkey;

        hr = open_key_for_clsid(rclsid, L"InprocServer32", KEY_READ, &hkey);
        if (FAILED(hr))
        {
            if (hr == REGDB_E_CLASSNOTREG)
                ERR("class %s not registered\n", debugstr_guid(rclsid));
            else if (hr == REGDB_E_KEYMISSING)
            {
                WARN("class %s not registered as in-proc server\n", debugstr_guid(rclsid));
                hr = REGDB_E_CLASSNOTREG;
            }
        }

        if (SUCCEEDED(hr))
        {
            clsreg.u.hkey = hkey;
            clsreg.origin = CLASS_REG_REGISTRY;

            hr = apartment_get_inproc_class_object(apt, &clsreg, rclsid, riid, clscontext, obj);
            RegCloseKey(hkey);
        }

        /* return if we got a class, otherwise fall through to one of the
         * other types */
        if (SUCCEEDED(hr))
        {
            apartment_release(apt);
            return hr;
        }
    }

    /* Next try in-process handler */
    if (clscontext & CLSCTX_INPROC_HANDLER)
    {
        HKEY hkey;

        hr = open_key_for_clsid(rclsid, L"InprocHandler32", KEY_READ, &hkey);
        if (FAILED(hr))
        {
            if (hr == REGDB_E_CLASSNOTREG)
                ERR("class %s not registered\n", debugstr_guid(rclsid));
            else if (hr == REGDB_E_KEYMISSING)
            {
                WARN("class %s not registered in-proc handler\n", debugstr_guid(rclsid));
                hr = REGDB_E_CLASSNOTREG;
            }
        }

        if (SUCCEEDED(hr))
        {
            clsreg.u.hkey = hkey;
            clsreg.origin = CLASS_REG_REGISTRY;

            hr = apartment_get_inproc_class_object(apt, &clsreg, rclsid, riid, clscontext, obj);
            RegCloseKey(hkey);
        }

        /* return if we got a class, otherwise fall through to one of the
         * other types */
        if (SUCCEEDED(hr))
        {
            apartment_release(apt);
            return hr;
        }
    }
    apartment_release(apt);

    /* Next try out of process */
    if (clscontext & CLSCTX_LOCAL_SERVER)
    {
        hr = rpc_get_local_class_object(rclsid, riid, obj);
        if (SUCCEEDED(hr))
            return hr;
    }

    /* Finally try remote: this requires networked DCOM (a lot of work) */
    if (clscontext & CLSCTX_REMOTE_SERVER)
    {
        FIXME ("CLSCTX_REMOTE_SERVER not supported\n");
        hr = REGDB_E_CLASSNOTREG;
    }

    if (FAILED(hr))
        ERR("no class object %s could be created for context %#lx\n", debugstr_guid(rclsid), clscontext);
#endif

    return hr;
}

HRESULT WINAPI DECLSPEC_HOTPATCH CoCreateInstanceEx(REFCLSID rclsid, IUnknown *outer, DWORD cls_context,
        COSERVERINFO *server_info, ULONG count, MULTI_QI *results)
{
    IClassFactory *factory;
    IUnknown *unk = NULL;
    CLSID clsid;
    HRESULT hr;

    //TRACE("%s, %p, %#lx, %p, %lu, %p\n", debugstr_guid(rclsid), outer, cls_context, server_info, count, results);

    if (!count || !results)
        return E_INVALIDARG;

    //if (server_info)
    //    FIXME("Server info is not supported.\n");

    init_multi_qi(count, results, E_NOINTERFACE);

    clsid = *rclsid;

#ifndef __STANDALONE__
    if (!(cls_context & CLSCTX_APPCONTAINER))
        CoGetTreatAsClass(rclsid, &clsid);

    if (FAILED(hr = com_get_class_object(&clsid, cls_context, NULL, &IID_IClassFactory, (void **)&factory)))
        return hr;

    hr = IClassFactory_CreateInstance(factory, outer, results[0].pIID, (void **)&unk);
    IClassFactory_Release(factory);
#else
    hr = VBScriptFactory_CreateInstance(factory, outer, results[0].pIID, (void **)&unk);
#endif
    if (FAILED(hr))
    {
        //if (hr == CLASS_E_NOAGGREGATION && outer)
        //    FIXME("Class %s does not support aggregation\n", debugstr_guid(&clsid));
        //else
        //    FIXME("no instance created for interface %s of class %s, hr %#lx.\n",
        //            debugstr_guid(results[0].pIID), debugstr_guid(&clsid), hr);
        return hr;
    }

    return return_multi_qi(unk, count, results, TRUE);
}

BOOL WINAPI DECLSPEC_HOTPATCH FileTimeToSystemTime( const FILETIME *ft, SYSTEMTIME *systime )
{
    return FALSE;
}

DWORD WINAPI DECLSPEC_HOTPATCH GetEnvironmentVariableW( LPCWSTR name, LPWSTR val, DWORD size ) {
    return 0;
}

HMODULE WINAPI DECLSPEC_HOTPATCH GetModuleHandleW( LPCWSTR module ) {
    return 0;
}

HLOCAL WINAPI DECLSPEC_HOTPATCH LocalAlloc( UINT flags, SIZE_T size ) {
    return 0;
}

BOOL WINAPI DECLSPEC_HOTPATCH SystemTimeToFileTime( const SYSTEMTIME *systime, FILETIME *ft )
{
    return FALSE;
}

NTSTATUS WINAPI NtGetNlsSectionPtr( ULONG type, ULONG id, void *unknown, void **ptr, SIZE_T *size ) {
    return 0;
}

NTSTATUS WINAPI NtQueryDefaultLocale( BOOLEAN user, LCID *lcid ) {
    return 0;
}

NTSTATUS WINAPI NtQueryInstallUILanguage( LANGID *lang ) {
    return 0;
}

LSTATUS WINAPI DECLSPEC_HOTPATCH RegCloseKey( HKEY hkey ) {
    return 0;
}

LSTATUS WINAPI DECLSPEC_HOTPATCH RegCreateKeyExW( HKEY hkey, LPCWSTR name, DWORD reserved, LPWSTR class,
                             DWORD options, REGSAM access, SECURITY_ATTRIBUTES *sa,
                             PHKEY retkey, LPDWORD dispos ) {
    return 0;
}

LSTATUS WINAPI RegEnumKeyExW( HKEY hkey, DWORD index, LPWSTR name, LPDWORD name_len,
                           LPDWORD reserved, LPWSTR class, LPDWORD class_len, FILETIME *ft )
{
    return 0;
}

LSTATUS WINAPI RegEnumValueW( HKEY hkey, DWORD index, LPWSTR value, LPDWORD val_count,
                              LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD count )
{
    return 0;
}

LSTATUS WINAPI DECLSPEC_HOTPATCH RegOpenKeyExW( HKEY hkey, LPCWSTR name, DWORD options, REGSAM access, PHKEY retkey )
{
    return 0;
}
LSTATUS WINAPI DECLSPEC_HOTPATCH RegQueryValueExW( HKEY hkey, LPCWSTR name, LPDWORD reserved, LPDWORD type,
                                                   LPBYTE data, LPDWORD count )
{
    return 0;
}
LONG WINAPI RegSetKeyValueW( HKEY hkey, LPCWSTR subkey, LPCWSTR name, DWORD type, const void *data, DWORD len )
{
      return 0;
}
LSTATUS WINAPI DECLSPEC_HOTPATCH RegSetValueExW( HKEY hkey, LPCWSTR name, DWORD reserved,
                                                 DWORD type, const BYTE *data, DWORD count )
{
    return 0;
}

void *WINAPI DECLSPEC_HOTPATCH RtlAllocateHeap( HANDLE heap, ULONG flags, SIZE_T size )
{
    return NULL;
}

LONG WINAPI RtlCompareUnicodeStrings( const WCHAR *s1, SIZE_T len1, const WCHAR *s2, SIZE_T len2,
                                      BOOLEAN case_insensitive )
{
    return 0;
}

NTSTATUS WINAPI RtlEnterCriticalSection( RTL_CRITICAL_SECTION *crit )
{
    return 0;
}

NTSTATUS WINAPI RtlFindMessage( HMODULE hmod, ULONG type, ULONG lang,
                                ULONG msg_id, const MESSAGE_RESOURCE_ENTRY **ret ) {
    return 0;
}

NTSYSAPI NTSTATUS  WINAPI RtlFormatMessage(LPCWSTR,ULONG,BOOLEAN,BOOLEAN,BOOLEAN,__ms_va_list *,LPWSTR,ULONG,ULONG*);
NTSYSAPI NTSTATUS  WINAPI RtlFormatMessageEx(LPCWSTR,ULONG,BOOLEAN,BOOLEAN,BOOLEAN,__ms_va_list *,LPWSTR,ULONG,ULONG*,ULONG);

NTSTATUS WINAPI RtlFormatMessage( const WCHAR *src, ULONG width, BOOLEAN ignore_inserts,
                                  BOOLEAN ansi, BOOLEAN is_array, va_list *args,
                                  WCHAR *buffer, ULONG size, ULONG *retsize ) {
    return 0;
}

NTSTATUS WINAPI RtlFormatMessageEx( const WCHAR *src, ULONG width, BOOLEAN ignore_inserts,
                                    BOOLEAN ansi, BOOLEAN is_array, va_list *args,
                                    WCHAR *buffer, ULONG size, ULONG *retsize, ULONG flags )
{
    return 0;
}

NTSTATUS WINAPI RtlGUIDFromString(PUNICODE_STRING str, GUID* guid) {
    return 0;
}

NTSTATUS WINAPI RtlGetLocaleFileMappingAddress( void **ptr, LCID *lcid, LARGE_INTEGER *size ) {
    return 0;
}

BOOLEAN WINAPI DECLSPEC_HOTPATCH RtlFreeHeap( HANDLE heap, ULONG flags, void *ptr )
{
    return FALSE;
}

NTSTATUS WINAPI RtlGetSystemPreferredUILanguages( DWORD flags, ULONG unknown, ULONG *count,
                                                  WCHAR *buffer, ULONG *size )
{
    return 0;
}

NTSTATUS WINAPI RtlGetProcessPreferredUILanguages( DWORD flags, ULONG *count, WCHAR *buffer, ULONG *size )
{
    return 0;
}

NTSTATUS WINAPI RtlGetThreadPreferredUILanguages( DWORD flags, ULONG *count, WCHAR *buffer, ULONG *size )
{
    return 0;
}

NTSTATUS WINAPI RtlGetUserPreferredUILanguages( DWORD flags, ULONG unknown, ULONG *count,
                                                WCHAR *buffer, ULONG *size )
{
    return 0;
}

NTSTATUS WINAPI RtlIdnToAscii( DWORD flags, const WCHAR *src, INT srclen, WCHAR *dst, INT *dstlen )
{
    return 0;
}

NTSTATUS WINAPI RtlIdnToNameprepUnicode( DWORD flags, const WCHAR *src, INT srclen,
                                         WCHAR *dst, INT *dstlen )
{
    return 0;
}

NTSTATUS WINAPI RtlIdnToUnicode( DWORD flags, const WCHAR *src, INT srclen, WCHAR *dst, INT *dstlen )
{
    return 0;
}

void WINAPI RtlInitCodePageTable( USHORT *ptr, CPTABLEINFO *info )
{
}

void WINAPI RtlInitUnicodeString( UNICODE_STRING *str, const WCHAR *data )
{
}

NTSTATUS WINAPI RtlIsNormalizedString( ULONG form, const WCHAR *str, INT len, BOOLEAN *res )
{
    return 0;
}

NTSTATUS WINAPI RtlLeaveCriticalSection( RTL_CRITICAL_SECTION *crit )
{
    return 0;
}

NTSTATUS WINAPI RtlMultiByteToUnicodeN( WCHAR *dst, DWORD dstlen, DWORD *reslen,
                                        const char *src, DWORD srclen )
{
    return 0;
}

NTSTATUS WINAPI RtlNormalizeString( ULONG form, const WCHAR *src, INT src_len, WCHAR *dst, INT *dst_len )
{
    return 0;
}

ULONG WINAPI RtlNtStatusToDosError( NTSTATUS status )
{
    return 0;
}

NTSTATUS WINAPI RtlQueryTimeZoneInformation(RTL_TIME_ZONE_INFORMATION *ret) {
    return 0;
}

NTSTATUS WINAPI RtlSetProcessPreferredUILanguages( DWORD flags, PCZZWSTR buffer, ULONG *count )
{
    return 0;
}

NTSTATUS WINAPI RtlSetThreadPreferredUILanguages( DWORD flags, PCZZWSTR buffer, ULONG *count )
{
    return 0;
}

NTSTATUS WINAPI RtlSetTimeZoneInformation( const RTL_TIME_ZONE_INFORMATION *tzinfo )
{
    return 0;
}

VOID WINAPI RtlTimeToTimeFields(
	const LARGE_INTEGER *liTime,
	PTIME_FIELDS TimeFields)
{
}

static inline unsigned int decode_utf8_char( unsigned char ch, const char **str, const char *strend )
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

static inline NTSTATUS utf8_mbstowcs( WCHAR *dst, unsigned int dstlen, unsigned int *reslen,
                                      const char *src, unsigned int srclen )
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
        if ((res = decode_utf8_char( ch, &src, srcend )) <= 0xffff)
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

static inline NTSTATUS utf8_mbstowcs_size( const char *src, unsigned int srclen, unsigned int *reslen )
{
    unsigned int res, len;
    NTSTATUS status = STATUS_SUCCESS;
    const char *srcend = src + srclen;

    for (len = 0; src < srcend; len++)
    {
        unsigned char ch = *src++;
        if (ch < 0x80) continue;
        if ((res = decode_utf8_char( ch, &src, srcend )) > 0x10ffff)
            status = STATUS_SOME_NOT_MAPPED;
        else
            if (res > 0xffff) len++;
    }
    *reslen = len;
    return status;
}

static inline int get_utf16( const WCHAR *src, unsigned int srclen, unsigned int *ch )
{
    if (IS_HIGH_SURROGATE( src[0] ))
    {
        if (srclen <= 1) return 0;
        if (!IS_LOW_SURROGATE( src[1] )) return 0;
        *ch = 0x10000 + ((src[0] & 0x3ff) << 10) + (src[1] & 0x3ff);
        return 2;
    }
    if (IS_LOW_SURROGATE( src[0] )) return 0;
    *ch = src[0];
    return 1;
}

static inline NTSTATUS utf8_wcstombs_size( const WCHAR *src, unsigned int srclen, unsigned int *reslen )
{
    unsigned int val, len;
    NTSTATUS status = STATUS_SUCCESS;

    for (len = 0; srclen; srclen--, src++)
    {
        if (*src < 0x80) len++;  /* 0x00-0x7f: 1 byte */
        else if (*src < 0x800) len += 2;  /* 0x80-0x7ff: 2 bytes */
        else
        {
            if (!get_utf16( src, srclen, &val ))
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

static inline NTSTATUS utf8_wcstombs( char *dst, unsigned int dstlen, unsigned int *reslen,
                                      const WCHAR *src, unsigned int srclen )
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
        if (!get_utf16( src, srclen, &val ))
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

NTSTATUS WINAPI RtlUTF8ToUnicodeN( WCHAR *dst, DWORD dstlen, DWORD *reslen, const char *src, DWORD srclen )
{
    unsigned int ret;
    NTSTATUS status;

    if (!dst)
        status = utf8_mbstowcs_size( src, srclen, &ret );
    else
        status = utf8_mbstowcs( dst, dstlen / sizeof(WCHAR), &ret, src, srclen );

    *reslen = ret * sizeof(WCHAR);
    return status;
}

NTSTATUS WINAPI RtlUnicodeToUTF8N( char *dst, DWORD dstlen, DWORD *reslen, const WCHAR *src, DWORD srclen )
{
    unsigned int ret;
    NTSTATUS status;

    if (!dst)
        status = utf8_wcstombs_size( src, srclen / sizeof(WCHAR), &ret );
    else
        status = utf8_wcstombs( dst, dstlen, &ret, src, srclen / sizeof(WCHAR) );

    *reslen = ret;
    return status;
}

UINT WINAPI GetPaletteEntries( HPALETTE palette, UINT start, UINT count, PALETTEENTRY *entries )
{
    return 0;
}

COLORREF WINAPI DECLSPEC_HOTPATCH GetSysColor( INT index )
{
    return 0;
}

int wcsnicmp( LPCWSTR str1, LPCWSTR str2, size_t n )
{
    int ret = 0;
    for ( ; n > 0; n--, str1++, str2++)
    {
        WCHAR ch1 = (*str1 >= 'A' && *str1 <= 'Z') ? *str1 + 32 : *str1;
        WCHAR ch2 = (*str2 >= 'A' && *str2 <= 'Z') ? *str2 + 32 : *str2;
        if ((ret = ch1 - ch2) ||  !*str1) break;
    }
    return ret;
}

int wcsicmp( LPCWSTR str1, LPCWSTR str2 ) {
    for (;;)
    {
        WCHAR ch1 = (*str1 >= 'A' && *str1 <= 'Z') ? *str1 + 32 : *str1;
        WCHAR ch2 = (*str2 >= 'A' && *str2 <= 'Z') ? *str2 + 32 : *str2;
        if (ch1 != ch2 || !*str1) return ch1 - ch2;
        str1++;
        str2++;
    }
}

INT WINAPI GetSystemMetrics( INT index ) {
  return 0;
}

BOOL WINAPI ScreenToClient( HWND hwnd, POINT *pt ) {
  return TRUE;
}

BOOL WINAPI DECLSPEC_HOTPATCH GetCursorPos( POINT *pt ) {
    return TRUE;
}

UINT WINAPI DECLSPEC_HOTPATCH GetCurrentDirectoryA( UINT buflen, LPSTR buf ) {
   getcwd(buf, buflen);
   return 0;
}

DWORD WINAPI DECLSPEC_HOTPATCH GetModuleFileNameA( HMODULE module, LPSTR filename, DWORD size ) {
   memset(filename, 0, size);
   return 0;
}

INT WINAPI MessageBoxA(HWND hWnd, LPCSTR text, LPCSTR title, UINT type) {
   printf("%s : %s\n", title, text);
   return 0;
}

VOID WINAPI OutputDebugStringA(LPCSTR text) {
   printf("%s", text);
}

DWORD WINAPI CharLowerBuffW(WCHAR *str, DWORD len)
{
    if (!str) return 0; /* YES */
    return LCMapStringW(LOCALE_USER_DEFAULT, LCMAP_LOWERCASE, str, len, str, len);
}

DWORD WINAPI CharLowerBuffA(char *str, DWORD len)
{
    DWORD lenW;
    WCHAR buffer[32];
    WCHAR *strW = buffer;

    if (!str) return 0; /* YES */

    lenW = MultiByteToWideChar(CP_ACP, 0, str, len, NULL, 0);
    if (lenW > ARRAY_SIZE(buffer))
    {
        strW = HeapAlloc(GetProcessHeap(), 0, lenW * sizeof(WCHAR));
        if (!strW) return 0;
    }
    MultiByteToWideChar(CP_ACP, 0, str, len, strW, lenW);
    CharLowerBuffW(strW, lenW);
    len = WideCharToMultiByte(CP_ACP, 0, strW, lenW, str, len, NULL, NULL);
    if (strW != buffer) HeapFree(GetProcessHeap(), 0, strW);
    return len;
}

void WINAPI DECLSPEC_HOTPATCH Sleep( DWORD timeout )
{
}

WINBASEAPI void WINAPI InitializeCriticalSection(CRITICAL_SECTION *lpCrit) {
}

WINBASEAPI void WINAPI LeaveCriticalSection(CRITICAL_SECTION *lpCrit) {
}

WINBASEAPI void WINAPI EnterCriticalSection(CRITICAL_SECTION *lpCrit) {
}

WINBASEAPI void WINAPI DeleteCriticalSection(CRITICAL_SECTION *lpCrit) {
}

WINBASEAPI BOOL WINAPI TryEnterCriticalSection(CRITICAL_SECTION *lpCrit) {
    return TRUE;
}

HRESULT WINAPI CreateProxyFromTypeInfo(ITypeInfo *typeinfo, IUnknown *outer, REFIID iid, IRpcProxyBuffer **proxy, void **obj) {
    return S_OK;
}

HRESULT WINAPI CreateStubFromTypeInfo(ITypeInfo *typeinfo, REFIID iid, IUnknown *server, IRpcStubBuffer **stub) {
    return S_OK;
}

BOOL WINAPI DECLSPEC_HOTPATCH FindActCtxSectionGuid( DWORD flags, const GUID *ext_guid, ULONG id, const GUID *guid, PACTCTX_SECTION_KEYED_DATA info ) {
    return FALSE;
}

BOOL WINAPI DECLSPEC_HOTPATCH IsWow64Process( HANDLE process, PBOOL wow64 ) {
    return FALSE;
}

LSTATUS WINAPI DECLSPEC_HOTPATCH RegOpenKeyExA( HKEY hkey, LPCSTR name, DWORD options, REGSAM access, PHKEY retkey ) {
    return STATUS_SUCCESS;
}

LSTATUS WINAPI RegQueryValueA( HKEY hkey, LPCSTR name, LPSTR data, LPLONG count ) {
    return STATUS_SUCCESS;
}

LSTATUS WINAPI DECLSPEC_HOTPATCH RegQueryValueExA( HKEY hkey, LPCSTR name, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD count ) {
    return STATUS_SUCCESS;
}

BOOL WINAPI CloseHandle(HANDLE handle) {
    return 0;
}

BOOL WINAPI CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists) {
    return 0;
}

BOOL WINAPI CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
    return 0;
}

HANDLE WINAPI CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    return 0;
}

BOOL WINAPI DeleteFileW(LPCWSTR lpFileName) {
    return 0;
}

BOOL WINAPI FileTimeToLocalFileTime(const FILETIME *lpFileTime, LPFILETIME lpLocalFileTime) {
    return 0;
}

BOOL WINAPI FindClose(HANDLE hFindFile) {
    return 0;
}

HANDLE WINAPI FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData) {
    return 0;
}

BOOL WINAPI FindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData) {
    return 0;
}

BOOL WINAPI GetDiskFreeSpaceExW(LPCWSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailableToCaller, PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes) {
    return 0;
}

UINT WINAPI GetDriveTypeW(LPCWSTR lpRootPathName) {
    return 0;
}

BOOL WINAPI GetFileAttributesExW(LPCWSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation) {
    return 0;
}

DWORD WINAPI GetFileAttributesW(LPCWSTR lpFileName) {
    DWORD status = INVALID_FILE_ATTRIBUTES;

    CHAR szFileName[MAX_PATH];
    int len = wcslen(lpFileName);
    WideCharToMultiByte(CP_ACP, 0, lpFileName, len, szFileName, len, NULL, NULL);
    szFileName[len] = '\0';

    struct stat statbuf;
    if (!stat(szFileName, &statbuf)) {
        status = 0;
        if (S_ISDIR(statbuf.st_mode)) {
            status |= FILE_ATTRIBUTE_DIRECTORY;
        }
        if (S_ISREG(statbuf.st_mode)) {
            status |= FILE_ATTRIBUTE_NORMAL;
        }
    }

    return status;
}

BOOL WINAPI GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize) {
    return 0;
}

DWORD WINAPI GetFullPathNameW(LPCWSTR lpFileName, DWORD nBufferLength, LPWSTR lpBuffer, LPWSTR *lpFilePart) {
    return 0;
}

DWORD WINAPI GetLogicalDrives(void) {
    return 0;
}

UINT WINAPI GetSystemDirectoryW(LPWSTR lpBuffer, UINT uSize) {
    return 0;
}

DWORD WINAPI GetTempPathW(DWORD nBufferLength, LPWSTR lpBuffer) {
    return 0;
}

BOOL WINAPI GetVolumeInformationW(LPCWSTR lpRootPathName, LPWSTR lpVolumeNameBuffer, DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, LPWSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize) {
    return 0;
}

UINT WINAPI GetWindowsDirectoryW(LPWSTR lpBuffer, UINT uSize) {
    return 0;
}

BOOL WINAPI IsTextUnicode(const VOID *lpv, int iSize, LPINT lpiResult) {
    return 0;
}

BOOL WINAPI MoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName) {
    return 0;
}

BOOL WINAPI ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
    return 0;
}

BOOL WINAPI RemoveDirectoryW(LPCWSTR lpPathName) {
    return 0;
}

BOOL WINAPI SetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes) {
    return 0;
}

DWORD WINAPI SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) {
    return 0;
}

BOOL WINAPI SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod) {
    return 0;
}

BOOL WINAPI WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
    return 0;
}

BOOL WINAPI VerQueryValueW( LPCVOID pBlock, LPCWSTR lpSubBlock,
                               LPVOID *lplpBuffer, PUINT puLen ) {
    return 0;
}

DWORD WINAPI GetFileVersionInfoSizeW( LPCWSTR filename, LPDWORD handle ) {
    return 0;
}

BOOL WINAPI GetFileVersionInfoW( LPCWSTR filename, DWORD handle, DWORD datasize, LPVOID data ) {
    return 0;
}