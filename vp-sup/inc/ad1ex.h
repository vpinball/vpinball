/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.01.75 */
/* at Fri Sep 18 16:27:35 1998
 */
/* Compiler settings for ad1ex.idl:
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

#ifndef __ad1ex_h__
#define __ad1ex_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IDebugApplicationEx_FWD_DEFINED__
#define __IDebugApplicationEx_FWD_DEFINED__
typedef interface IDebugApplicationEx IDebugApplicationEx;
#endif 	/* __IDebugApplicationEx_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplicationEx_FWD_DEFINED__
#define __IRemoteDebugApplicationEx_FWD_DEFINED__
typedef interface IRemoteDebugApplicationEx IRemoteDebugApplicationEx;
#endif 	/* __IRemoteDebugApplicationEx_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplicationThreadEx_FWD_DEFINED__
#define __IRemoteDebugApplicationThreadEx_FWD_DEFINED__
typedef interface IRemoteDebugApplicationThreadEx IRemoteDebugApplicationThreadEx;
#endif 	/* __IRemoteDebugApplicationThreadEx_FWD_DEFINED__ */


#ifndef __IDebugDocumentHelperEx_FWD_DEFINED__
#define __IDebugDocumentHelperEx_FWD_DEFINED__
typedef interface IDebugDocumentHelperEx IDebugDocumentHelperEx;
#endif 	/* __IDebugDocumentHelperEx_FWD_DEFINED__ */


#ifndef __IDebugHelperEx_FWD_DEFINED__
#define __IDebugHelperEx_FWD_DEFINED__
typedef interface IDebugHelperEx IDebugHelperEx;
#endif 	/* __IDebugHelperEx_FWD_DEFINED__ */


#ifndef __IDebugSetValueCallback_FWD_DEFINED__
#define __IDebugSetValueCallback_FWD_DEFINED__
typedef interface IDebugSetValueCallback IDebugSetValueCallback;
#endif 	/* __IDebugSetValueCallback_FWD_DEFINED__ */


#ifndef __ISetNextStatement_FWD_DEFINED__
#define __ISetNextStatement_FWD_DEFINED__
typedef interface ISetNextStatement ISetNextStatement;
#endif 	/* __ISetNextStatement_FWD_DEFINED__ */


#ifndef __IDebugSessionProviderEx_FWD_DEFINED__
#define __IDebugSessionProviderEx_FWD_DEFINED__
typedef interface IDebugSessionProviderEx IDebugSessionProviderEx;
#endif 	/* __IDebugSessionProviderEx_FWD_DEFINED__ */


/* header files for imported files */
#include "activdbg.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL_itf_ad1ex_0000
 * at Fri Sep 18 16:27:35 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local] */ 









DEFINE_GUID(IID_IDebugHelperExOld, 0xE0284F00, 0xEDA1, 0x11d0, 0xB4, 0x52, 0x00, 0xA0, 0x24, 0x4A, 0x1D, 0xD2);


extern RPC_IF_HANDLE __MIDL_itf_ad1ex_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ad1ex_0000_v0_0_s_ifspec;

#ifndef __IDebugApplicationEx_INTERFACE_DEFINED__
#define __IDebugApplicationEx_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugApplicationEx
 * at Fri Sep 18 16:27:35 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugApplicationEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C00-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugApplicationEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE onCallEnter( 
            /* [in] */ DWORD dwLim) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onCallOut( 
            /* [in] */ DWORD dwLim,
            /* [in] */ DWORD dwAddrDest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onCallReturn( 
            /* [in] */ DWORD dwLim) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onCallExit( 
            /* [in] */ DWORD dwLim,
            /* [in] */ DWORD dwAddrDest) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugApplicationExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugApplicationEx __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugApplicationEx __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugApplicationEx __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onCallEnter )( 
            IDebugApplicationEx __RPC_FAR * This,
            /* [in] */ DWORD dwLim);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onCallOut )( 
            IDebugApplicationEx __RPC_FAR * This,
            /* [in] */ DWORD dwLim,
            /* [in] */ DWORD dwAddrDest);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onCallReturn )( 
            IDebugApplicationEx __RPC_FAR * This,
            /* [in] */ DWORD dwLim);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onCallExit )( 
            IDebugApplicationEx __RPC_FAR * This,
            /* [in] */ DWORD dwLim,
            /* [in] */ DWORD dwAddrDest);
        
        END_INTERFACE
    } IDebugApplicationExVtbl;

    interface IDebugApplicationEx
    {
        CONST_VTBL struct IDebugApplicationExVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugApplicationEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugApplicationEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugApplicationEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugApplicationEx_onCallEnter(This,dwLim)	\
    (This)->lpVtbl -> onCallEnter(This,dwLim)

#define IDebugApplicationEx_onCallOut(This,dwLim,dwAddrDest)	\
    (This)->lpVtbl -> onCallOut(This,dwLim,dwAddrDest)

#define IDebugApplicationEx_onCallReturn(This,dwLim)	\
    (This)->lpVtbl -> onCallReturn(This,dwLim)

#define IDebugApplicationEx_onCallExit(This,dwLim,dwAddrDest)	\
    (This)->lpVtbl -> onCallExit(This,dwLim,dwAddrDest)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugApplicationEx_onCallEnter_Proxy( 
    IDebugApplicationEx __RPC_FAR * This,
    /* [in] */ DWORD dwLim);


void __RPC_STUB IDebugApplicationEx_onCallEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationEx_onCallOut_Proxy( 
    IDebugApplicationEx __RPC_FAR * This,
    /* [in] */ DWORD dwLim,
    /* [in] */ DWORD dwAddrDest);


void __RPC_STUB IDebugApplicationEx_onCallOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationEx_onCallReturn_Proxy( 
    IDebugApplicationEx __RPC_FAR * This,
    /* [in] */ DWORD dwLim);


void __RPC_STUB IDebugApplicationEx_onCallReturn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationEx_onCallExit_Proxy( 
    IDebugApplicationEx __RPC_FAR * This,
    /* [in] */ DWORD dwLim,
    /* [in] */ DWORD dwAddrDest);


void __RPC_STUB IDebugApplicationEx_onCallExit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugApplicationEx_INTERFACE_DEFINED__ */


#ifndef __IRemoteDebugApplicationEx_INTERFACE_DEFINED__
#define __IRemoteDebugApplicationEx_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRemoteDebugApplicationEx
 * at Fri Sep 18 16:27:35 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IRemoteDebugApplicationEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C01-CB0C-11d0-B5C9-00A0244A0E7A")
    IRemoteDebugApplicationEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetHostPid( 
            /* [out] */ DWORD __RPC_FAR *dwHostPid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHostMachineName( 
            /* [out] */ BSTR __RPC_FAR *pbstrHostMachineName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLocale( 
            /* [in] */ DWORD dwLangID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ForceStepMode( 
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *pStepThread) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeBreak( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProxyBlanketAndAddRef( 
            /* [in] */ IUnknown __RPC_FAR *pUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseFromSetProxyBlanket( 
            /* [in] */ IUnknown __RPC_FAR *pUnk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRemoteDebugApplicationExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRemoteDebugApplicationEx __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRemoteDebugApplicationEx __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRemoteDebugApplicationEx __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHostPid )( 
            IRemoteDebugApplicationEx __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *dwHostPid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHostMachineName )( 
            IRemoteDebugApplicationEx __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrHostMachineName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLocale )( 
            IRemoteDebugApplicationEx __RPC_FAR * This,
            /* [in] */ DWORD dwLangID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ForceStepMode )( 
            IRemoteDebugApplicationEx __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *pStepThread);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RevokeBreak )( 
            IRemoteDebugApplicationEx __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProxyBlanketAndAddRef )( 
            IRemoteDebugApplicationEx __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseFromSetProxyBlanket )( 
            IRemoteDebugApplicationEx __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnk);
        
        END_INTERFACE
    } IRemoteDebugApplicationExVtbl;

    interface IRemoteDebugApplicationEx
    {
        CONST_VTBL struct IRemoteDebugApplicationExVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteDebugApplicationEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteDebugApplicationEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteDebugApplicationEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteDebugApplicationEx_GetHostPid(This,dwHostPid)	\
    (This)->lpVtbl -> GetHostPid(This,dwHostPid)

#define IRemoteDebugApplicationEx_GetHostMachineName(This,pbstrHostMachineName)	\
    (This)->lpVtbl -> GetHostMachineName(This,pbstrHostMachineName)

#define IRemoteDebugApplicationEx_SetLocale(This,dwLangID)	\
    (This)->lpVtbl -> SetLocale(This,dwLangID)

#define IRemoteDebugApplicationEx_ForceStepMode(This,pStepThread)	\
    (This)->lpVtbl -> ForceStepMode(This,pStepThread)

#define IRemoteDebugApplicationEx_RevokeBreak(This)	\
    (This)->lpVtbl -> RevokeBreak(This)

#define IRemoteDebugApplicationEx_SetProxyBlanketAndAddRef(This,pUnk)	\
    (This)->lpVtbl -> SetProxyBlanketAndAddRef(This,pUnk)

#define IRemoteDebugApplicationEx_ReleaseFromSetProxyBlanket(This,pUnk)	\
    (This)->lpVtbl -> ReleaseFromSetProxyBlanket(This,pUnk)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEx_GetHostPid_Proxy( 
    IRemoteDebugApplicationEx __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *dwHostPid);


void __RPC_STUB IRemoteDebugApplicationEx_GetHostPid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEx_GetHostMachineName_Proxy( 
    IRemoteDebugApplicationEx __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrHostMachineName);


void __RPC_STUB IRemoteDebugApplicationEx_GetHostMachineName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEx_SetLocale_Proxy( 
    IRemoteDebugApplicationEx __RPC_FAR * This,
    /* [in] */ DWORD dwLangID);


void __RPC_STUB IRemoteDebugApplicationEx_SetLocale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEx_ForceStepMode_Proxy( 
    IRemoteDebugApplicationEx __RPC_FAR * This,
    /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *pStepThread);


void __RPC_STUB IRemoteDebugApplicationEx_ForceStepMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEx_RevokeBreak_Proxy( 
    IRemoteDebugApplicationEx __RPC_FAR * This);


void __RPC_STUB IRemoteDebugApplicationEx_RevokeBreak_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEx_SetProxyBlanketAndAddRef_Proxy( 
    IRemoteDebugApplicationEx __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnk);


void __RPC_STUB IRemoteDebugApplicationEx_SetProxyBlanketAndAddRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEx_ReleaseFromSetProxyBlanket_Proxy( 
    IRemoteDebugApplicationEx __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnk);


void __RPC_STUB IRemoteDebugApplicationEx_ReleaseFromSetProxyBlanket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRemoteDebugApplicationEx_INTERFACE_DEFINED__ */


#ifndef __IRemoteDebugApplicationThreadEx_INTERFACE_DEFINED__
#define __IRemoteDebugApplicationThreadEx_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRemoteDebugApplicationThreadEx
 * at Fri Sep 18 16:27:35 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IRemoteDebugApplicationThreadEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("B9B32B0C-9147-11d1-94EA-00C04FA302A1")
    IRemoteDebugApplicationThreadEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumGlobalExpressionContexts( 
            /* [out] */ IEnumDebugExpressionContexts __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRemoteDebugApplicationThreadExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRemoteDebugApplicationThreadEx __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRemoteDebugApplicationThreadEx __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRemoteDebugApplicationThreadEx __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumGlobalExpressionContexts )( 
            IRemoteDebugApplicationThreadEx __RPC_FAR * This,
            /* [out] */ IEnumDebugExpressionContexts __RPC_FAR *__RPC_FAR *ppEnum);
        
        END_INTERFACE
    } IRemoteDebugApplicationThreadExVtbl;

    interface IRemoteDebugApplicationThreadEx
    {
        CONST_VTBL struct IRemoteDebugApplicationThreadExVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteDebugApplicationThreadEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteDebugApplicationThreadEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteDebugApplicationThreadEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteDebugApplicationThreadEx_EnumGlobalExpressionContexts(This,ppEnum)	\
    (This)->lpVtbl -> EnumGlobalExpressionContexts(This,ppEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThreadEx_EnumGlobalExpressionContexts_Proxy( 
    IRemoteDebugApplicationThreadEx __RPC_FAR * This,
    /* [out] */ IEnumDebugExpressionContexts __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB IRemoteDebugApplicationThreadEx_EnumGlobalExpressionContexts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRemoteDebugApplicationThreadEx_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentHelperEx_INTERFACE_DEFINED__
#define __IDebugDocumentHelperEx_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugDocumentHelperEx
 * at Fri Sep 18 16:27:35 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugDocumentHelperEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C02-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentHelperEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetDocumentClassId( 
            /* [in] */ CLSID clsidDocument) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentHelperExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugDocumentHelperEx __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugDocumentHelperEx __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugDocumentHelperEx __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDocumentClassId )( 
            IDebugDocumentHelperEx __RPC_FAR * This,
            /* [in] */ CLSID clsidDocument);
        
        END_INTERFACE
    } IDebugDocumentHelperExVtbl;

    interface IDebugDocumentHelperEx
    {
        CONST_VTBL struct IDebugDocumentHelperExVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentHelperEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentHelperEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentHelperEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentHelperEx_SetDocumentClassId(This,clsidDocument)	\
    (This)->lpVtbl -> SetDocumentClassId(This,clsidDocument)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentHelperEx_SetDocumentClassId_Proxy( 
    IDebugDocumentHelperEx __RPC_FAR * This,
    /* [in] */ CLSID clsidDocument);


void __RPC_STUB IDebugDocumentHelperEx_SetDocumentClassId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentHelperEx_INTERFACE_DEFINED__ */


#ifndef __IDebugHelperEx_INTERFACE_DEFINED__
#define __IDebugHelperEx_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugHelperEx
 * at Fri Sep 18 16:27:35 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IDebugHelperEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C08-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugHelperEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreatePropertyBrowserFromError( 
            /* [in] */ IActiveScriptError __RPC_FAR *pase,
            /* [in] */ LPCOLESTR pszName,
            /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
            /* [in] */ IDebugFormatter __RPC_FAR *pdf,
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateWriteablePropertyBrowser( 
            /* [in] */ VARIANT __RPC_FAR *pvar,
            /* [in] */ LPCOLESTR bstrName,
            /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
            /* [in] */ IDebugFormatter __RPC_FAR *pdf,
            /* [in] */ IDebugSetValueCallback __RPC_FAR *pdsvcb,
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreatePropertyBrowserFromCodeContext( 
            /* [in] */ IDebugCodeContext __RPC_FAR *pdcc,
            /* [in] */ LPCOLESTR pszName,
            /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugHelperExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugHelperEx __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugHelperEx __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugHelperEx __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreatePropertyBrowserFromError )( 
            IDebugHelperEx __RPC_FAR * This,
            /* [in] */ IActiveScriptError __RPC_FAR *pase,
            /* [in] */ LPCOLESTR pszName,
            /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
            /* [in] */ IDebugFormatter __RPC_FAR *pdf,
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdp);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateWriteablePropertyBrowser )( 
            IDebugHelperEx __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *pvar,
            /* [in] */ LPCOLESTR bstrName,
            /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
            /* [in] */ IDebugFormatter __RPC_FAR *pdf,
            /* [in] */ IDebugSetValueCallback __RPC_FAR *pdsvcb,
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdob);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreatePropertyBrowserFromCodeContext )( 
            IDebugHelperEx __RPC_FAR * This,
            /* [in] */ IDebugCodeContext __RPC_FAR *pdcc,
            /* [in] */ LPCOLESTR pszName,
            /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdp);
        
        END_INTERFACE
    } IDebugHelperExVtbl;

    interface IDebugHelperEx
    {
        CONST_VTBL struct IDebugHelperExVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugHelperEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugHelperEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugHelperEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugHelperEx_CreatePropertyBrowserFromError(This,pase,pszName,pdat,pdf,ppdp)	\
    (This)->lpVtbl -> CreatePropertyBrowserFromError(This,pase,pszName,pdat,pdf,ppdp)

#define IDebugHelperEx_CreateWriteablePropertyBrowser(This,pvar,bstrName,pdat,pdf,pdsvcb,ppdob)	\
    (This)->lpVtbl -> CreateWriteablePropertyBrowser(This,pvar,bstrName,pdat,pdf,pdsvcb,ppdob)

#define IDebugHelperEx_CreatePropertyBrowserFromCodeContext(This,pdcc,pszName,pdat,ppdp)	\
    (This)->lpVtbl -> CreatePropertyBrowserFromCodeContext(This,pdcc,pszName,pdat,ppdp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugHelperEx_CreatePropertyBrowserFromError_Proxy( 
    IDebugHelperEx __RPC_FAR * This,
    /* [in] */ IActiveScriptError __RPC_FAR *pase,
    /* [in] */ LPCOLESTR pszName,
    /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
    /* [in] */ IDebugFormatter __RPC_FAR *pdf,
    /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdp);


void __RPC_STUB IDebugHelperEx_CreatePropertyBrowserFromError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugHelperEx_CreateWriteablePropertyBrowser_Proxy( 
    IDebugHelperEx __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *pvar,
    /* [in] */ LPCOLESTR bstrName,
    /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
    /* [in] */ IDebugFormatter __RPC_FAR *pdf,
    /* [in] */ IDebugSetValueCallback __RPC_FAR *pdsvcb,
    /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdob);


void __RPC_STUB IDebugHelperEx_CreateWriteablePropertyBrowser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugHelperEx_CreatePropertyBrowserFromCodeContext_Proxy( 
    IDebugHelperEx __RPC_FAR * This,
    /* [in] */ IDebugCodeContext __RPC_FAR *pdcc,
    /* [in] */ LPCOLESTR pszName,
    /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
    /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdp);


void __RPC_STUB IDebugHelperEx_CreatePropertyBrowserFromCodeContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugHelperEx_INTERFACE_DEFINED__ */


#ifndef __IDebugSetValueCallback_INTERFACE_DEFINED__
#define __IDebugSetValueCallback_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugSetValueCallback
 * at Fri Sep 18 16:27:35 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IDebugSetValueCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C06-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugSetValueCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetValue( 
            /* [in] */ VARIANT __RPC_FAR *pvarNode,
            /* [in] */ DISPID dispid,
            /* [in] */ ULONG cIndices,
            /* [size_is][in] */ LONG __RPC_FAR *rgIndices,
            /* [in] */ LPCOLESTR pszValue,
            /* [in] */ UINT nRadix,
            /* [out] */ BSTR __RPC_FAR *pbstrError) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugSetValueCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugSetValueCallback __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugSetValueCallback __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugSetValueCallback __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetValue )( 
            IDebugSetValueCallback __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *pvarNode,
            /* [in] */ DISPID dispid,
            /* [in] */ ULONG cIndices,
            /* [size_is][in] */ LONG __RPC_FAR *rgIndices,
            /* [in] */ LPCOLESTR pszValue,
            /* [in] */ UINT nRadix,
            /* [out] */ BSTR __RPC_FAR *pbstrError);
        
        END_INTERFACE
    } IDebugSetValueCallbackVtbl;

    interface IDebugSetValueCallback
    {
        CONST_VTBL struct IDebugSetValueCallbackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugSetValueCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugSetValueCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugSetValueCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugSetValueCallback_SetValue(This,pvarNode,dispid,cIndices,rgIndices,pszValue,nRadix,pbstrError)	\
    (This)->lpVtbl -> SetValue(This,pvarNode,dispid,cIndices,rgIndices,pszValue,nRadix,pbstrError)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugSetValueCallback_SetValue_Proxy( 
    IDebugSetValueCallback __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *pvarNode,
    /* [in] */ DISPID dispid,
    /* [in] */ ULONG cIndices,
    /* [size_is][in] */ LONG __RPC_FAR *rgIndices,
    /* [in] */ LPCOLESTR pszValue,
    /* [in] */ UINT nRadix,
    /* [out] */ BSTR __RPC_FAR *pbstrError);


void __RPC_STUB IDebugSetValueCallback_SetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugSetValueCallback_INTERFACE_DEFINED__ */


#ifndef __ISetNextStatement_INTERFACE_DEFINED__
#define __ISetNextStatement_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ISetNextStatement
 * at Fri Sep 18 16:27:35 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_ISetNextStatement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C03-CB0C-11d0-B5C9-00A0244A0E7A")
    ISetNextStatement : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CanSetNextStatement( 
            /* [in] */ IDebugStackFrame __RPC_FAR *pStackFrame,
            /* [in] */ IDebugCodeContext __RPC_FAR *pCodeContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNextStatement( 
            /* [in] */ IDebugStackFrame __RPC_FAR *pStackFrame,
            /* [in] */ IDebugCodeContext __RPC_FAR *pCodeContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISetNextStatementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISetNextStatement __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISetNextStatement __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISetNextStatement __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CanSetNextStatement )( 
            ISetNextStatement __RPC_FAR * This,
            /* [in] */ IDebugStackFrame __RPC_FAR *pStackFrame,
            /* [in] */ IDebugCodeContext __RPC_FAR *pCodeContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetNextStatement )( 
            ISetNextStatement __RPC_FAR * This,
            /* [in] */ IDebugStackFrame __RPC_FAR *pStackFrame,
            /* [in] */ IDebugCodeContext __RPC_FAR *pCodeContext);
        
        END_INTERFACE
    } ISetNextStatementVtbl;

    interface ISetNextStatement
    {
        CONST_VTBL struct ISetNextStatementVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISetNextStatement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISetNextStatement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISetNextStatement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISetNextStatement_CanSetNextStatement(This,pStackFrame,pCodeContext)	\
    (This)->lpVtbl -> CanSetNextStatement(This,pStackFrame,pCodeContext)

#define ISetNextStatement_SetNextStatement(This,pStackFrame,pCodeContext)	\
    (This)->lpVtbl -> SetNextStatement(This,pStackFrame,pCodeContext)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISetNextStatement_CanSetNextStatement_Proxy( 
    ISetNextStatement __RPC_FAR * This,
    /* [in] */ IDebugStackFrame __RPC_FAR *pStackFrame,
    /* [in] */ IDebugCodeContext __RPC_FAR *pCodeContext);


void __RPC_STUB ISetNextStatement_CanSetNextStatement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISetNextStatement_SetNextStatement_Proxy( 
    ISetNextStatement __RPC_FAR * This,
    /* [in] */ IDebugStackFrame __RPC_FAR *pStackFrame,
    /* [in] */ IDebugCodeContext __RPC_FAR *pCodeContext);


void __RPC_STUB ISetNextStatement_SetNextStatement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISetNextStatement_INTERFACE_DEFINED__ */


#ifndef __IDebugSessionProviderEx_INTERFACE_DEFINED__
#define __IDebugSessionProviderEx_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugSessionProviderEx
 * at Fri Sep 18 16:27:35 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IDebugSessionProviderEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C09-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugSessionProviderEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE StartDebugSession( 
            /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
            /* [in] */ BOOL fQuery) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CanJITDebug( 
            /* [in] */ DWORD pid) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugSessionProviderExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugSessionProviderEx __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugSessionProviderEx __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugSessionProviderEx __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StartDebugSession )( 
            IDebugSessionProviderEx __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
            /* [in] */ BOOL fQuery);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CanJITDebug )( 
            IDebugSessionProviderEx __RPC_FAR * This,
            /* [in] */ DWORD pid);
        
        END_INTERFACE
    } IDebugSessionProviderExVtbl;

    interface IDebugSessionProviderEx
    {
        CONST_VTBL struct IDebugSessionProviderExVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugSessionProviderEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugSessionProviderEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugSessionProviderEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugSessionProviderEx_StartDebugSession(This,pda,fQuery)	\
    (This)->lpVtbl -> StartDebugSession(This,pda,fQuery)

#define IDebugSessionProviderEx_CanJITDebug(This,pid)	\
    (This)->lpVtbl -> CanJITDebug(This,pid)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugSessionProviderEx_StartDebugSession_Proxy( 
    IDebugSessionProviderEx __RPC_FAR * This,
    /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
    /* [in] */ BOOL fQuery);


void __RPC_STUB IDebugSessionProviderEx_StartDebugSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugSessionProviderEx_CanJITDebug_Proxy( 
    IDebugSessionProviderEx __RPC_FAR * This,
    /* [in] */ DWORD pid);


void __RPC_STUB IDebugSessionProviderEx_CanJITDebug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugSessionProviderEx_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
