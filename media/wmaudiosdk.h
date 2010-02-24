/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.01.75 */
/* at Mon Aug 09 13:10:31 1999
 */
/* Compiler settings for .\wmaudiosdk.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __wmaudiosdk_h__
#define __wmaudiosdk_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IWMAudioWriter_FWD_DEFINED__
#define __IWMAudioWriter_FWD_DEFINED__
typedef interface IWMAudioWriter IWMAudioWriter;
#endif 	/* __IWMAudioWriter_FWD_DEFINED__ */


#ifndef __IWMAudioReader_FWD_DEFINED__
#define __IWMAudioReader_FWD_DEFINED__
typedef interface IWMAudioReader IWMAudioReader;
#endif 	/* __IWMAudioReader_FWD_DEFINED__ */


#ifndef __IWMAudioReadCallback_FWD_DEFINED__
#define __IWMAudioReadCallback_FWD_DEFINED__
typedef interface IWMAudioReadCallback IWMAudioReadCallback;
#endif 	/* __IWMAudioReadCallback_FWD_DEFINED__ */


#ifndef __IWMAudioInfo_FWD_DEFINED__
#define __IWMAudioInfo_FWD_DEFINED__
typedef interface IWMAudioInfo IWMAudioInfo;
#endif 	/* __IWMAudioInfo_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL_itf_wmaudiosdk_0000
 * at Mon Aug 09 13:10:31 1999
 * using MIDL 3.01.75
 ****************************************/
/* [local] */ 


//=========================================================================
//
//  THIS SOFTWARE HAS BEEN LICENSED FROM MICROSOFT CORPORATION PURSUANT 
//  TO THE TERMS OF AN END USER LICENSE AGREEMENT ("EULA").  
//  PLEASE REFER TO THE TEXT OF THE EULA TO DETERMINE THE RIGHTS TO USE THE SOFTWARE.  
//
// Copyright (C) Microsoft Corporation, 1996 - 1999  All Rights Reserved.
//
//=========================================================================
typedef struct tWAVEFORMATEX WAVEFORMATEX;




#include "nserror.h" 
#include "asferr.h" 
EXTERN_GUID( IID_IWMAudioWriter, 0x1A5636F1, 0xDB5E, 0x11d2, 0x9D, 0x41, 0x00, 0x60, 0x08, 0x31, 0x78, 0xAF );
EXTERN_GUID( IID_IWMAudioReader, 0x1A5636F2, 0xDB5E, 0x11d2, 0x9D, 0x41, 0x00, 0x60, 0x08, 0x31, 0x78, 0xAF );
EXTERN_GUID( IID_IWMAudioReadCallback, 0x1A5636F3, 0xDB5E, 0x11d2, 0x9D, 0x41, 0x00, 0x60, 0x08, 0x31, 0x78, 0xAF );
EXTERN_GUID( IID_IWMAudioInfo, 0xaa139f0, 0xf6a8, 0x11d2, 0x97, 0xf7, 0x0, 0xa0, 0xc9, 0x5e, 0xa8, 0x50 );
#define WMT_SAMPLE_MUSIC  0
#define WMT_SAMPLE_SPEECH 0xFFFFFFFF
////////////////////////////////////////////////////////////////
//
// These are the special case attributes that give information 
// about the ASF file.
//
static const DWORD g_dwWMASpecialAttributes = 7;
static const WCHAR *g_wszWMADuration = L"Duration";
static const WCHAR *g_wszWMABitrate = L"Bitrate";
static const WCHAR *g_wszWMASeekable = L"Seekable";
static const WCHAR *g_wszWMABroadcast = L"Broadcast";
static const WCHAR *g_wszWMAProtected = L"Is_Protected";
static const WCHAR *g_wszWMATrusted = L"Is_Trusted";
static const WCHAR *g_wszWMASignature_Name = L"Signature_Name";

////////////////////////////////////////////////////////////////
//
// The content description object supports 5 basic attributes.
//
static const DWORD g_dwWMAContentAttributes = 5;
static const WCHAR *g_wszWMATitle = L"Title";
static const WCHAR *g_wszWMAAuthor = L"Author";
static const WCHAR *g_wszWMADescription = L"Description";
static const WCHAR *g_wszWMARating = L"Rating";
static const WCHAR *g_wszWMACopyright = L"Copyright";

////////////////////////////////////////////////////////////////
//
// These attributes are used to set DRM properties on an ASF.
//
static const WCHAR *g_wszWMAUse_DRM = L"Use_DRM";
static const WCHAR *g_wszWMADRM_Flags = L"DRM_Flags";
static const WCHAR *g_wszWMADRM_Level = L"DRM_Level";

////////////////////////////////////////////////////////////////
//
// These are the additional attributes defined in the ASF attribute
// namespace that gives information about the content in the ASF file.
//
static const WCHAR *g_wszWMAAlbumTitle = L"WM/AlbumTitle";
static const WCHAR *g_wszWMATrack = L"WM/Track";
static const WCHAR *g_wszWMAPromotionURL = L"WM/PromotionURL";
static const WCHAR *g_wszWMAAlbumCoverURL = L"WM/AlbumCoverURL";
static const WCHAR *g_wszWMAGenre = L"WM/Genre";
static const WCHAR *g_wszWMAYear = L"WM/Year";

HRESULT STDMETHODCALLTYPE WMAudioCreateWriter( LPCWSTR pszFilename, IWMAudioWriter **ppIWMAudioWriter ); 
HRESULT STDMETHODCALLTYPE WMAudioCreateReader( LPCWSTR pszFilename, IWMAudioReadCallback *pIWMReadCallback, IWMAudioReader **ppIWMAudioReader, void *pvReserved ); 
HRESULT STDMETHODCALLTYPE WMAudioCreateInfo( LPCWSTR pszFilename, IWMAudioInfo **ppIWMAudioInfo );
typedef 
enum WMT_STATUS
    {	WMT_ERROR	= 0,
	WMT_BUFFERING_START	= 1,
	WMT_BUFFERING_STOP	= 2,
	WMT_EOF	= 3,
	WMT_LOCATING	= 4,
	WMT_CONNECTING	= 5,
	WMT_NO_RIGHTS	= 6,
	WMT_MISSING_CODEC	= 7
    }	WMT_STATUS;

typedef 
enum WMT_ATTR_DATATYPE
    {	WMT_TYPE_DWORD	= 0,
	WMT_TYPE_STRING	= 1,
	WMT_TYPE_BINARY	= 2,
	WMT_TYPE_BOOL	= 3
    }	WMT_ATTR_DATATYPE;

typedef 
enum WMT_RIGHTS
    {	WMT_RIGHT_PLAYBACK	= 0x1,
	WMT_RIGHT_COPY_TO_PORTABLE	= 0x2,
	WMT_RIGHT_COPY_TO_CD	= 0x8
    }	WMT_RIGHTS;

typedef 
enum WMT_AUDIO_OPTIONS
    {	WMT_OPTION_DEFAULT	= 0
    }	WMT_AUDIO_OPTIONS;



extern RPC_IF_HANDLE __MIDL_itf_wmaudiosdk_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmaudiosdk_0000_v0_0_s_ifspec;

#ifndef __IWMAudioWriter_INTERFACE_DEFINED__
#define __IWMAudioWriter_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IWMAudioWriter
 * at Mon Aug 09 13:10:31 1999
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IWMAudioWriter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("1A5636F1-DB5E-11d2-9D41-0060083178AF")
    IWMAudioWriter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetAttribute( 
            /* [in] */ LPCWSTR pszName,
            /* [in] */ WMT_ATTR_DATATYPE Type,
            /* [in] */ const BYTE __RPC_FAR *pValue,
            /* [in] */ WORD cbLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInputFormat( 
            /* [in] */ const WAVEFORMATEX __RPC_FAR *pWfx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOutputFormat( 
            /* [in] */ DWORD dwBitrate,
            /* [in] */ DWORD dwSampleRate,
            /* [in] */ DWORD dwNumChannels,
            /* [in] */ DWORD dwAudioOptions) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputFormat( 
            /* [out] */ DWORD __RPC_FAR *pdwBitrate,
            /* [out] */ DWORD __RPC_FAR *pdwSampleRate,
            /* [out] */ DWORD __RPC_FAR *pdwNumChannels,
            /* [out] */ DWORD __RPC_FAR *pdwAudioOptions) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WriteSample( 
            /* [in] */ const BYTE __RPC_FAR *pData,
            /* [in] */ DWORD cbData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Flush( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWMAudioWriterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMAudioWriter __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMAudioWriter __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMAudioWriter __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAttribute )( 
            IWMAudioWriter __RPC_FAR * This,
            /* [in] */ LPCWSTR pszName,
            /* [in] */ WMT_ATTR_DATATYPE Type,
            /* [in] */ const BYTE __RPC_FAR *pValue,
            /* [in] */ WORD cbLength);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetInputFormat )( 
            IWMAudioWriter __RPC_FAR * This,
            /* [in] */ const WAVEFORMATEX __RPC_FAR *pWfx);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetOutputFormat )( 
            IWMAudioWriter __RPC_FAR * This,
            /* [in] */ DWORD dwBitrate,
            /* [in] */ DWORD dwSampleRate,
            /* [in] */ DWORD dwNumChannels,
            /* [in] */ DWORD dwAudioOptions);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOutputFormat )( 
            IWMAudioWriter __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwBitrate,
            /* [out] */ DWORD __RPC_FAR *pdwSampleRate,
            /* [out] */ DWORD __RPC_FAR *pdwNumChannels,
            /* [out] */ DWORD __RPC_FAR *pdwAudioOptions);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WriteSample )( 
            IWMAudioWriter __RPC_FAR * This,
            /* [in] */ const BYTE __RPC_FAR *pData,
            /* [in] */ DWORD cbData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Flush )( 
            IWMAudioWriter __RPC_FAR * This);
        
        END_INTERFACE
    } IWMAudioWriterVtbl;

    interface IWMAudioWriter
    {
        CONST_VTBL struct IWMAudioWriterVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMAudioWriter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMAudioWriter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMAudioWriter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMAudioWriter_SetAttribute(This,pszName,Type,pValue,cbLength)	\
    (This)->lpVtbl -> SetAttribute(This,pszName,Type,pValue,cbLength)

#define IWMAudioWriter_SetInputFormat(This,pWfx)	\
    (This)->lpVtbl -> SetInputFormat(This,pWfx)

#define IWMAudioWriter_SetOutputFormat(This,dwBitrate,dwSampleRate,dwNumChannels,dwAudioOptions)	\
    (This)->lpVtbl -> SetOutputFormat(This,dwBitrate,dwSampleRate,dwNumChannels,dwAudioOptions)

#define IWMAudioWriter_GetOutputFormat(This,pdwBitrate,pdwSampleRate,pdwNumChannels,pdwAudioOptions)	\
    (This)->lpVtbl -> GetOutputFormat(This,pdwBitrate,pdwSampleRate,pdwNumChannels,pdwAudioOptions)

#define IWMAudioWriter_WriteSample(This,pData,cbData)	\
    (This)->lpVtbl -> WriteSample(This,pData,cbData)

#define IWMAudioWriter_Flush(This)	\
    (This)->lpVtbl -> Flush(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWMAudioWriter_SetAttribute_Proxy( 
    IWMAudioWriter __RPC_FAR * This,
    /* [in] */ LPCWSTR pszName,
    /* [in] */ WMT_ATTR_DATATYPE Type,
    /* [in] */ const BYTE __RPC_FAR *pValue,
    /* [in] */ WORD cbLength);


void __RPC_STUB IWMAudioWriter_SetAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioWriter_SetInputFormat_Proxy( 
    IWMAudioWriter __RPC_FAR * This,
    /* [in] */ const WAVEFORMATEX __RPC_FAR *pWfx);


void __RPC_STUB IWMAudioWriter_SetInputFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioWriter_SetOutputFormat_Proxy( 
    IWMAudioWriter __RPC_FAR * This,
    /* [in] */ DWORD dwBitrate,
    /* [in] */ DWORD dwSampleRate,
    /* [in] */ DWORD dwNumChannels,
    /* [in] */ DWORD dwAudioOptions);


void __RPC_STUB IWMAudioWriter_SetOutputFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioWriter_GetOutputFormat_Proxy( 
    IWMAudioWriter __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwBitrate,
    /* [out] */ DWORD __RPC_FAR *pdwSampleRate,
    /* [out] */ DWORD __RPC_FAR *pdwNumChannels,
    /* [out] */ DWORD __RPC_FAR *pdwAudioOptions);


void __RPC_STUB IWMAudioWriter_GetOutputFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioWriter_WriteSample_Proxy( 
    IWMAudioWriter __RPC_FAR * This,
    /* [in] */ const BYTE __RPC_FAR *pData,
    /* [in] */ DWORD cbData);


void __RPC_STUB IWMAudioWriter_WriteSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioWriter_Flush_Proxy( 
    IWMAudioWriter __RPC_FAR * This);


void __RPC_STUB IWMAudioWriter_Flush_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWMAudioWriter_INTERFACE_DEFINED__ */


#ifndef __IWMAudioReader_INTERFACE_DEFINED__
#define __IWMAudioReader_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IWMAudioReader
 * at Mon Aug 09 13:10:31 1999
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IWMAudioReader;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("1A5636F2-DB5E-11d2-9D41-0060083178AF")
    IWMAudioReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAttributeByName( 
            /* [in] */ LPCWSTR pszName,
            /* [out] */ WMT_ATTR_DATATYPE __RPC_FAR *pType,
            /* [out] */ BYTE __RPC_FAR *pValue,
            /* [out][in] */ WORD __RPC_FAR *pcbLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributeCount( 
            /* [out] */ WORD __RPC_FAR *pcAttributes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributeByIndex( 
            /* [in] */ WORD wIndex,
            /* [out] */ WCHAR __RPC_FAR *pwszName,
            /* [out][in] */ WORD __RPC_FAR *pcbNameLen,
            /* [out] */ WMT_ATTR_DATATYPE __RPC_FAR *pType,
            /* [out] */ BYTE __RPC_FAR *pValue,
            /* [out][in] */ WORD __RPC_FAR *pcbLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputFormat( 
            /* [out] */ WAVEFORMATEX __RPC_FAR *pWfx,
            /* [in] */ DWORD cbSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Seek( 
            DWORD dwMsTime) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWMAudioReaderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMAudioReader __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMAudioReader __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMAudioReader __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAttributeByName )( 
            IWMAudioReader __RPC_FAR * This,
            /* [in] */ LPCWSTR pszName,
            /* [out] */ WMT_ATTR_DATATYPE __RPC_FAR *pType,
            /* [out] */ BYTE __RPC_FAR *pValue,
            /* [out][in] */ WORD __RPC_FAR *pcbLength);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAttributeCount )( 
            IWMAudioReader __RPC_FAR * This,
            /* [out] */ WORD __RPC_FAR *pcAttributes);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAttributeByIndex )( 
            IWMAudioReader __RPC_FAR * This,
            /* [in] */ WORD wIndex,
            /* [out] */ WCHAR __RPC_FAR *pwszName,
            /* [out][in] */ WORD __RPC_FAR *pcbNameLen,
            /* [out] */ WMT_ATTR_DATATYPE __RPC_FAR *pType,
            /* [out] */ BYTE __RPC_FAR *pValue,
            /* [out][in] */ WORD __RPC_FAR *pcbLength);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOutputFormat )( 
            IWMAudioReader __RPC_FAR * This,
            /* [out] */ WAVEFORMATEX __RPC_FAR *pWfx,
            /* [in] */ DWORD cbSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Start )( 
            IWMAudioReader __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stop )( 
            IWMAudioReader __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Seek )( 
            IWMAudioReader __RPC_FAR * This,
            DWORD dwMsTime);
        
        END_INTERFACE
    } IWMAudioReaderVtbl;

    interface IWMAudioReader
    {
        CONST_VTBL struct IWMAudioReaderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMAudioReader_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMAudioReader_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMAudioReader_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMAudioReader_GetAttributeByName(This,pszName,pType,pValue,pcbLength)	\
    (This)->lpVtbl -> GetAttributeByName(This,pszName,pType,pValue,pcbLength)

#define IWMAudioReader_GetAttributeCount(This,pcAttributes)	\
    (This)->lpVtbl -> GetAttributeCount(This,pcAttributes)

#define IWMAudioReader_GetAttributeByIndex(This,wIndex,pwszName,pcbNameLen,pType,pValue,pcbLength)	\
    (This)->lpVtbl -> GetAttributeByIndex(This,wIndex,pwszName,pcbNameLen,pType,pValue,pcbLength)

#define IWMAudioReader_GetOutputFormat(This,pWfx,cbSize)	\
    (This)->lpVtbl -> GetOutputFormat(This,pWfx,cbSize)

#define IWMAudioReader_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define IWMAudioReader_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IWMAudioReader_Seek(This,dwMsTime)	\
    (This)->lpVtbl -> Seek(This,dwMsTime)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWMAudioReader_GetAttributeByName_Proxy( 
    IWMAudioReader __RPC_FAR * This,
    /* [in] */ LPCWSTR pszName,
    /* [out] */ WMT_ATTR_DATATYPE __RPC_FAR *pType,
    /* [out] */ BYTE __RPC_FAR *pValue,
    /* [out][in] */ WORD __RPC_FAR *pcbLength);


void __RPC_STUB IWMAudioReader_GetAttributeByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioReader_GetAttributeCount_Proxy( 
    IWMAudioReader __RPC_FAR * This,
    /* [out] */ WORD __RPC_FAR *pcAttributes);


void __RPC_STUB IWMAudioReader_GetAttributeCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioReader_GetAttributeByIndex_Proxy( 
    IWMAudioReader __RPC_FAR * This,
    /* [in] */ WORD wIndex,
    /* [out] */ WCHAR __RPC_FAR *pwszName,
    /* [out][in] */ WORD __RPC_FAR *pcbNameLen,
    /* [out] */ WMT_ATTR_DATATYPE __RPC_FAR *pType,
    /* [out] */ BYTE __RPC_FAR *pValue,
    /* [out][in] */ WORD __RPC_FAR *pcbLength);


void __RPC_STUB IWMAudioReader_GetAttributeByIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioReader_GetOutputFormat_Proxy( 
    IWMAudioReader __RPC_FAR * This,
    /* [out] */ WAVEFORMATEX __RPC_FAR *pWfx,
    /* [in] */ DWORD cbSize);


void __RPC_STUB IWMAudioReader_GetOutputFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioReader_Start_Proxy( 
    IWMAudioReader __RPC_FAR * This);


void __RPC_STUB IWMAudioReader_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioReader_Stop_Proxy( 
    IWMAudioReader __RPC_FAR * This);


void __RPC_STUB IWMAudioReader_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioReader_Seek_Proxy( 
    IWMAudioReader __RPC_FAR * This,
    DWORD dwMsTime);


void __RPC_STUB IWMAudioReader_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWMAudioReader_INTERFACE_DEFINED__ */


#ifndef __IWMAudioReadCallback_INTERFACE_DEFINED__
#define __IWMAudioReadCallback_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IWMAudioReadCallback
 * at Mon Aug 09 13:10:31 1999
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IWMAudioReadCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("1A5636F3-DB5E-11d2-9D41-0060083178AF")
    IWMAudioReadCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnSample( 
            /* [in] */ const BYTE __RPC_FAR *pData,
            /* [in] */ DWORD cbData,
            /* [in] */ DWORD dwMsTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnStatus( 
            /* [in] */ WMT_STATUS Status,
            /* [in] */ HRESULT hr,
            /* [in] */ const VARIANT __RPC_FAR *pParam) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWMAudioReadCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMAudioReadCallback __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMAudioReadCallback __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMAudioReadCallback __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnSample )( 
            IWMAudioReadCallback __RPC_FAR * This,
            /* [in] */ const BYTE __RPC_FAR *pData,
            /* [in] */ DWORD cbData,
            /* [in] */ DWORD dwMsTime);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnStatus )( 
            IWMAudioReadCallback __RPC_FAR * This,
            /* [in] */ WMT_STATUS Status,
            /* [in] */ HRESULT hr,
            /* [in] */ const VARIANT __RPC_FAR *pParam);
        
        END_INTERFACE
    } IWMAudioReadCallbackVtbl;

    interface IWMAudioReadCallback
    {
        CONST_VTBL struct IWMAudioReadCallbackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMAudioReadCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMAudioReadCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMAudioReadCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMAudioReadCallback_OnSample(This,pData,cbData,dwMsTime)	\
    (This)->lpVtbl -> OnSample(This,pData,cbData,dwMsTime)

#define IWMAudioReadCallback_OnStatus(This,Status,hr,pParam)	\
    (This)->lpVtbl -> OnStatus(This,Status,hr,pParam)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWMAudioReadCallback_OnSample_Proxy( 
    IWMAudioReadCallback __RPC_FAR * This,
    /* [in] */ const BYTE __RPC_FAR *pData,
    /* [in] */ DWORD cbData,
    /* [in] */ DWORD dwMsTime);


void __RPC_STUB IWMAudioReadCallback_OnSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioReadCallback_OnStatus_Proxy( 
    IWMAudioReadCallback __RPC_FAR * This,
    /* [in] */ WMT_STATUS Status,
    /* [in] */ HRESULT hr,
    /* [in] */ const VARIANT __RPC_FAR *pParam);


void __RPC_STUB IWMAudioReadCallback_OnStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWMAudioReadCallback_INTERFACE_DEFINED__ */


#ifndef __IWMAudioInfo_INTERFACE_DEFINED__
#define __IWMAudioInfo_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IWMAudioInfo
 * at Mon Aug 09 13:10:31 1999
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IWMAudioInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0AA139F0-F6A8-11d2-97F7-00A0C95EA850")
    IWMAudioInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAttributeCount( 
            /* [out] */ WORD __RPC_FAR *pcAttributes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributeByIndex( 
            /* [in] */ WORD wIndex,
            /* [out] */ WCHAR __RPC_FAR *pwszName,
            /* [out][in] */ WORD __RPC_FAR *pcbNameLen,
            /* [out] */ WMT_ATTR_DATATYPE __RPC_FAR *pType,
            /* [out] */ BYTE __RPC_FAR *pValue,
            /* [out][in] */ WORD __RPC_FAR *pcbLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributeByName( 
            /* [in] */ LPCWSTR pszName,
            /* [out] */ WMT_ATTR_DATATYPE __RPC_FAR *pType,
            /* [out] */ BYTE __RPC_FAR *pValue,
            /* [out][in] */ WORD __RPC_FAR *pcbLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAttribute( 
            /* [in] */ LPCWSTR pszName,
            /* [in] */ WMT_ATTR_DATATYPE Type,
            /* [in] */ const BYTE __RPC_FAR *pValue,
            /* [in] */ WORD cbLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWMAudioInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMAudioInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMAudioInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMAudioInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAttributeCount )( 
            IWMAudioInfo __RPC_FAR * This,
            /* [out] */ WORD __RPC_FAR *pcAttributes);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAttributeByIndex )( 
            IWMAudioInfo __RPC_FAR * This,
            /* [in] */ WORD wIndex,
            /* [out] */ WCHAR __RPC_FAR *pwszName,
            /* [out][in] */ WORD __RPC_FAR *pcbNameLen,
            /* [out] */ WMT_ATTR_DATATYPE __RPC_FAR *pType,
            /* [out] */ BYTE __RPC_FAR *pValue,
            /* [out][in] */ WORD __RPC_FAR *pcbLength);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAttributeByName )( 
            IWMAudioInfo __RPC_FAR * This,
            /* [in] */ LPCWSTR pszName,
            /* [out] */ WMT_ATTR_DATATYPE __RPC_FAR *pType,
            /* [out] */ BYTE __RPC_FAR *pValue,
            /* [out][in] */ WORD __RPC_FAR *pcbLength);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAttribute )( 
            IWMAudioInfo __RPC_FAR * This,
            /* [in] */ LPCWSTR pszName,
            /* [in] */ WMT_ATTR_DATATYPE Type,
            /* [in] */ const BYTE __RPC_FAR *pValue,
            /* [in] */ WORD cbLength);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IWMAudioInfo __RPC_FAR * This);
        
        END_INTERFACE
    } IWMAudioInfoVtbl;

    interface IWMAudioInfo
    {
        CONST_VTBL struct IWMAudioInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMAudioInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMAudioInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMAudioInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMAudioInfo_GetAttributeCount(This,pcAttributes)	\
    (This)->lpVtbl -> GetAttributeCount(This,pcAttributes)

#define IWMAudioInfo_GetAttributeByIndex(This,wIndex,pwszName,pcbNameLen,pType,pValue,pcbLength)	\
    (This)->lpVtbl -> GetAttributeByIndex(This,wIndex,pwszName,pcbNameLen,pType,pValue,pcbLength)

#define IWMAudioInfo_GetAttributeByName(This,pszName,pType,pValue,pcbLength)	\
    (This)->lpVtbl -> GetAttributeByName(This,pszName,pType,pValue,pcbLength)

#define IWMAudioInfo_SetAttribute(This,pszName,Type,pValue,cbLength)	\
    (This)->lpVtbl -> SetAttribute(This,pszName,Type,pValue,cbLength)

#define IWMAudioInfo_Close(This)	\
    (This)->lpVtbl -> Close(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWMAudioInfo_GetAttributeCount_Proxy( 
    IWMAudioInfo __RPC_FAR * This,
    /* [out] */ WORD __RPC_FAR *pcAttributes);


void __RPC_STUB IWMAudioInfo_GetAttributeCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioInfo_GetAttributeByIndex_Proxy( 
    IWMAudioInfo __RPC_FAR * This,
    /* [in] */ WORD wIndex,
    /* [out] */ WCHAR __RPC_FAR *pwszName,
    /* [out][in] */ WORD __RPC_FAR *pcbNameLen,
    /* [out] */ WMT_ATTR_DATATYPE __RPC_FAR *pType,
    /* [out] */ BYTE __RPC_FAR *pValue,
    /* [out][in] */ WORD __RPC_FAR *pcbLength);


void __RPC_STUB IWMAudioInfo_GetAttributeByIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioInfo_GetAttributeByName_Proxy( 
    IWMAudioInfo __RPC_FAR * This,
    /* [in] */ LPCWSTR pszName,
    /* [out] */ WMT_ATTR_DATATYPE __RPC_FAR *pType,
    /* [out] */ BYTE __RPC_FAR *pValue,
    /* [out][in] */ WORD __RPC_FAR *pcbLength);


void __RPC_STUB IWMAudioInfo_GetAttributeByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioInfo_SetAttribute_Proxy( 
    IWMAudioInfo __RPC_FAR * This,
    /* [in] */ LPCWSTR pszName,
    /* [in] */ WMT_ATTR_DATATYPE Type,
    /* [in] */ const BYTE __RPC_FAR *pValue,
    /* [in] */ WORD cbLength);


void __RPC_STUB IWMAudioInfo_SetAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAudioInfo_Close_Proxy( 
    IWMAudioInfo __RPC_FAR * This);


void __RPC_STUB IWMAudioInfo_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWMAudioInfo_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
