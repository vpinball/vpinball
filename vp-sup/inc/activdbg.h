/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.01.75 */
/* at Fri Sep 18 16:27:25 1998
 */
/* Compiler settings for activdbg.idl:
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

#ifndef __activdbg_h__
#define __activdbg_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IActiveScriptDebug_FWD_DEFINED__
#define __IActiveScriptDebug_FWD_DEFINED__
typedef interface IActiveScriptDebug IActiveScriptDebug;
#endif 	/* __IActiveScriptDebug_FWD_DEFINED__ */


#ifndef __IActiveScriptSiteDebug_FWD_DEFINED__
#define __IActiveScriptSiteDebug_FWD_DEFINED__
typedef interface IActiveScriptSiteDebug IActiveScriptSiteDebug;
#endif 	/* __IActiveScriptSiteDebug_FWD_DEFINED__ */


#ifndef __IActiveScriptErrorDebug_FWD_DEFINED__
#define __IActiveScriptErrorDebug_FWD_DEFINED__
typedef interface IActiveScriptErrorDebug IActiveScriptErrorDebug;
#endif 	/* __IActiveScriptErrorDebug_FWD_DEFINED__ */


#ifndef __IDebugCodeContext_FWD_DEFINED__
#define __IDebugCodeContext_FWD_DEFINED__
typedef interface IDebugCodeContext IDebugCodeContext;
#endif 	/* __IDebugCodeContext_FWD_DEFINED__ */


#ifndef __IDebugExpression_FWD_DEFINED__
#define __IDebugExpression_FWD_DEFINED__
typedef interface IDebugExpression IDebugExpression;
#endif 	/* __IDebugExpression_FWD_DEFINED__ */


#ifndef __IDebugExpressionContext_FWD_DEFINED__
#define __IDebugExpressionContext_FWD_DEFINED__
typedef interface IDebugExpressionContext IDebugExpressionContext;
#endif 	/* __IDebugExpressionContext_FWD_DEFINED__ */


#ifndef __IDebugExpressionCallBack_FWD_DEFINED__
#define __IDebugExpressionCallBack_FWD_DEFINED__
typedef interface IDebugExpressionCallBack IDebugExpressionCallBack;
#endif 	/* __IDebugExpressionCallBack_FWD_DEFINED__ */


#ifndef __IDebugStackFrame_FWD_DEFINED__
#define __IDebugStackFrame_FWD_DEFINED__
typedef interface IDebugStackFrame IDebugStackFrame;
#endif 	/* __IDebugStackFrame_FWD_DEFINED__ */


#ifndef __IDebugStackFrameSniffer_FWD_DEFINED__
#define __IDebugStackFrameSniffer_FWD_DEFINED__
typedef interface IDebugStackFrameSniffer IDebugStackFrameSniffer;
#endif 	/* __IDebugStackFrameSniffer_FWD_DEFINED__ */


#ifndef __IDebugStackFrameSnifferEx_FWD_DEFINED__
#define __IDebugStackFrameSnifferEx_FWD_DEFINED__
typedef interface IDebugStackFrameSnifferEx IDebugStackFrameSnifferEx;
#endif 	/* __IDebugStackFrameSnifferEx_FWD_DEFINED__ */


#ifndef __IDebugSyncOperation_FWD_DEFINED__
#define __IDebugSyncOperation_FWD_DEFINED__
typedef interface IDebugSyncOperation IDebugSyncOperation;
#endif 	/* __IDebugSyncOperation_FWD_DEFINED__ */


#ifndef __IDebugAsyncOperation_FWD_DEFINED__
#define __IDebugAsyncOperation_FWD_DEFINED__
typedef interface IDebugAsyncOperation IDebugAsyncOperation;
#endif 	/* __IDebugAsyncOperation_FWD_DEFINED__ */


#ifndef __IDebugAsyncOperationCallBack_FWD_DEFINED__
#define __IDebugAsyncOperationCallBack_FWD_DEFINED__
typedef interface IDebugAsyncOperationCallBack IDebugAsyncOperationCallBack;
#endif 	/* __IDebugAsyncOperationCallBack_FWD_DEFINED__ */


#ifndef __IEnumDebugCodeContexts_FWD_DEFINED__
#define __IEnumDebugCodeContexts_FWD_DEFINED__
typedef interface IEnumDebugCodeContexts IEnumDebugCodeContexts;
#endif 	/* __IEnumDebugCodeContexts_FWD_DEFINED__ */


#ifndef __IEnumDebugStackFrames_FWD_DEFINED__
#define __IEnumDebugStackFrames_FWD_DEFINED__
typedef interface IEnumDebugStackFrames IEnumDebugStackFrames;
#endif 	/* __IEnumDebugStackFrames_FWD_DEFINED__ */


#ifndef __IDebugDocumentInfo_FWD_DEFINED__
#define __IDebugDocumentInfo_FWD_DEFINED__
typedef interface IDebugDocumentInfo IDebugDocumentInfo;
#endif 	/* __IDebugDocumentInfo_FWD_DEFINED__ */


#ifndef __IDebugDocumentProvider_FWD_DEFINED__
#define __IDebugDocumentProvider_FWD_DEFINED__
typedef interface IDebugDocumentProvider IDebugDocumentProvider;
#endif 	/* __IDebugDocumentProvider_FWD_DEFINED__ */


#ifndef __IDebugDocument_FWD_DEFINED__
#define __IDebugDocument_FWD_DEFINED__
typedef interface IDebugDocument IDebugDocument;
#endif 	/* __IDebugDocument_FWD_DEFINED__ */


#ifndef __IDebugDocumentText_FWD_DEFINED__
#define __IDebugDocumentText_FWD_DEFINED__
typedef interface IDebugDocumentText IDebugDocumentText;
#endif 	/* __IDebugDocumentText_FWD_DEFINED__ */


#ifndef __IDebugDocumentTextEvents_FWD_DEFINED__
#define __IDebugDocumentTextEvents_FWD_DEFINED__
typedef interface IDebugDocumentTextEvents IDebugDocumentTextEvents;
#endif 	/* __IDebugDocumentTextEvents_FWD_DEFINED__ */


#ifndef __IDebugDocumentTextAuthor_FWD_DEFINED__
#define __IDebugDocumentTextAuthor_FWD_DEFINED__
typedef interface IDebugDocumentTextAuthor IDebugDocumentTextAuthor;
#endif 	/* __IDebugDocumentTextAuthor_FWD_DEFINED__ */


#ifndef __IDebugDocumentTextExternalAuthor_FWD_DEFINED__
#define __IDebugDocumentTextExternalAuthor_FWD_DEFINED__
typedef interface IDebugDocumentTextExternalAuthor IDebugDocumentTextExternalAuthor;
#endif 	/* __IDebugDocumentTextExternalAuthor_FWD_DEFINED__ */


#ifndef __IDebugDocumentHelper_FWD_DEFINED__
#define __IDebugDocumentHelper_FWD_DEFINED__
typedef interface IDebugDocumentHelper IDebugDocumentHelper;
#endif 	/* __IDebugDocumentHelper_FWD_DEFINED__ */


#ifndef __IDebugDocumentHost_FWD_DEFINED__
#define __IDebugDocumentHost_FWD_DEFINED__
typedef interface IDebugDocumentHost IDebugDocumentHost;
#endif 	/* __IDebugDocumentHost_FWD_DEFINED__ */


#ifndef __IDebugDocumentContext_FWD_DEFINED__
#define __IDebugDocumentContext_FWD_DEFINED__
typedef interface IDebugDocumentContext IDebugDocumentContext;
#endif 	/* __IDebugDocumentContext_FWD_DEFINED__ */


#ifndef __IDebugSessionProvider_FWD_DEFINED__
#define __IDebugSessionProvider_FWD_DEFINED__
typedef interface IDebugSessionProvider IDebugSessionProvider;
#endif 	/* __IDebugSessionProvider_FWD_DEFINED__ */


#ifndef __IApplicationDebugger_FWD_DEFINED__
#define __IApplicationDebugger_FWD_DEFINED__
typedef interface IApplicationDebugger IApplicationDebugger;
#endif 	/* __IApplicationDebugger_FWD_DEFINED__ */


#ifndef __IApplicationDebuggerUI_FWD_DEFINED__
#define __IApplicationDebuggerUI_FWD_DEFINED__
typedef interface IApplicationDebuggerUI IApplicationDebuggerUI;
#endif 	/* __IApplicationDebuggerUI_FWD_DEFINED__ */


#ifndef __IMachineDebugManager_FWD_DEFINED__
#define __IMachineDebugManager_FWD_DEFINED__
typedef interface IMachineDebugManager IMachineDebugManager;
#endif 	/* __IMachineDebugManager_FWD_DEFINED__ */


#ifndef __IMachineDebugManagerCookie_FWD_DEFINED__
#define __IMachineDebugManagerCookie_FWD_DEFINED__
typedef interface IMachineDebugManagerCookie IMachineDebugManagerCookie;
#endif 	/* __IMachineDebugManagerCookie_FWD_DEFINED__ */


#ifndef __IMachineDebugManagerEvents_FWD_DEFINED__
#define __IMachineDebugManagerEvents_FWD_DEFINED__
typedef interface IMachineDebugManagerEvents IMachineDebugManagerEvents;
#endif 	/* __IMachineDebugManagerEvents_FWD_DEFINED__ */


#ifndef __IProcessDebugManager_FWD_DEFINED__
#define __IProcessDebugManager_FWD_DEFINED__
typedef interface IProcessDebugManager IProcessDebugManager;
#endif 	/* __IProcessDebugManager_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplication_FWD_DEFINED__
#define __IRemoteDebugApplication_FWD_DEFINED__
typedef interface IRemoteDebugApplication IRemoteDebugApplication;
#endif 	/* __IRemoteDebugApplication_FWD_DEFINED__ */


#ifndef __IDebugApplication_FWD_DEFINED__
#define __IDebugApplication_FWD_DEFINED__
typedef interface IDebugApplication IDebugApplication;
#endif 	/* __IDebugApplication_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplicationEvents_FWD_DEFINED__
#define __IRemoteDebugApplicationEvents_FWD_DEFINED__
typedef interface IRemoteDebugApplicationEvents IRemoteDebugApplicationEvents;
#endif 	/* __IRemoteDebugApplicationEvents_FWD_DEFINED__ */


#ifndef __IDebugApplicationNode_FWD_DEFINED__
#define __IDebugApplicationNode_FWD_DEFINED__
typedef interface IDebugApplicationNode IDebugApplicationNode;
#endif 	/* __IDebugApplicationNode_FWD_DEFINED__ */


#ifndef __IDebugApplicationNodeEvents_FWD_DEFINED__
#define __IDebugApplicationNodeEvents_FWD_DEFINED__
typedef interface IDebugApplicationNodeEvents IDebugApplicationNodeEvents;
#endif 	/* __IDebugApplicationNodeEvents_FWD_DEFINED__ */


#ifndef __IDebugThreadCall_FWD_DEFINED__
#define __IDebugThreadCall_FWD_DEFINED__
typedef interface IDebugThreadCall IDebugThreadCall;
#endif 	/* __IDebugThreadCall_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplicationThread_FWD_DEFINED__
#define __IRemoteDebugApplicationThread_FWD_DEFINED__
typedef interface IRemoteDebugApplicationThread IRemoteDebugApplicationThread;
#endif 	/* __IRemoteDebugApplicationThread_FWD_DEFINED__ */


#ifndef __IDebugApplicationThread_FWD_DEFINED__
#define __IDebugApplicationThread_FWD_DEFINED__
typedef interface IDebugApplicationThread IDebugApplicationThread;
#endif 	/* __IDebugApplicationThread_FWD_DEFINED__ */


#ifndef __IDebugCookie_FWD_DEFINED__
#define __IDebugCookie_FWD_DEFINED__
typedef interface IDebugCookie IDebugCookie;
#endif 	/* __IDebugCookie_FWD_DEFINED__ */


#ifndef __IEnumDebugApplicationNodes_FWD_DEFINED__
#define __IEnumDebugApplicationNodes_FWD_DEFINED__
typedef interface IEnumDebugApplicationNodes IEnumDebugApplicationNodes;
#endif 	/* __IEnumDebugApplicationNodes_FWD_DEFINED__ */


#ifndef __IEnumRemoteDebugApplications_FWD_DEFINED__
#define __IEnumRemoteDebugApplications_FWD_DEFINED__
typedef interface IEnumRemoteDebugApplications IEnumRemoteDebugApplications;
#endif 	/* __IEnumRemoteDebugApplications_FWD_DEFINED__ */


#ifndef __IEnumRemoteDebugApplicationThreads_FWD_DEFINED__
#define __IEnumRemoteDebugApplicationThreads_FWD_DEFINED__
typedef interface IEnumRemoteDebugApplicationThreads IEnumRemoteDebugApplicationThreads;
#endif 	/* __IEnumRemoteDebugApplicationThreads_FWD_DEFINED__ */


#ifndef __IDebugFormatter_FWD_DEFINED__
#define __IDebugFormatter_FWD_DEFINED__
typedef interface IDebugFormatter IDebugFormatter;
#endif 	/* __IDebugFormatter_FWD_DEFINED__ */


#ifndef __ISimpleConnectionPoint_FWD_DEFINED__
#define __ISimpleConnectionPoint_FWD_DEFINED__
typedef interface ISimpleConnectionPoint ISimpleConnectionPoint;
#endif 	/* __ISimpleConnectionPoint_FWD_DEFINED__ */


#ifndef __IDebugHelper_FWD_DEFINED__
#define __IDebugHelper_FWD_DEFINED__
typedef interface IDebugHelper IDebugHelper;
#endif 	/* __IDebugHelper_FWD_DEFINED__ */


#ifndef __IEnumDebugExpressionContexts_FWD_DEFINED__
#define __IEnumDebugExpressionContexts_FWD_DEFINED__
typedef interface IEnumDebugExpressionContexts IEnumDebugExpressionContexts;
#endif 	/* __IEnumDebugExpressionContexts_FWD_DEFINED__ */


#ifndef __IProvideExpressionContexts_FWD_DEFINED__
#define __IProvideExpressionContexts_FWD_DEFINED__
typedef interface IProvideExpressionContexts IProvideExpressionContexts;
#endif 	/* __IProvideExpressionContexts_FWD_DEFINED__ */


#ifndef __IActiveScriptDebug_FWD_DEFINED__
#define __IActiveScriptDebug_FWD_DEFINED__
typedef interface IActiveScriptDebug IActiveScriptDebug;
#endif 	/* __IActiveScriptDebug_FWD_DEFINED__ */


#ifndef __IActiveScriptErrorDebug_FWD_DEFINED__
#define __IActiveScriptErrorDebug_FWD_DEFINED__
typedef interface IActiveScriptErrorDebug IActiveScriptErrorDebug;
#endif 	/* __IActiveScriptErrorDebug_FWD_DEFINED__ */


#ifndef __IActiveScriptSiteDebug_FWD_DEFINED__
#define __IActiveScriptSiteDebug_FWD_DEFINED__
typedef interface IActiveScriptSiteDebug IActiveScriptSiteDebug;
#endif 	/* __IActiveScriptSiteDebug_FWD_DEFINED__ */


#ifndef __IApplicationDebugger_FWD_DEFINED__
#define __IApplicationDebugger_FWD_DEFINED__
typedef interface IApplicationDebugger IApplicationDebugger;
#endif 	/* __IApplicationDebugger_FWD_DEFINED__ */


#ifndef __IApplicationDebuggerUI_FWD_DEFINED__
#define __IApplicationDebuggerUI_FWD_DEFINED__
typedef interface IApplicationDebuggerUI IApplicationDebuggerUI;
#endif 	/* __IApplicationDebuggerUI_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplication_FWD_DEFINED__
#define __IRemoteDebugApplication_FWD_DEFINED__
typedef interface IRemoteDebugApplication IRemoteDebugApplication;
#endif 	/* __IRemoteDebugApplication_FWD_DEFINED__ */


#ifndef __IDebugApplication_FWD_DEFINED__
#define __IDebugApplication_FWD_DEFINED__
typedef interface IDebugApplication IDebugApplication;
#endif 	/* __IDebugApplication_FWD_DEFINED__ */


#ifndef __IDebugDocumentInfo_FWD_DEFINED__
#define __IDebugDocumentInfo_FWD_DEFINED__
typedef interface IDebugDocumentInfo IDebugDocumentInfo;
#endif 	/* __IDebugDocumentInfo_FWD_DEFINED__ */


#ifndef __IDebugDocumentProvider_FWD_DEFINED__
#define __IDebugDocumentProvider_FWD_DEFINED__
typedef interface IDebugDocumentProvider IDebugDocumentProvider;
#endif 	/* __IDebugDocumentProvider_FWD_DEFINED__ */


#ifndef __IDebugApplicationNode_FWD_DEFINED__
#define __IDebugApplicationNode_FWD_DEFINED__
typedef interface IDebugApplicationNode IDebugApplicationNode;
#endif 	/* __IDebugApplicationNode_FWD_DEFINED__ */


#ifndef __IDebugApplicationNodeEvents_FWD_DEFINED__
#define __IDebugApplicationNodeEvents_FWD_DEFINED__
typedef interface IDebugApplicationNodeEvents IDebugApplicationNodeEvents;
#endif 	/* __IDebugApplicationNodeEvents_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplicationThread_FWD_DEFINED__
#define __IRemoteDebugApplicationThread_FWD_DEFINED__
typedef interface IRemoteDebugApplicationThread IRemoteDebugApplicationThread;
#endif 	/* __IRemoteDebugApplicationThread_FWD_DEFINED__ */


#ifndef __IDebugApplicationThread_FWD_DEFINED__
#define __IDebugApplicationThread_FWD_DEFINED__
typedef interface IDebugApplicationThread IDebugApplicationThread;
#endif 	/* __IDebugApplicationThread_FWD_DEFINED__ */


#ifndef __IDebugAsyncOperation_FWD_DEFINED__
#define __IDebugAsyncOperation_FWD_DEFINED__
typedef interface IDebugAsyncOperation IDebugAsyncOperation;
#endif 	/* __IDebugAsyncOperation_FWD_DEFINED__ */


#ifndef __IDebugAsyncOperationCallBack_FWD_DEFINED__
#define __IDebugAsyncOperationCallBack_FWD_DEFINED__
typedef interface IDebugAsyncOperationCallBack IDebugAsyncOperationCallBack;
#endif 	/* __IDebugAsyncOperationCallBack_FWD_DEFINED__ */


#ifndef __IDebugCodeContext_FWD_DEFINED__
#define __IDebugCodeContext_FWD_DEFINED__
typedef interface IDebugCodeContext IDebugCodeContext;
#endif 	/* __IDebugCodeContext_FWD_DEFINED__ */


#ifndef __IDebugCookie_FWD_DEFINED__
#define __IDebugCookie_FWD_DEFINED__
typedef interface IDebugCookie IDebugCookie;
#endif 	/* __IDebugCookie_FWD_DEFINED__ */


#ifndef __IDebugDocument_FWD_DEFINED__
#define __IDebugDocument_FWD_DEFINED__
typedef interface IDebugDocument IDebugDocument;
#endif 	/* __IDebugDocument_FWD_DEFINED__ */


#ifndef __IDebugDocumentContext_FWD_DEFINED__
#define __IDebugDocumentContext_FWD_DEFINED__
typedef interface IDebugDocumentContext IDebugDocumentContext;
#endif 	/* __IDebugDocumentContext_FWD_DEFINED__ */


#ifndef __IDebugDocumentHelper_FWD_DEFINED__
#define __IDebugDocumentHelper_FWD_DEFINED__
typedef interface IDebugDocumentHelper IDebugDocumentHelper;
#endif 	/* __IDebugDocumentHelper_FWD_DEFINED__ */


#ifndef __IDebugDocumentHost_FWD_DEFINED__
#define __IDebugDocumentHost_FWD_DEFINED__
typedef interface IDebugDocumentHost IDebugDocumentHost;
#endif 	/* __IDebugDocumentHost_FWD_DEFINED__ */


#ifndef __IDebugDocumentText_FWD_DEFINED__
#define __IDebugDocumentText_FWD_DEFINED__
typedef interface IDebugDocumentText IDebugDocumentText;
#endif 	/* __IDebugDocumentText_FWD_DEFINED__ */


#ifndef __IDebugDocumentTextAuthor_FWD_DEFINED__
#define __IDebugDocumentTextAuthor_FWD_DEFINED__
typedef interface IDebugDocumentTextAuthor IDebugDocumentTextAuthor;
#endif 	/* __IDebugDocumentTextAuthor_FWD_DEFINED__ */


#ifndef __IDebugDocumentTextEvents_FWD_DEFINED__
#define __IDebugDocumentTextEvents_FWD_DEFINED__
typedef interface IDebugDocumentTextEvents IDebugDocumentTextEvents;
#endif 	/* __IDebugDocumentTextEvents_FWD_DEFINED__ */


#ifndef __IDebugDocumentTextExternalAuthor_FWD_DEFINED__
#define __IDebugDocumentTextExternalAuthor_FWD_DEFINED__
typedef interface IDebugDocumentTextExternalAuthor IDebugDocumentTextExternalAuthor;
#endif 	/* __IDebugDocumentTextExternalAuthor_FWD_DEFINED__ */


#ifndef __IDebugExpression_FWD_DEFINED__
#define __IDebugExpression_FWD_DEFINED__
typedef interface IDebugExpression IDebugExpression;
#endif 	/* __IDebugExpression_FWD_DEFINED__ */


#ifndef __IDebugExpressionCallBack_FWD_DEFINED__
#define __IDebugExpressionCallBack_FWD_DEFINED__
typedef interface IDebugExpressionCallBack IDebugExpressionCallBack;
#endif 	/* __IDebugExpressionCallBack_FWD_DEFINED__ */


#ifndef __IDebugExpressionContext_FWD_DEFINED__
#define __IDebugExpressionContext_FWD_DEFINED__
typedef interface IDebugExpressionContext IDebugExpressionContext;
#endif 	/* __IDebugExpressionContext_FWD_DEFINED__ */


#ifndef __IDebugFormatter_FWD_DEFINED__
#define __IDebugFormatter_FWD_DEFINED__
typedef interface IDebugFormatter IDebugFormatter;
#endif 	/* __IDebugFormatter_FWD_DEFINED__ */


#ifndef __IDebugHelper_FWD_DEFINED__
#define __IDebugHelper_FWD_DEFINED__
typedef interface IDebugHelper IDebugHelper;
#endif 	/* __IDebugHelper_FWD_DEFINED__ */


#ifndef __IDebugSessionProvider_FWD_DEFINED__
#define __IDebugSessionProvider_FWD_DEFINED__
typedef interface IDebugSessionProvider IDebugSessionProvider;
#endif 	/* __IDebugSessionProvider_FWD_DEFINED__ */


#ifndef __IDebugStackFrame_FWD_DEFINED__
#define __IDebugStackFrame_FWD_DEFINED__
typedef interface IDebugStackFrame IDebugStackFrame;
#endif 	/* __IDebugStackFrame_FWD_DEFINED__ */


#ifndef __IDebugStackFrameSniffer_FWD_DEFINED__
#define __IDebugStackFrameSniffer_FWD_DEFINED__
typedef interface IDebugStackFrameSniffer IDebugStackFrameSniffer;
#endif 	/* __IDebugStackFrameSniffer_FWD_DEFINED__ */


#ifndef __IDebugStackFrameSnifferEx_FWD_DEFINED__
#define __IDebugStackFrameSnifferEx_FWD_DEFINED__
typedef interface IDebugStackFrameSnifferEx IDebugStackFrameSnifferEx;
#endif 	/* __IDebugStackFrameSnifferEx_FWD_DEFINED__ */


#ifndef __IDebugSyncOperation_FWD_DEFINED__
#define __IDebugSyncOperation_FWD_DEFINED__
typedef interface IDebugSyncOperation IDebugSyncOperation;
#endif 	/* __IDebugSyncOperation_FWD_DEFINED__ */


#ifndef __IDebugThreadCall_FWD_DEFINED__
#define __IDebugThreadCall_FWD_DEFINED__
typedef interface IDebugThreadCall IDebugThreadCall;
#endif 	/* __IDebugThreadCall_FWD_DEFINED__ */


#ifndef __IEnumDebugApplicationNodes_FWD_DEFINED__
#define __IEnumDebugApplicationNodes_FWD_DEFINED__
typedef interface IEnumDebugApplicationNodes IEnumDebugApplicationNodes;
#endif 	/* __IEnumDebugApplicationNodes_FWD_DEFINED__ */


#ifndef __IEnumDebugCodeContexts_FWD_DEFINED__
#define __IEnumDebugCodeContexts_FWD_DEFINED__
typedef interface IEnumDebugCodeContexts IEnumDebugCodeContexts;
#endif 	/* __IEnumDebugCodeContexts_FWD_DEFINED__ */


#ifndef __IEnumDebugExpressionContexts_FWD_DEFINED__
#define __IEnumDebugExpressionContexts_FWD_DEFINED__
typedef interface IEnumDebugExpressionContexts IEnumDebugExpressionContexts;
#endif 	/* __IEnumDebugExpressionContexts_FWD_DEFINED__ */


#ifndef __IEnumDebugStackFrames_FWD_DEFINED__
#define __IEnumDebugStackFrames_FWD_DEFINED__
typedef interface IEnumDebugStackFrames IEnumDebugStackFrames;
#endif 	/* __IEnumDebugStackFrames_FWD_DEFINED__ */


#ifndef __IEnumRemoteDebugApplications_FWD_DEFINED__
#define __IEnumRemoteDebugApplications_FWD_DEFINED__
typedef interface IEnumRemoteDebugApplications IEnumRemoteDebugApplications;
#endif 	/* __IEnumRemoteDebugApplications_FWD_DEFINED__ */


#ifndef __IEnumRemoteDebugApplicationThreads_FWD_DEFINED__
#define __IEnumRemoteDebugApplicationThreads_FWD_DEFINED__
typedef interface IEnumRemoteDebugApplicationThreads IEnumRemoteDebugApplicationThreads;
#endif 	/* __IEnumRemoteDebugApplicationThreads_FWD_DEFINED__ */


#ifndef __IProcessDebugManager_FWD_DEFINED__
#define __IProcessDebugManager_FWD_DEFINED__
typedef interface IProcessDebugManager IProcessDebugManager;
#endif 	/* __IProcessDebugManager_FWD_DEFINED__ */


#ifndef __IProvideExpressionContexts_FWD_DEFINED__
#define __IProvideExpressionContexts_FWD_DEFINED__
typedef interface IProvideExpressionContexts IProvideExpressionContexts;
#endif 	/* __IProvideExpressionContexts_FWD_DEFINED__ */


#ifndef __IMachineDebugManager_FWD_DEFINED__
#define __IMachineDebugManager_FWD_DEFINED__
typedef interface IMachineDebugManager IMachineDebugManager;
#endif 	/* __IMachineDebugManager_FWD_DEFINED__ */


#ifndef __IMachineDebugManagerCookie_FWD_DEFINED__
#define __IMachineDebugManagerCookie_FWD_DEFINED__
typedef interface IMachineDebugManagerCookie IMachineDebugManagerCookie;
#endif 	/* __IMachineDebugManagerCookie_FWD_DEFINED__ */


#ifndef __IMachineDebugManagerEvents_FWD_DEFINED__
#define __IMachineDebugManagerEvents_FWD_DEFINED__
typedef interface IMachineDebugManagerEvents IMachineDebugManagerEvents;
#endif 	/* __IMachineDebugManagerEvents_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplicationEvents_FWD_DEFINED__
#define __IRemoteDebugApplicationEvents_FWD_DEFINED__
typedef interface IRemoteDebugApplicationEvents IRemoteDebugApplicationEvents;
#endif 	/* __IRemoteDebugApplicationEvents_FWD_DEFINED__ */


#ifndef __ISimpleConnectionPoint_FWD_DEFINED__
#define __ISimpleConnectionPoint_FWD_DEFINED__
typedef interface ISimpleConnectionPoint ISimpleConnectionPoint;
#endif 	/* __ISimpleConnectionPoint_FWD_DEFINED__ */


#ifndef __ProcessDebugManager_FWD_DEFINED__
#define __ProcessDebugManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class ProcessDebugManager ProcessDebugManager;
#else
typedef struct ProcessDebugManager ProcessDebugManager;
#endif /* __cplusplus */

#endif 	/* __ProcessDebugManager_FWD_DEFINED__ */


#ifndef __DebugHelper_FWD_DEFINED__
#define __DebugHelper_FWD_DEFINED__

#ifdef __cplusplus
typedef class DebugHelper DebugHelper;
#else
typedef struct DebugHelper DebugHelper;
#endif /* __cplusplus */

#endif 	/* __DebugHelper_FWD_DEFINED__ */


#ifndef __CDebugDocumentHelper_FWD_DEFINED__
#define __CDebugDocumentHelper_FWD_DEFINED__

#ifdef __cplusplus
typedef class CDebugDocumentHelper CDebugDocumentHelper;
#else
typedef struct CDebugDocumentHelper CDebugDocumentHelper;
#endif /* __cplusplus */

#endif 	/* __CDebugDocumentHelper_FWD_DEFINED__ */


#ifndef __MachineDebugManager_FWD_DEFINED__
#define __MachineDebugManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class MachineDebugManager MachineDebugManager;
#else
typedef struct MachineDebugManager MachineDebugManager;
#endif /* __cplusplus */

#endif 	/* __MachineDebugManager_FWD_DEFINED__ */


#ifndef __DefaultDebugSessionProvider_FWD_DEFINED__
#define __DefaultDebugSessionProvider_FWD_DEFINED__

#ifdef __cplusplus
typedef class DefaultDebugSessionProvider DefaultDebugSessionProvider;
#else
typedef struct DefaultDebugSessionProvider DefaultDebugSessionProvider;
#endif /* __cplusplus */

#endif 	/* __DefaultDebugSessionProvider_FWD_DEFINED__ */


/* header files for imported files */
#include "ocidl.h"
#include "activscp.h"
#include "dbgprop.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL_itf_activdbg_0000
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local] */ 


//=--------------------------------------------------------------------------=
// ActivDbg.h
//=--------------------------------------------------------------------------=
// (C) Copyright 1996 Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
#pragma comment(lib,"uuid.lib")
//
// Declarations for ActiveX Script authoring/Debugging.
//


















































typedef 
enum tagBREAKPOINT_STATE
    {	BREAKPOINT_DELETED	= 0,
	BREAKPOINT_DISABLED	= 1,
	BREAKPOINT_ENABLED	= 2
    }	BREAKPOINT_STATE;

typedef DWORD APPBREAKFLAGS;

#define	APPBREAKFLAG_DEBUGGER_BLOCK	( 0x1 )

#define	APPBREAKFLAG_DEBUGGER_HALT	( 0x2 )

#define	APPBREAKFLAG_STEP	( 0x10000 )

#define	APPBREAKFLAG_NESTED	( 0x20000 )

#define	APPBREAKFLAG_STEPTYPE_SOURCE	( 0 )

#define	APPBREAKFLAG_STEPTYPE_BYTECODE	( 0x100000 )

#define	APPBREAKFLAG_STEPTYPE_MACHINE	( 0x200000 )

#define	APPBREAKFLAG_STEPTYPE_MASK	( 0xf00000 )

#define	APPBREAKFLAG_IN_BREAKPOINT	( 0x80000000 )

typedef 
enum tagBREAKREASON
    {	BREAKREASON_STEP	= 0,
	BREAKREASON_BREAKPOINT	= BREAKREASON_STEP + 1,
	BREAKREASON_DEBUGGER_BLOCK	= BREAKREASON_BREAKPOINT + 1,
	BREAKREASON_HOST_INITIATED	= BREAKREASON_DEBUGGER_BLOCK + 1,
	BREAKREASON_LANGUAGE_INITIATED	= BREAKREASON_HOST_INITIATED + 1,
	BREAKREASON_DEBUGGER_HALT	= BREAKREASON_LANGUAGE_INITIATED + 1,
	BREAKREASON_ERROR	= BREAKREASON_DEBUGGER_HALT + 1,
	BREAKREASON_JIT	= BREAKREASON_ERROR + 1
    }	BREAKREASON;

typedef 
enum tagBREAKRESUME_ACTION
    {	BREAKRESUMEACTION_ABORT	= 0,
	BREAKRESUMEACTION_CONTINUE	= BREAKRESUMEACTION_ABORT + 1,
	BREAKRESUMEACTION_STEP_INTO	= BREAKRESUMEACTION_CONTINUE + 1,
	BREAKRESUMEACTION_STEP_OVER	= BREAKRESUMEACTION_STEP_INTO + 1,
	BREAKRESUMEACTION_STEP_OUT	= BREAKRESUMEACTION_STEP_OVER + 1,
	BREAKRESUMEACTION_IGNORE	= BREAKRESUMEACTION_STEP_OUT + 1
    }	BREAKRESUMEACTION;

typedef 
enum tagERRORRESUMEACTION
    {	ERRORRESUMEACTION_ReexecuteErrorStatement	= 0,
	ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller	= ERRORRESUMEACTION_ReexecuteErrorStatement + 1,
	ERRORRESUMEACTION_SkipErrorStatement	= ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller + 1
    }	ERRORRESUMEACTION;

typedef 
enum tagDOCUMENTNAMETYPE
    {	DOCUMENTNAMETYPE_APPNODE	= 0,
	DOCUMENTNAMETYPE_TITLE	= DOCUMENTNAMETYPE_APPNODE + 1,
	DOCUMENTNAMETYPE_FILE_TAIL	= DOCUMENTNAMETYPE_TITLE + 1,
	DOCUMENTNAMETYPE_URL	= DOCUMENTNAMETYPE_FILE_TAIL + 1
    }	DOCUMENTNAMETYPE;

typedef WORD SOURCE_TEXT_ATTR;

#define	SOURCETEXT_ATTR_KEYWORD	( 0x1 )

#define	SOURCETEXT_ATTR_COMMENT	( 0x2 )

#define	SOURCETEXT_ATTR_NONSOURCE	( 0x4 )

#define	SOURCETEXT_ATTR_OPERATOR	( 0x8 )

#define	SOURCETEXT_ATTR_NUMBER	( 0x10 )

#define	SOURCETEXT_ATTR_STRING	( 0x20 )

#define	SOURCETEXT_ATTR_FUNCTION_START	( 0x40 )

typedef DWORD TEXT_DOC_ATTR;

#define	TEXT_DOC_ATTR_READONLY	( 0x1 )

#define	DEBUG_TEXT_ISEXPRESSION	( 0x1 )

#define	DEBUG_TEXT_RETURNVALUE	( 0x2 )

#define	DEBUG_TEXT_NOSIDEEFFECTS	( 0x4 )

#define	DEBUG_TEXT_ALLOWBREAKPOINTS	( 0x8 )

#define	DEBUG_TEXT_ALLOWERRORREPORT	( 0x10 )

#define	DEBUG_TEXT_EVALUATETOCODECONTEXT	( 0x20 )



extern RPC_IF_HANDLE __MIDL_itf_activdbg_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0000_v0_0_s_ifspec;

#ifndef __IActiveScriptDebug_INTERFACE_DEFINED__
#define __IActiveScriptDebug_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IActiveScriptDebug
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IActiveScriptDebug;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C10-CB0C-11d0-B5C9-00A0244A0E7A")
    IActiveScriptDebug : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetScriptTextAttributes( 
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pattr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScriptletTextAttributes( 
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pattr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCodeContextsOfPosition( 
            /* [in] */ DWORD dwSourceContext,
            /* [in] */ ULONG uCharacterOffset,
            /* [in] */ ULONG uNumChars,
            /* [out] */ IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IActiveScriptDebugVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IActiveScriptDebug __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IActiveScriptDebug __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IActiveScriptDebug __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScriptTextAttributes )( 
            IActiveScriptDebug __RPC_FAR * This,
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pattr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScriptletTextAttributes )( 
            IActiveScriptDebug __RPC_FAR * This,
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pattr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumCodeContextsOfPosition )( 
            IActiveScriptDebug __RPC_FAR * This,
            /* [in] */ DWORD dwSourceContext,
            /* [in] */ ULONG uCharacterOffset,
            /* [in] */ ULONG uNumChars,
            /* [out] */ IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc);
        
        END_INTERFACE
    } IActiveScriptDebugVtbl;

    interface IActiveScriptDebug
    {
        CONST_VTBL struct IActiveScriptDebugVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveScriptDebug_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveScriptDebug_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveScriptDebug_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveScriptDebug_GetScriptTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)	\
    (This)->lpVtbl -> GetScriptTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)

#define IActiveScriptDebug_GetScriptletTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)	\
    (This)->lpVtbl -> GetScriptletTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)

#define IActiveScriptDebug_EnumCodeContextsOfPosition(This,dwSourceContext,uCharacterOffset,uNumChars,ppescc)	\
    (This)->lpVtbl -> EnumCodeContextsOfPosition(This,dwSourceContext,uCharacterOffset,uNumChars,ppescc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IActiveScriptDebug_GetScriptTextAttributes_Proxy( 
    IActiveScriptDebug __RPC_FAR * This,
    /* [size_is][in] */ LPCOLESTR pstrCode,
    /* [in] */ ULONG uNumCodeChars,
    /* [in] */ LPCOLESTR pstrDelimiter,
    /* [in] */ DWORD dwFlags,
    /* [size_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pattr);


void __RPC_STUB IActiveScriptDebug_GetScriptTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptDebug_GetScriptletTextAttributes_Proxy( 
    IActiveScriptDebug __RPC_FAR * This,
    /* [size_is][in] */ LPCOLESTR pstrCode,
    /* [in] */ ULONG uNumCodeChars,
    /* [in] */ LPCOLESTR pstrDelimiter,
    /* [in] */ DWORD dwFlags,
    /* [size_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pattr);


void __RPC_STUB IActiveScriptDebug_GetScriptletTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptDebug_EnumCodeContextsOfPosition_Proxy( 
    IActiveScriptDebug __RPC_FAR * This,
    /* [in] */ DWORD dwSourceContext,
    /* [in] */ ULONG uCharacterOffset,
    /* [in] */ ULONG uNumChars,
    /* [out] */ IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc);


void __RPC_STUB IActiveScriptDebug_EnumCodeContextsOfPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IActiveScriptDebug_INTERFACE_DEFINED__ */


#ifndef __IActiveScriptSiteDebug_INTERFACE_DEFINED__
#define __IActiveScriptSiteDebug_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IActiveScriptSiteDebug
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][uuid][object] */ 



EXTERN_C const IID IID_IActiveScriptSiteDebug;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C11-CB0C-11d0-B5C9-00A0244A0E7A")
    IActiveScriptSiteDebug : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocumentContextFromPosition( 
            /* [in] */ DWORD dwSourceContext,
            /* [in] */ ULONG uCharacterOffset,
            /* [in] */ ULONG uNumChars,
            /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetApplication( 
            /* [out] */ IDebugApplication __RPC_FAR *__RPC_FAR *ppda) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRootApplicationNode( 
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdanRoot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnScriptErrorDebug( 
            /* [in] */ IActiveScriptErrorDebug __RPC_FAR *pErrorDebug,
            /* [out] */ BOOL __RPC_FAR *pfEnterDebugger,
            /* [out] */ BOOL __RPC_FAR *pfCallOnScriptErrorWhenContinuing) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IActiveScriptSiteDebugVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IActiveScriptSiteDebug __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IActiveScriptSiteDebug __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IActiveScriptSiteDebug __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocumentContextFromPosition )( 
            IActiveScriptSiteDebug __RPC_FAR * This,
            /* [in] */ DWORD dwSourceContext,
            /* [in] */ ULONG uCharacterOffset,
            /* [in] */ ULONG uNumChars,
            /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetApplication )( 
            IActiveScriptSiteDebug __RPC_FAR * This,
            /* [out] */ IDebugApplication __RPC_FAR *__RPC_FAR *ppda);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRootApplicationNode )( 
            IActiveScriptSiteDebug __RPC_FAR * This,
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdanRoot);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnScriptErrorDebug )( 
            IActiveScriptSiteDebug __RPC_FAR * This,
            /* [in] */ IActiveScriptErrorDebug __RPC_FAR *pErrorDebug,
            /* [out] */ BOOL __RPC_FAR *pfEnterDebugger,
            /* [out] */ BOOL __RPC_FAR *pfCallOnScriptErrorWhenContinuing);
        
        END_INTERFACE
    } IActiveScriptSiteDebugVtbl;

    interface IActiveScriptSiteDebug
    {
        CONST_VTBL struct IActiveScriptSiteDebugVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveScriptSiteDebug_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveScriptSiteDebug_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveScriptSiteDebug_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveScriptSiteDebug_GetDocumentContextFromPosition(This,dwSourceContext,uCharacterOffset,uNumChars,ppsc)	\
    (This)->lpVtbl -> GetDocumentContextFromPosition(This,dwSourceContext,uCharacterOffset,uNumChars,ppsc)

#define IActiveScriptSiteDebug_GetApplication(This,ppda)	\
    (This)->lpVtbl -> GetApplication(This,ppda)

#define IActiveScriptSiteDebug_GetRootApplicationNode(This,ppdanRoot)	\
    (This)->lpVtbl -> GetRootApplicationNode(This,ppdanRoot)

#define IActiveScriptSiteDebug_OnScriptErrorDebug(This,pErrorDebug,pfEnterDebugger,pfCallOnScriptErrorWhenContinuing)	\
    (This)->lpVtbl -> OnScriptErrorDebug(This,pErrorDebug,pfEnterDebugger,pfCallOnScriptErrorWhenContinuing)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IActiveScriptSiteDebug_GetDocumentContextFromPosition_Proxy( 
    IActiveScriptSiteDebug __RPC_FAR * This,
    /* [in] */ DWORD dwSourceContext,
    /* [in] */ ULONG uCharacterOffset,
    /* [in] */ ULONG uNumChars,
    /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc);


void __RPC_STUB IActiveScriptSiteDebug_GetDocumentContextFromPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptSiteDebug_GetApplication_Proxy( 
    IActiveScriptSiteDebug __RPC_FAR * This,
    /* [out] */ IDebugApplication __RPC_FAR *__RPC_FAR *ppda);


void __RPC_STUB IActiveScriptSiteDebug_GetApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptSiteDebug_GetRootApplicationNode_Proxy( 
    IActiveScriptSiteDebug __RPC_FAR * This,
    /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdanRoot);


void __RPC_STUB IActiveScriptSiteDebug_GetRootApplicationNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptSiteDebug_OnScriptErrorDebug_Proxy( 
    IActiveScriptSiteDebug __RPC_FAR * This,
    /* [in] */ IActiveScriptErrorDebug __RPC_FAR *pErrorDebug,
    /* [out] */ BOOL __RPC_FAR *pfEnterDebugger,
    /* [out] */ BOOL __RPC_FAR *pfCallOnScriptErrorWhenContinuing);


void __RPC_STUB IActiveScriptSiteDebug_OnScriptErrorDebug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IActiveScriptSiteDebug_INTERFACE_DEFINED__ */


#ifndef __IActiveScriptErrorDebug_INTERFACE_DEFINED__
#define __IActiveScriptErrorDebug_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IActiveScriptErrorDebug
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IActiveScriptErrorDebug;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C12-CB0C-11d0-B5C9-00A0244A0E7A")
    IActiveScriptErrorDebug : public IActiveScriptError
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocumentContext( 
            /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppssc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStackFrame( 
            /* [out] */ IDebugStackFrame __RPC_FAR *__RPC_FAR *ppdsf) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IActiveScriptErrorDebugVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IActiveScriptErrorDebug __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IActiveScriptErrorDebug __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IActiveScriptErrorDebug __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetExceptionInfo )( 
            IActiveScriptErrorDebug __RPC_FAR * This,
            /* [out] */ EXCEPINFO __RPC_FAR *pexcepinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSourcePosition )( 
            IActiveScriptErrorDebug __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwSourceContext,
            /* [out] */ ULONG __RPC_FAR *pulLineNumber,
            /* [out] */ LONG __RPC_FAR *plCharacterPosition);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSourceLineText )( 
            IActiveScriptErrorDebug __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrSourceLine);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocumentContext )( 
            IActiveScriptErrorDebug __RPC_FAR * This,
            /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppssc);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStackFrame )( 
            IActiveScriptErrorDebug __RPC_FAR * This,
            /* [out] */ IDebugStackFrame __RPC_FAR *__RPC_FAR *ppdsf);
        
        END_INTERFACE
    } IActiveScriptErrorDebugVtbl;

    interface IActiveScriptErrorDebug
    {
        CONST_VTBL struct IActiveScriptErrorDebugVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveScriptErrorDebug_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveScriptErrorDebug_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveScriptErrorDebug_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveScriptErrorDebug_GetExceptionInfo(This,pexcepinfo)	\
    (This)->lpVtbl -> GetExceptionInfo(This,pexcepinfo)

#define IActiveScriptErrorDebug_GetSourcePosition(This,pdwSourceContext,pulLineNumber,plCharacterPosition)	\
    (This)->lpVtbl -> GetSourcePosition(This,pdwSourceContext,pulLineNumber,plCharacterPosition)

#define IActiveScriptErrorDebug_GetSourceLineText(This,pbstrSourceLine)	\
    (This)->lpVtbl -> GetSourceLineText(This,pbstrSourceLine)


#define IActiveScriptErrorDebug_GetDocumentContext(This,ppssc)	\
    (This)->lpVtbl -> GetDocumentContext(This,ppssc)

#define IActiveScriptErrorDebug_GetStackFrame(This,ppdsf)	\
    (This)->lpVtbl -> GetStackFrame(This,ppdsf)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IActiveScriptErrorDebug_GetDocumentContext_Proxy( 
    IActiveScriptErrorDebug __RPC_FAR * This,
    /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppssc);


void __RPC_STUB IActiveScriptErrorDebug_GetDocumentContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptErrorDebug_GetStackFrame_Proxy( 
    IActiveScriptErrorDebug __RPC_FAR * This,
    /* [out] */ IDebugStackFrame __RPC_FAR *__RPC_FAR *ppdsf);


void __RPC_STUB IActiveScriptErrorDebug_GetStackFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IActiveScriptErrorDebug_INTERFACE_DEFINED__ */


#ifndef __IDebugCodeContext_INTERFACE_DEFINED__
#define __IDebugCodeContext_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugCodeContext
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugCodeContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C13-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugCodeContext : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocumentContext( 
            /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBreakPoint( 
            /* [in] */ BREAKPOINT_STATE bps) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugCodeContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugCodeContext __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugCodeContext __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugCodeContext __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocumentContext )( 
            IDebugCodeContext __RPC_FAR * This,
            /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetBreakPoint )( 
            IDebugCodeContext __RPC_FAR * This,
            /* [in] */ BREAKPOINT_STATE bps);
        
        END_INTERFACE
    } IDebugCodeContextVtbl;

    interface IDebugCodeContext
    {
        CONST_VTBL struct IDebugCodeContextVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugCodeContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugCodeContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugCodeContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugCodeContext_GetDocumentContext(This,ppsc)	\
    (This)->lpVtbl -> GetDocumentContext(This,ppsc)

#define IDebugCodeContext_SetBreakPoint(This,bps)	\
    (This)->lpVtbl -> SetBreakPoint(This,bps)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugCodeContext_GetDocumentContext_Proxy( 
    IDebugCodeContext __RPC_FAR * This,
    /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc);


void __RPC_STUB IDebugCodeContext_GetDocumentContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugCodeContext_SetBreakPoint_Proxy( 
    IDebugCodeContext __RPC_FAR * This,
    /* [in] */ BREAKPOINT_STATE bps);


void __RPC_STUB IDebugCodeContext_SetBreakPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugCodeContext_INTERFACE_DEFINED__ */


#ifndef __IDebugExpression_INTERFACE_DEFINED__
#define __IDebugExpression_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugExpression
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugExpression;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C14-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugExpression : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Start( 
            /* [in] */ IDebugExpressionCallBack __RPC_FAR *pdecb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Abort( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryIsComplete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetResultAsString( 
            /* [out] */ HRESULT __RPC_FAR *phrResult,
            /* [out] */ BSTR __RPC_FAR *pbstrResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetResultAsDebugProperty( 
            /* [out] */ HRESULT __RPC_FAR *phrResult,
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugExpressionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugExpression __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugExpression __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugExpression __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Start )( 
            IDebugExpression __RPC_FAR * This,
            /* [in] */ IDebugExpressionCallBack __RPC_FAR *pdecb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Abort )( 
            IDebugExpression __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryIsComplete )( 
            IDebugExpression __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetResultAsString )( 
            IDebugExpression __RPC_FAR * This,
            /* [out] */ HRESULT __RPC_FAR *phrResult,
            /* [out] */ BSTR __RPC_FAR *pbstrResult);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetResultAsDebugProperty )( 
            IDebugExpression __RPC_FAR * This,
            /* [out] */ HRESULT __RPC_FAR *phrResult,
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdp);
        
        END_INTERFACE
    } IDebugExpressionVtbl;

    interface IDebugExpression
    {
        CONST_VTBL struct IDebugExpressionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugExpression_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugExpression_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugExpression_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugExpression_Start(This,pdecb)	\
    (This)->lpVtbl -> Start(This,pdecb)

#define IDebugExpression_Abort(This)	\
    (This)->lpVtbl -> Abort(This)

#define IDebugExpression_QueryIsComplete(This)	\
    (This)->lpVtbl -> QueryIsComplete(This)

#define IDebugExpression_GetResultAsString(This,phrResult,pbstrResult)	\
    (This)->lpVtbl -> GetResultAsString(This,phrResult,pbstrResult)

#define IDebugExpression_GetResultAsDebugProperty(This,phrResult,ppdp)	\
    (This)->lpVtbl -> GetResultAsDebugProperty(This,phrResult,ppdp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugExpression_Start_Proxy( 
    IDebugExpression __RPC_FAR * This,
    /* [in] */ IDebugExpressionCallBack __RPC_FAR *pdecb);


void __RPC_STUB IDebugExpression_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugExpression_Abort_Proxy( 
    IDebugExpression __RPC_FAR * This);


void __RPC_STUB IDebugExpression_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugExpression_QueryIsComplete_Proxy( 
    IDebugExpression __RPC_FAR * This);


void __RPC_STUB IDebugExpression_QueryIsComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugExpression_GetResultAsString_Proxy( 
    IDebugExpression __RPC_FAR * This,
    /* [out] */ HRESULT __RPC_FAR *phrResult,
    /* [out] */ BSTR __RPC_FAR *pbstrResult);


void __RPC_STUB IDebugExpression_GetResultAsString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugExpression_GetResultAsDebugProperty_Proxy( 
    IDebugExpression __RPC_FAR * This,
    /* [out] */ HRESULT __RPC_FAR *phrResult,
    /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdp);


void __RPC_STUB IDebugExpression_GetResultAsDebugProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugExpression_INTERFACE_DEFINED__ */


#ifndef __IDebugExpressionContext_INTERFACE_DEFINED__
#define __IDebugExpressionContext_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugExpressionContext
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IDebugExpressionContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C15-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugExpressionContext : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParseLanguageText( 
            /* [in] */ LPCOLESTR pstrCode,
            /* [in] */ UINT nRadix,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [out] */ IDebugExpression __RPC_FAR *__RPC_FAR *ppe) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLanguageInfo( 
            /* [out] */ BSTR __RPC_FAR *pbstrLanguageName,
            /* [out] */ GUID __RPC_FAR *pLanguageID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugExpressionContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugExpressionContext __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugExpressionContext __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugExpressionContext __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ParseLanguageText )( 
            IDebugExpressionContext __RPC_FAR * This,
            /* [in] */ LPCOLESTR pstrCode,
            /* [in] */ UINT nRadix,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [out] */ IDebugExpression __RPC_FAR *__RPC_FAR *ppe);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLanguageInfo )( 
            IDebugExpressionContext __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrLanguageName,
            /* [out] */ GUID __RPC_FAR *pLanguageID);
        
        END_INTERFACE
    } IDebugExpressionContextVtbl;

    interface IDebugExpressionContext
    {
        CONST_VTBL struct IDebugExpressionContextVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugExpressionContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugExpressionContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugExpressionContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugExpressionContext_ParseLanguageText(This,pstrCode,nRadix,pstrDelimiter,dwFlags,ppe)	\
    (This)->lpVtbl -> ParseLanguageText(This,pstrCode,nRadix,pstrDelimiter,dwFlags,ppe)

#define IDebugExpressionContext_GetLanguageInfo(This,pbstrLanguageName,pLanguageID)	\
    (This)->lpVtbl -> GetLanguageInfo(This,pbstrLanguageName,pLanguageID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugExpressionContext_ParseLanguageText_Proxy( 
    IDebugExpressionContext __RPC_FAR * This,
    /* [in] */ LPCOLESTR pstrCode,
    /* [in] */ UINT nRadix,
    /* [in] */ LPCOLESTR pstrDelimiter,
    /* [in] */ DWORD dwFlags,
    /* [out] */ IDebugExpression __RPC_FAR *__RPC_FAR *ppe);


void __RPC_STUB IDebugExpressionContext_ParseLanguageText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugExpressionContext_GetLanguageInfo_Proxy( 
    IDebugExpressionContext __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrLanguageName,
    /* [out] */ GUID __RPC_FAR *pLanguageID);


void __RPC_STUB IDebugExpressionContext_GetLanguageInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugExpressionContext_INTERFACE_DEFINED__ */


#ifndef __IDebugExpressionCallBack_INTERFACE_DEFINED__
#define __IDebugExpressionCallBack_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugExpressionCallBack
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugExpressionCallBack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C16-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugExpressionCallBack : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE onComplete( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugExpressionCallBackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugExpressionCallBack __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugExpressionCallBack __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugExpressionCallBack __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onComplete )( 
            IDebugExpressionCallBack __RPC_FAR * This);
        
        END_INTERFACE
    } IDebugExpressionCallBackVtbl;

    interface IDebugExpressionCallBack
    {
        CONST_VTBL struct IDebugExpressionCallBackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugExpressionCallBack_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugExpressionCallBack_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugExpressionCallBack_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugExpressionCallBack_onComplete(This)	\
    (This)->lpVtbl -> onComplete(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugExpressionCallBack_onComplete_Proxy( 
    IDebugExpressionCallBack __RPC_FAR * This);


void __RPC_STUB IDebugExpressionCallBack_onComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugExpressionCallBack_INTERFACE_DEFINED__ */


#ifndef __IDebugStackFrame_INTERFACE_DEFINED__
#define __IDebugStackFrame_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugStackFrame
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugStackFrame;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C17-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugStackFrame : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCodeContext( 
            /* [out] */ IDebugCodeContext __RPC_FAR *__RPC_FAR *ppcc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescriptionString( 
            /* [in] */ BOOL fLong,
            /* [out] */ BSTR __RPC_FAR *pbstrDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLanguageString( 
            /* [in] */ BOOL fLong,
            /* [out] */ BSTR __RPC_FAR *pbstrLanguage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetThread( 
            /* [out] */ IDebugApplicationThread __RPC_FAR *__RPC_FAR *ppat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDebugProperty( 
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppDebugProp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugStackFrameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugStackFrame __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugStackFrame __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugStackFrame __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCodeContext )( 
            IDebugStackFrame __RPC_FAR * This,
            /* [out] */ IDebugCodeContext __RPC_FAR *__RPC_FAR *ppcc);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDescriptionString )( 
            IDebugStackFrame __RPC_FAR * This,
            /* [in] */ BOOL fLong,
            /* [out] */ BSTR __RPC_FAR *pbstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLanguageString )( 
            IDebugStackFrame __RPC_FAR * This,
            /* [in] */ BOOL fLong,
            /* [out] */ BSTR __RPC_FAR *pbstrLanguage);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetThread )( 
            IDebugStackFrame __RPC_FAR * This,
            /* [out] */ IDebugApplicationThread __RPC_FAR *__RPC_FAR *ppat);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDebugProperty )( 
            IDebugStackFrame __RPC_FAR * This,
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppDebugProp);
        
        END_INTERFACE
    } IDebugStackFrameVtbl;

    interface IDebugStackFrame
    {
        CONST_VTBL struct IDebugStackFrameVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugStackFrame_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugStackFrame_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugStackFrame_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugStackFrame_GetCodeContext(This,ppcc)	\
    (This)->lpVtbl -> GetCodeContext(This,ppcc)

#define IDebugStackFrame_GetDescriptionString(This,fLong,pbstrDescription)	\
    (This)->lpVtbl -> GetDescriptionString(This,fLong,pbstrDescription)

#define IDebugStackFrame_GetLanguageString(This,fLong,pbstrLanguage)	\
    (This)->lpVtbl -> GetLanguageString(This,fLong,pbstrLanguage)

#define IDebugStackFrame_GetThread(This,ppat)	\
    (This)->lpVtbl -> GetThread(This,ppat)

#define IDebugStackFrame_GetDebugProperty(This,ppDebugProp)	\
    (This)->lpVtbl -> GetDebugProperty(This,ppDebugProp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugStackFrame_GetCodeContext_Proxy( 
    IDebugStackFrame __RPC_FAR * This,
    /* [out] */ IDebugCodeContext __RPC_FAR *__RPC_FAR *ppcc);


void __RPC_STUB IDebugStackFrame_GetCodeContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugStackFrame_GetDescriptionString_Proxy( 
    IDebugStackFrame __RPC_FAR * This,
    /* [in] */ BOOL fLong,
    /* [out] */ BSTR __RPC_FAR *pbstrDescription);


void __RPC_STUB IDebugStackFrame_GetDescriptionString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugStackFrame_GetLanguageString_Proxy( 
    IDebugStackFrame __RPC_FAR * This,
    /* [in] */ BOOL fLong,
    /* [out] */ BSTR __RPC_FAR *pbstrLanguage);


void __RPC_STUB IDebugStackFrame_GetLanguageString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugStackFrame_GetThread_Proxy( 
    IDebugStackFrame __RPC_FAR * This,
    /* [out] */ IDebugApplicationThread __RPC_FAR *__RPC_FAR *ppat);


void __RPC_STUB IDebugStackFrame_GetThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugStackFrame_GetDebugProperty_Proxy( 
    IDebugStackFrame __RPC_FAR * This,
    /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppDebugProp);


void __RPC_STUB IDebugStackFrame_GetDebugProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugStackFrame_INTERFACE_DEFINED__ */


#ifndef __IDebugStackFrameSniffer_INTERFACE_DEFINED__
#define __IDebugStackFrameSniffer_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugStackFrameSniffer
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugStackFrameSniffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C18-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugStackFrameSniffer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumStackFrames( 
            /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugStackFrameSnifferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugStackFrameSniffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugStackFrameSniffer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugStackFrameSniffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumStackFrames )( 
            IDebugStackFrameSniffer __RPC_FAR * This,
            /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf);
        
        END_INTERFACE
    } IDebugStackFrameSnifferVtbl;

    interface IDebugStackFrameSniffer
    {
        CONST_VTBL struct IDebugStackFrameSnifferVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugStackFrameSniffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugStackFrameSniffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugStackFrameSniffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugStackFrameSniffer_EnumStackFrames(This,ppedsf)	\
    (This)->lpVtbl -> EnumStackFrames(This,ppedsf)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugStackFrameSniffer_EnumStackFrames_Proxy( 
    IDebugStackFrameSniffer __RPC_FAR * This,
    /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf);


void __RPC_STUB IDebugStackFrameSniffer_EnumStackFrames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugStackFrameSniffer_INTERFACE_DEFINED__ */


#ifndef __IDebugStackFrameSnifferEx_INTERFACE_DEFINED__
#define __IDebugStackFrameSnifferEx_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugStackFrameSnifferEx
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugStackFrameSnifferEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C19-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugStackFrameSnifferEx : public IDebugStackFrameSniffer
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumStackFramesEx( 
            /* [in] */ DWORD dwSpMin,
            /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugStackFrameSnifferExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugStackFrameSnifferEx __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugStackFrameSnifferEx __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugStackFrameSnifferEx __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumStackFrames )( 
            IDebugStackFrameSnifferEx __RPC_FAR * This,
            /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumStackFramesEx )( 
            IDebugStackFrameSnifferEx __RPC_FAR * This,
            /* [in] */ DWORD dwSpMin,
            /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf);
        
        END_INTERFACE
    } IDebugStackFrameSnifferExVtbl;

    interface IDebugStackFrameSnifferEx
    {
        CONST_VTBL struct IDebugStackFrameSnifferExVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugStackFrameSnifferEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugStackFrameSnifferEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugStackFrameSnifferEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugStackFrameSnifferEx_EnumStackFrames(This,ppedsf)	\
    (This)->lpVtbl -> EnumStackFrames(This,ppedsf)


#define IDebugStackFrameSnifferEx_EnumStackFramesEx(This,dwSpMin,ppedsf)	\
    (This)->lpVtbl -> EnumStackFramesEx(This,dwSpMin,ppedsf)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugStackFrameSnifferEx_EnumStackFramesEx_Proxy( 
    IDebugStackFrameSnifferEx __RPC_FAR * This,
    /* [in] */ DWORD dwSpMin,
    /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf);


void __RPC_STUB IDebugStackFrameSnifferEx_EnumStackFramesEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugStackFrameSnifferEx_INTERFACE_DEFINED__ */


#ifndef __IDebugSyncOperation_INTERFACE_DEFINED__
#define __IDebugSyncOperation_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugSyncOperation
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugSyncOperation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C1a-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugSyncOperation : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetTargetThread( 
            /* [out] */ IDebugApplicationThread __RPC_FAR *__RPC_FAR *ppatTarget) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Execute( 
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InProgressAbort( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugSyncOperationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugSyncOperation __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugSyncOperation __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugSyncOperation __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTargetThread )( 
            IDebugSyncOperation __RPC_FAR * This,
            /* [out] */ IDebugApplicationThread __RPC_FAR *__RPC_FAR *ppatTarget);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Execute )( 
            IDebugSyncOperation __RPC_FAR * This,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkResult);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InProgressAbort )( 
            IDebugSyncOperation __RPC_FAR * This);
        
        END_INTERFACE
    } IDebugSyncOperationVtbl;

    interface IDebugSyncOperation
    {
        CONST_VTBL struct IDebugSyncOperationVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugSyncOperation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugSyncOperation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugSyncOperation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugSyncOperation_GetTargetThread(This,ppatTarget)	\
    (This)->lpVtbl -> GetTargetThread(This,ppatTarget)

#define IDebugSyncOperation_Execute(This,ppunkResult)	\
    (This)->lpVtbl -> Execute(This,ppunkResult)

#define IDebugSyncOperation_InProgressAbort(This)	\
    (This)->lpVtbl -> InProgressAbort(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugSyncOperation_GetTargetThread_Proxy( 
    IDebugSyncOperation __RPC_FAR * This,
    /* [out] */ IDebugApplicationThread __RPC_FAR *__RPC_FAR *ppatTarget);


void __RPC_STUB IDebugSyncOperation_GetTargetThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugSyncOperation_Execute_Proxy( 
    IDebugSyncOperation __RPC_FAR * This,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkResult);


void __RPC_STUB IDebugSyncOperation_Execute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugSyncOperation_InProgressAbort_Proxy( 
    IDebugSyncOperation __RPC_FAR * This);


void __RPC_STUB IDebugSyncOperation_InProgressAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugSyncOperation_INTERFACE_DEFINED__ */


#ifndef __IDebugAsyncOperation_INTERFACE_DEFINED__
#define __IDebugAsyncOperation_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugAsyncOperation
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugAsyncOperation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C1b-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugAsyncOperation : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSyncDebugOperation( 
            /* [out] */ IDebugSyncOperation __RPC_FAR *__RPC_FAR *ppsdo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Start( 
            IDebugAsyncOperationCallBack __RPC_FAR *padocb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Abort( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryIsComplete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetResult( 
            /* [out] */ HRESULT __RPC_FAR *phrResult,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugAsyncOperationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugAsyncOperation __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugAsyncOperation __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugAsyncOperation __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSyncDebugOperation )( 
            IDebugAsyncOperation __RPC_FAR * This,
            /* [out] */ IDebugSyncOperation __RPC_FAR *__RPC_FAR *ppsdo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Start )( 
            IDebugAsyncOperation __RPC_FAR * This,
            IDebugAsyncOperationCallBack __RPC_FAR *padocb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Abort )( 
            IDebugAsyncOperation __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryIsComplete )( 
            IDebugAsyncOperation __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetResult )( 
            IDebugAsyncOperation __RPC_FAR * This,
            /* [out] */ HRESULT __RPC_FAR *phrResult,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkResult);
        
        END_INTERFACE
    } IDebugAsyncOperationVtbl;

    interface IDebugAsyncOperation
    {
        CONST_VTBL struct IDebugAsyncOperationVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugAsyncOperation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugAsyncOperation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugAsyncOperation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugAsyncOperation_GetSyncDebugOperation(This,ppsdo)	\
    (This)->lpVtbl -> GetSyncDebugOperation(This,ppsdo)

#define IDebugAsyncOperation_Start(This,padocb)	\
    (This)->lpVtbl -> Start(This,padocb)

#define IDebugAsyncOperation_Abort(This)	\
    (This)->lpVtbl -> Abort(This)

#define IDebugAsyncOperation_QueryIsComplete(This)	\
    (This)->lpVtbl -> QueryIsComplete(This)

#define IDebugAsyncOperation_GetResult(This,phrResult,ppunkResult)	\
    (This)->lpVtbl -> GetResult(This,phrResult,ppunkResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugAsyncOperation_GetSyncDebugOperation_Proxy( 
    IDebugAsyncOperation __RPC_FAR * This,
    /* [out] */ IDebugSyncOperation __RPC_FAR *__RPC_FAR *ppsdo);


void __RPC_STUB IDebugAsyncOperation_GetSyncDebugOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugAsyncOperation_Start_Proxy( 
    IDebugAsyncOperation __RPC_FAR * This,
    IDebugAsyncOperationCallBack __RPC_FAR *padocb);


void __RPC_STUB IDebugAsyncOperation_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugAsyncOperation_Abort_Proxy( 
    IDebugAsyncOperation __RPC_FAR * This);


void __RPC_STUB IDebugAsyncOperation_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugAsyncOperation_QueryIsComplete_Proxy( 
    IDebugAsyncOperation __RPC_FAR * This);


void __RPC_STUB IDebugAsyncOperation_QueryIsComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugAsyncOperation_GetResult_Proxy( 
    IDebugAsyncOperation __RPC_FAR * This,
    /* [out] */ HRESULT __RPC_FAR *phrResult,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkResult);


void __RPC_STUB IDebugAsyncOperation_GetResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugAsyncOperation_INTERFACE_DEFINED__ */


#ifndef __IDebugAsyncOperationCallBack_INTERFACE_DEFINED__
#define __IDebugAsyncOperationCallBack_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugAsyncOperationCallBack
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugAsyncOperationCallBack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C1c-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugAsyncOperationCallBack : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE onComplete( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugAsyncOperationCallBackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugAsyncOperationCallBack __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugAsyncOperationCallBack __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugAsyncOperationCallBack __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onComplete )( 
            IDebugAsyncOperationCallBack __RPC_FAR * This);
        
        END_INTERFACE
    } IDebugAsyncOperationCallBackVtbl;

    interface IDebugAsyncOperationCallBack
    {
        CONST_VTBL struct IDebugAsyncOperationCallBackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugAsyncOperationCallBack_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugAsyncOperationCallBack_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugAsyncOperationCallBack_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugAsyncOperationCallBack_onComplete(This)	\
    (This)->lpVtbl -> onComplete(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugAsyncOperationCallBack_onComplete_Proxy( 
    IDebugAsyncOperationCallBack __RPC_FAR * This);


void __RPC_STUB IDebugAsyncOperationCallBack_onComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugAsyncOperationCallBack_INTERFACE_DEFINED__ */


#ifndef __IEnumDebugCodeContexts_INTERFACE_DEFINED__
#define __IEnumDebugCodeContexts_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumDebugCodeContexts
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IEnumDebugCodeContexts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C1d-CB0C-11d0-B5C9-00A0244A0E7A")
    IEnumDebugCodeContexts : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IDebugCodeContext __RPC_FAR *__RPC_FAR *pscc,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumDebugCodeContextsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumDebugCodeContexts __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumDebugCodeContexts __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumDebugCodeContexts __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumDebugCodeContexts __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ IDebugCodeContext __RPC_FAR *__RPC_FAR *pscc,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumDebugCodeContexts __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumDebugCodeContexts __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumDebugCodeContexts __RPC_FAR * This,
            /* [out] */ IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc);
        
        END_INTERFACE
    } IEnumDebugCodeContextsVtbl;

    interface IEnumDebugCodeContexts
    {
        CONST_VTBL struct IEnumDebugCodeContextsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDebugCodeContexts_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDebugCodeContexts_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDebugCodeContexts_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDebugCodeContexts_Next(This,celt,pscc,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,pscc,pceltFetched)

#define IEnumDebugCodeContexts_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumDebugCodeContexts_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDebugCodeContexts_Clone(This,ppescc)	\
    (This)->lpVtbl -> Clone(This,ppescc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumDebugCodeContexts_RemoteNext_Proxy( 
    IEnumDebugCodeContexts __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IDebugCodeContext __RPC_FAR *__RPC_FAR *pscc,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumDebugCodeContexts_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugCodeContexts_Skip_Proxy( 
    IEnumDebugCodeContexts __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumDebugCodeContexts_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugCodeContexts_Reset_Proxy( 
    IEnumDebugCodeContexts __RPC_FAR * This);


void __RPC_STUB IEnumDebugCodeContexts_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugCodeContexts_Clone_Proxy( 
    IEnumDebugCodeContexts __RPC_FAR * This,
    /* [out] */ IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc);


void __RPC_STUB IEnumDebugCodeContexts_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumDebugCodeContexts_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL_itf_activdbg_0177
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local] */ 


typedef struct  tagDebugStackFrameDescriptor
    {
    IDebugStackFrame __RPC_FAR *pdsf;
    DWORD dwMin;
    DWORD dwLim;
    BOOL fFinal;
    IUnknown __RPC_FAR *punkFinal;
    }	DebugStackFrameDescriptor;



extern RPC_IF_HANDLE __MIDL_itf_activdbg_0177_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0177_v0_0_s_ifspec;

#ifndef __IEnumDebugStackFrames_INTERFACE_DEFINED__
#define __IEnumDebugStackFrames_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumDebugStackFrames
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IEnumDebugStackFrames;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C1e-CB0C-11d0-B5C9-00A0244A0E7A")
    IEnumDebugStackFrames : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ DebugStackFrameDescriptor __RPC_FAR *prgdsfd,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumDebugStackFramesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumDebugStackFrames __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumDebugStackFrames __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumDebugStackFrames __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumDebugStackFrames __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ DebugStackFrameDescriptor __RPC_FAR *prgdsfd,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumDebugStackFrames __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumDebugStackFrames __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumDebugStackFrames __RPC_FAR * This,
            /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf);
        
        END_INTERFACE
    } IEnumDebugStackFramesVtbl;

    interface IEnumDebugStackFrames
    {
        CONST_VTBL struct IEnumDebugStackFramesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDebugStackFrames_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDebugStackFrames_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDebugStackFrames_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDebugStackFrames_Next(This,celt,prgdsfd,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,prgdsfd,pceltFetched)

#define IEnumDebugStackFrames_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumDebugStackFrames_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDebugStackFrames_Clone(This,ppedsf)	\
    (This)->lpVtbl -> Clone(This,ppedsf)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumDebugStackFrames_RemoteNext_Proxy( 
    IEnumDebugStackFrames __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ DebugStackFrameDescriptor __RPC_FAR *prgdsfd,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumDebugStackFrames_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugStackFrames_Skip_Proxy( 
    IEnumDebugStackFrames __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumDebugStackFrames_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugStackFrames_Reset_Proxy( 
    IEnumDebugStackFrames __RPC_FAR * This);


void __RPC_STUB IEnumDebugStackFrames_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugStackFrames_Clone_Proxy( 
    IEnumDebugStackFrames __RPC_FAR * This,
    /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf);


void __RPC_STUB IEnumDebugStackFrames_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumDebugStackFrames_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentInfo_INTERFACE_DEFINED__
#define __IDebugDocumentInfo_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugDocumentInfo
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [optimize][unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IDebugDocumentInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C1f-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName( 
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDocumentClassId( 
            /* [out] */ CLSID __RPC_FAR *pclsidDocument) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugDocumentInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugDocumentInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugDocumentInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            IDebugDocumentInfo __RPC_FAR * This,
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR __RPC_FAR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocumentClassId )( 
            IDebugDocumentInfo __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pclsidDocument);
        
        END_INTERFACE
    } IDebugDocumentInfoVtbl;

    interface IDebugDocumentInfo
    {
        CONST_VTBL struct IDebugDocumentInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentInfo_GetName(This,dnt,pbstrName)	\
    (This)->lpVtbl -> GetName(This,dnt,pbstrName)

#define IDebugDocumentInfo_GetDocumentClassId(This,pclsidDocument)	\
    (This)->lpVtbl -> GetDocumentClassId(This,pclsidDocument)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentInfo_GetName_Proxy( 
    IDebugDocumentInfo __RPC_FAR * This,
    /* [in] */ DOCUMENTNAMETYPE dnt,
    /* [out] */ BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IDebugDocumentInfo_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentInfo_GetDocumentClassId_Proxy( 
    IDebugDocumentInfo __RPC_FAR * This,
    /* [out] */ CLSID __RPC_FAR *pclsidDocument);


void __RPC_STUB IDebugDocumentInfo_GetDocumentClassId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentInfo_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentProvider_INTERFACE_DEFINED__
#define __IDebugDocumentProvider_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugDocumentProvider
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IDebugDocumentProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C20-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentProvider : public IDebugDocumentInfo
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocument( 
            /* [out] */ IDebugDocument __RPC_FAR *__RPC_FAR *ppssd) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugDocumentProvider __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugDocumentProvider __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugDocumentProvider __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            IDebugDocumentProvider __RPC_FAR * This,
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR __RPC_FAR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocumentClassId )( 
            IDebugDocumentProvider __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pclsidDocument);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocument )( 
            IDebugDocumentProvider __RPC_FAR * This,
            /* [out] */ IDebugDocument __RPC_FAR *__RPC_FAR *ppssd);
        
        END_INTERFACE
    } IDebugDocumentProviderVtbl;

    interface IDebugDocumentProvider
    {
        CONST_VTBL struct IDebugDocumentProviderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentProvider_GetName(This,dnt,pbstrName)	\
    (This)->lpVtbl -> GetName(This,dnt,pbstrName)

#define IDebugDocumentProvider_GetDocumentClassId(This,pclsidDocument)	\
    (This)->lpVtbl -> GetDocumentClassId(This,pclsidDocument)


#define IDebugDocumentProvider_GetDocument(This,ppssd)	\
    (This)->lpVtbl -> GetDocument(This,ppssd)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentProvider_GetDocument_Proxy( 
    IDebugDocumentProvider __RPC_FAR * This,
    /* [out] */ IDebugDocument __RPC_FAR *__RPC_FAR *ppssd);


void __RPC_STUB IDebugDocumentProvider_GetDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentProvider_INTERFACE_DEFINED__ */


#ifndef __IDebugDocument_INTERFACE_DEFINED__
#define __IDebugDocument_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugDocument
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C21-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocument : public IDebugDocumentInfo
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugDocument __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugDocument __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugDocument __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            IDebugDocument __RPC_FAR * This,
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR __RPC_FAR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocumentClassId )( 
            IDebugDocument __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pclsidDocument);
        
        END_INTERFACE
    } IDebugDocumentVtbl;

    interface IDebugDocument
    {
        CONST_VTBL struct IDebugDocumentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocument_GetName(This,dnt,pbstrName)	\
    (This)->lpVtbl -> GetName(This,dnt,pbstrName)

#define IDebugDocument_GetDocumentClassId(This,pclsidDocument)	\
    (This)->lpVtbl -> GetDocumentClassId(This,pclsidDocument)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDebugDocument_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentText_INTERFACE_DEFINED__
#define __IDebugDocumentText_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugDocumentText
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugDocumentText;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C22-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentText : public IDebugDocument
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocumentAttributes( 
            /* [out] */ TEXT_DOC_ATTR __RPC_FAR *ptextdocattr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSize( 
            /* [out] */ ULONG __RPC_FAR *pcNumLines,
            /* [out] */ ULONG __RPC_FAR *pcNumChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPositionOfLine( 
            /* [in] */ ULONG cLineNumber,
            /* [out] */ ULONG __RPC_FAR *pcCharacterPosition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLineOfPosition( 
            /* [in] */ ULONG cCharacterPosition,
            /* [out] */ ULONG __RPC_FAR *pcLineNumber,
            /* [out] */ ULONG __RPC_FAR *pcCharacterOffsetInLine) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [size_is][length_is][out][in] */ WCHAR __RPC_FAR *pcharText,
            /* [full][size_is][length_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr,
            /* [out][in] */ ULONG __RPC_FAR *pcNumChars,
            /* [in] */ ULONG cMaxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPositionOfContext( 
            /* [in] */ IDebugDocumentContext __RPC_FAR *psc,
            /* [out] */ ULONG __RPC_FAR *pcCharacterPosition,
            /* [out] */ ULONG __RPC_FAR *cNumChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContextOfPosition( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumChars,
            /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentTextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugDocumentText __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugDocumentText __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugDocumentText __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            IDebugDocumentText __RPC_FAR * This,
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR __RPC_FAR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocumentClassId )( 
            IDebugDocumentText __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pclsidDocument);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocumentAttributes )( 
            IDebugDocumentText __RPC_FAR * This,
            /* [out] */ TEXT_DOC_ATTR __RPC_FAR *ptextdocattr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSize )( 
            IDebugDocumentText __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcNumLines,
            /* [out] */ ULONG __RPC_FAR *pcNumChars);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPositionOfLine )( 
            IDebugDocumentText __RPC_FAR * This,
            /* [in] */ ULONG cLineNumber,
            /* [out] */ ULONG __RPC_FAR *pcCharacterPosition);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLineOfPosition )( 
            IDebugDocumentText __RPC_FAR * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [out] */ ULONG __RPC_FAR *pcLineNumber,
            /* [out] */ ULONG __RPC_FAR *pcCharacterOffsetInLine);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetText )( 
            IDebugDocumentText __RPC_FAR * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [size_is][length_is][out][in] */ WCHAR __RPC_FAR *pcharText,
            /* [full][size_is][length_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr,
            /* [out][in] */ ULONG __RPC_FAR *pcNumChars,
            /* [in] */ ULONG cMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPositionOfContext )( 
            IDebugDocumentText __RPC_FAR * This,
            /* [in] */ IDebugDocumentContext __RPC_FAR *psc,
            /* [out] */ ULONG __RPC_FAR *pcCharacterPosition,
            /* [out] */ ULONG __RPC_FAR *cNumChars);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetContextOfPosition )( 
            IDebugDocumentText __RPC_FAR * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumChars,
            /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc);
        
        END_INTERFACE
    } IDebugDocumentTextVtbl;

    interface IDebugDocumentText
    {
        CONST_VTBL struct IDebugDocumentTextVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentText_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentText_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentText_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentText_GetName(This,dnt,pbstrName)	\
    (This)->lpVtbl -> GetName(This,dnt,pbstrName)

#define IDebugDocumentText_GetDocumentClassId(This,pclsidDocument)	\
    (This)->lpVtbl -> GetDocumentClassId(This,pclsidDocument)



#define IDebugDocumentText_GetDocumentAttributes(This,ptextdocattr)	\
    (This)->lpVtbl -> GetDocumentAttributes(This,ptextdocattr)

#define IDebugDocumentText_GetSize(This,pcNumLines,pcNumChars)	\
    (This)->lpVtbl -> GetSize(This,pcNumLines,pcNumChars)

#define IDebugDocumentText_GetPositionOfLine(This,cLineNumber,pcCharacterPosition)	\
    (This)->lpVtbl -> GetPositionOfLine(This,cLineNumber,pcCharacterPosition)

#define IDebugDocumentText_GetLineOfPosition(This,cCharacterPosition,pcLineNumber,pcCharacterOffsetInLine)	\
    (This)->lpVtbl -> GetLineOfPosition(This,cCharacterPosition,pcLineNumber,pcCharacterOffsetInLine)

#define IDebugDocumentText_GetText(This,cCharacterPosition,pcharText,pstaTextAttr,pcNumChars,cMaxChars)	\
    (This)->lpVtbl -> GetText(This,cCharacterPosition,pcharText,pstaTextAttr,pcNumChars,cMaxChars)

#define IDebugDocumentText_GetPositionOfContext(This,psc,pcCharacterPosition,cNumChars)	\
    (This)->lpVtbl -> GetPositionOfContext(This,psc,pcCharacterPosition,cNumChars)

#define IDebugDocumentText_GetContextOfPosition(This,cCharacterPosition,cNumChars,ppsc)	\
    (This)->lpVtbl -> GetContextOfPosition(This,cCharacterPosition,cNumChars,ppsc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetDocumentAttributes_Proxy( 
    IDebugDocumentText __RPC_FAR * This,
    /* [out] */ TEXT_DOC_ATTR __RPC_FAR *ptextdocattr);


void __RPC_STUB IDebugDocumentText_GetDocumentAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetSize_Proxy( 
    IDebugDocumentText __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcNumLines,
    /* [out] */ ULONG __RPC_FAR *pcNumChars);


void __RPC_STUB IDebugDocumentText_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetPositionOfLine_Proxy( 
    IDebugDocumentText __RPC_FAR * This,
    /* [in] */ ULONG cLineNumber,
    /* [out] */ ULONG __RPC_FAR *pcCharacterPosition);


void __RPC_STUB IDebugDocumentText_GetPositionOfLine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetLineOfPosition_Proxy( 
    IDebugDocumentText __RPC_FAR * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [out] */ ULONG __RPC_FAR *pcLineNumber,
    /* [out] */ ULONG __RPC_FAR *pcCharacterOffsetInLine);


void __RPC_STUB IDebugDocumentText_GetLineOfPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetText_Proxy( 
    IDebugDocumentText __RPC_FAR * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [size_is][length_is][out][in] */ WCHAR __RPC_FAR *pcharText,
    /* [full][size_is][length_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr,
    /* [out][in] */ ULONG __RPC_FAR *pcNumChars,
    /* [in] */ ULONG cMaxChars);


void __RPC_STUB IDebugDocumentText_GetText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetPositionOfContext_Proxy( 
    IDebugDocumentText __RPC_FAR * This,
    /* [in] */ IDebugDocumentContext __RPC_FAR *psc,
    /* [out] */ ULONG __RPC_FAR *pcCharacterPosition,
    /* [out] */ ULONG __RPC_FAR *cNumChars);


void __RPC_STUB IDebugDocumentText_GetPositionOfContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetContextOfPosition_Proxy( 
    IDebugDocumentText __RPC_FAR * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumChars,
    /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc);


void __RPC_STUB IDebugDocumentText_GetContextOfPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentText_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentTextEvents_INTERFACE_DEFINED__
#define __IDebugDocumentTextEvents_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugDocumentTextEvents
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugDocumentTextEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C23-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentTextEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE onDestroy( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onInsertText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToInsert) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onRemoveText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToRemove) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onReplaceText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToReplace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onUpdateTextAttributes( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToUpdate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onUpdateDocumentAttributes( 
            /* [in] */ TEXT_DOC_ATTR textdocattr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentTextEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugDocumentTextEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugDocumentTextEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugDocumentTextEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onDestroy )( 
            IDebugDocumentTextEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onInsertText )( 
            IDebugDocumentTextEvents __RPC_FAR * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToInsert);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onRemoveText )( 
            IDebugDocumentTextEvents __RPC_FAR * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToRemove);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onReplaceText )( 
            IDebugDocumentTextEvents __RPC_FAR * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToReplace);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onUpdateTextAttributes )( 
            IDebugDocumentTextEvents __RPC_FAR * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToUpdate);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onUpdateDocumentAttributes )( 
            IDebugDocumentTextEvents __RPC_FAR * This,
            /* [in] */ TEXT_DOC_ATTR textdocattr);
        
        END_INTERFACE
    } IDebugDocumentTextEventsVtbl;

    interface IDebugDocumentTextEvents
    {
        CONST_VTBL struct IDebugDocumentTextEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentTextEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentTextEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentTextEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentTextEvents_onDestroy(This)	\
    (This)->lpVtbl -> onDestroy(This)

#define IDebugDocumentTextEvents_onInsertText(This,cCharacterPosition,cNumToInsert)	\
    (This)->lpVtbl -> onInsertText(This,cCharacterPosition,cNumToInsert)

#define IDebugDocumentTextEvents_onRemoveText(This,cCharacterPosition,cNumToRemove)	\
    (This)->lpVtbl -> onRemoveText(This,cCharacterPosition,cNumToRemove)

#define IDebugDocumentTextEvents_onReplaceText(This,cCharacterPosition,cNumToReplace)	\
    (This)->lpVtbl -> onReplaceText(This,cCharacterPosition,cNumToReplace)

#define IDebugDocumentTextEvents_onUpdateTextAttributes(This,cCharacterPosition,cNumToUpdate)	\
    (This)->lpVtbl -> onUpdateTextAttributes(This,cCharacterPosition,cNumToUpdate)

#define IDebugDocumentTextEvents_onUpdateDocumentAttributes(This,textdocattr)	\
    (This)->lpVtbl -> onUpdateDocumentAttributes(This,textdocattr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentTextEvents_onDestroy_Proxy( 
    IDebugDocumentTextEvents __RPC_FAR * This);


void __RPC_STUB IDebugDocumentTextEvents_onDestroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextEvents_onInsertText_Proxy( 
    IDebugDocumentTextEvents __RPC_FAR * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToInsert);


void __RPC_STUB IDebugDocumentTextEvents_onInsertText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextEvents_onRemoveText_Proxy( 
    IDebugDocumentTextEvents __RPC_FAR * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToRemove);


void __RPC_STUB IDebugDocumentTextEvents_onRemoveText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextEvents_onReplaceText_Proxy( 
    IDebugDocumentTextEvents __RPC_FAR * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToReplace);


void __RPC_STUB IDebugDocumentTextEvents_onReplaceText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextEvents_onUpdateTextAttributes_Proxy( 
    IDebugDocumentTextEvents __RPC_FAR * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToUpdate);


void __RPC_STUB IDebugDocumentTextEvents_onUpdateTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextEvents_onUpdateDocumentAttributes_Proxy( 
    IDebugDocumentTextEvents __RPC_FAR * This,
    /* [in] */ TEXT_DOC_ATTR textdocattr);


void __RPC_STUB IDebugDocumentTextEvents_onUpdateDocumentAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentTextEvents_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentTextAuthor_INTERFACE_DEFINED__
#define __IDebugDocumentTextAuthor_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugDocumentTextAuthor
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugDocumentTextAuthor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C24-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentTextAuthor : public IDebugDocumentText
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InsertText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToInsert,
            /* [size_is][in] */ OLECHAR __RPC_FAR pcharText[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToRemove) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReplaceText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToReplace,
            /* [size_is][in] */ OLECHAR __RPC_FAR pcharText[  ]) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentTextAuthorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugDocumentTextAuthor __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugDocumentTextAuthor __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugDocumentTextAuthor __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            IDebugDocumentTextAuthor __RPC_FAR * This,
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR __RPC_FAR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocumentClassId )( 
            IDebugDocumentTextAuthor __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pclsidDocument);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocumentAttributes )( 
            IDebugDocumentTextAuthor __RPC_FAR * This,
            /* [out] */ TEXT_DOC_ATTR __RPC_FAR *ptextdocattr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSize )( 
            IDebugDocumentTextAuthor __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcNumLines,
            /* [out] */ ULONG __RPC_FAR *pcNumChars);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPositionOfLine )( 
            IDebugDocumentTextAuthor __RPC_FAR * This,
            /* [in] */ ULONG cLineNumber,
            /* [out] */ ULONG __RPC_FAR *pcCharacterPosition);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLineOfPosition )( 
            IDebugDocumentTextAuthor __RPC_FAR * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [out] */ ULONG __RPC_FAR *pcLineNumber,
            /* [out] */ ULONG __RPC_FAR *pcCharacterOffsetInLine);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetText )( 
            IDebugDocumentTextAuthor __RPC_FAR * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [size_is][length_is][out][in] */ WCHAR __RPC_FAR *pcharText,
            /* [full][size_is][length_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr,
            /* [out][in] */ ULONG __RPC_FAR *pcNumChars,
            /* [in] */ ULONG cMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPositionOfContext )( 
            IDebugDocumentTextAuthor __RPC_FAR * This,
            /* [in] */ IDebugDocumentContext __RPC_FAR *psc,
            /* [out] */ ULONG __RPC_FAR *pcCharacterPosition,
            /* [out] */ ULONG __RPC_FAR *cNumChars);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetContextOfPosition )( 
            IDebugDocumentTextAuthor __RPC_FAR * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumChars,
            /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InsertText )( 
            IDebugDocumentTextAuthor __RPC_FAR * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToInsert,
            /* [size_is][in] */ OLECHAR __RPC_FAR pcharText[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveText )( 
            IDebugDocumentTextAuthor __RPC_FAR * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToRemove);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReplaceText )( 
            IDebugDocumentTextAuthor __RPC_FAR * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToReplace,
            /* [size_is][in] */ OLECHAR __RPC_FAR pcharText[  ]);
        
        END_INTERFACE
    } IDebugDocumentTextAuthorVtbl;

    interface IDebugDocumentTextAuthor
    {
        CONST_VTBL struct IDebugDocumentTextAuthorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentTextAuthor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentTextAuthor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentTextAuthor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentTextAuthor_GetName(This,dnt,pbstrName)	\
    (This)->lpVtbl -> GetName(This,dnt,pbstrName)

#define IDebugDocumentTextAuthor_GetDocumentClassId(This,pclsidDocument)	\
    (This)->lpVtbl -> GetDocumentClassId(This,pclsidDocument)



#define IDebugDocumentTextAuthor_GetDocumentAttributes(This,ptextdocattr)	\
    (This)->lpVtbl -> GetDocumentAttributes(This,ptextdocattr)

#define IDebugDocumentTextAuthor_GetSize(This,pcNumLines,pcNumChars)	\
    (This)->lpVtbl -> GetSize(This,pcNumLines,pcNumChars)

#define IDebugDocumentTextAuthor_GetPositionOfLine(This,cLineNumber,pcCharacterPosition)	\
    (This)->lpVtbl -> GetPositionOfLine(This,cLineNumber,pcCharacterPosition)

#define IDebugDocumentTextAuthor_GetLineOfPosition(This,cCharacterPosition,pcLineNumber,pcCharacterOffsetInLine)	\
    (This)->lpVtbl -> GetLineOfPosition(This,cCharacterPosition,pcLineNumber,pcCharacterOffsetInLine)

#define IDebugDocumentTextAuthor_GetText(This,cCharacterPosition,pcharText,pstaTextAttr,pcNumChars,cMaxChars)	\
    (This)->lpVtbl -> GetText(This,cCharacterPosition,pcharText,pstaTextAttr,pcNumChars,cMaxChars)

#define IDebugDocumentTextAuthor_GetPositionOfContext(This,psc,pcCharacterPosition,cNumChars)	\
    (This)->lpVtbl -> GetPositionOfContext(This,psc,pcCharacterPosition,cNumChars)

#define IDebugDocumentTextAuthor_GetContextOfPosition(This,cCharacterPosition,cNumChars,ppsc)	\
    (This)->lpVtbl -> GetContextOfPosition(This,cCharacterPosition,cNumChars,ppsc)


#define IDebugDocumentTextAuthor_InsertText(This,cCharacterPosition,cNumToInsert,pcharText)	\
    (This)->lpVtbl -> InsertText(This,cCharacterPosition,cNumToInsert,pcharText)

#define IDebugDocumentTextAuthor_RemoveText(This,cCharacterPosition,cNumToRemove)	\
    (This)->lpVtbl -> RemoveText(This,cCharacterPosition,cNumToRemove)

#define IDebugDocumentTextAuthor_ReplaceText(This,cCharacterPosition,cNumToReplace,pcharText)	\
    (This)->lpVtbl -> ReplaceText(This,cCharacterPosition,cNumToReplace,pcharText)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentTextAuthor_InsertText_Proxy( 
    IDebugDocumentTextAuthor __RPC_FAR * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToInsert,
    /* [size_is][in] */ OLECHAR __RPC_FAR pcharText[  ]);


void __RPC_STUB IDebugDocumentTextAuthor_InsertText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextAuthor_RemoveText_Proxy( 
    IDebugDocumentTextAuthor __RPC_FAR * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToRemove);


void __RPC_STUB IDebugDocumentTextAuthor_RemoveText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextAuthor_ReplaceText_Proxy( 
    IDebugDocumentTextAuthor __RPC_FAR * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToReplace,
    /* [size_is][in] */ OLECHAR __RPC_FAR pcharText[  ]);


void __RPC_STUB IDebugDocumentTextAuthor_ReplaceText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentTextAuthor_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentTextExternalAuthor_INTERFACE_DEFINED__
#define __IDebugDocumentTextExternalAuthor_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugDocumentTextExternalAuthor
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugDocumentTextExternalAuthor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C25-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentTextExternalAuthor : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPathName( 
            /* [out] */ BSTR __RPC_FAR *pbstrLongName,
            /* [out] */ BOOL __RPC_FAR *pfIsOriginalFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFileName( 
            /* [out] */ BSTR __RPC_FAR *pbstrShortName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyChanged( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentTextExternalAuthorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugDocumentTextExternalAuthor __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugDocumentTextExternalAuthor __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugDocumentTextExternalAuthor __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPathName )( 
            IDebugDocumentTextExternalAuthor __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrLongName,
            /* [out] */ BOOL __RPC_FAR *pfIsOriginalFile);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFileName )( 
            IDebugDocumentTextExternalAuthor __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrShortName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NotifyChanged )( 
            IDebugDocumentTextExternalAuthor __RPC_FAR * This);
        
        END_INTERFACE
    } IDebugDocumentTextExternalAuthorVtbl;

    interface IDebugDocumentTextExternalAuthor
    {
        CONST_VTBL struct IDebugDocumentTextExternalAuthorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentTextExternalAuthor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentTextExternalAuthor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentTextExternalAuthor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentTextExternalAuthor_GetPathName(This,pbstrLongName,pfIsOriginalFile)	\
    (This)->lpVtbl -> GetPathName(This,pbstrLongName,pfIsOriginalFile)

#define IDebugDocumentTextExternalAuthor_GetFileName(This,pbstrShortName)	\
    (This)->lpVtbl -> GetFileName(This,pbstrShortName)

#define IDebugDocumentTextExternalAuthor_NotifyChanged(This)	\
    (This)->lpVtbl -> NotifyChanged(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentTextExternalAuthor_GetPathName_Proxy( 
    IDebugDocumentTextExternalAuthor __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrLongName,
    /* [out] */ BOOL __RPC_FAR *pfIsOriginalFile);


void __RPC_STUB IDebugDocumentTextExternalAuthor_GetPathName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextExternalAuthor_GetFileName_Proxy( 
    IDebugDocumentTextExternalAuthor __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrShortName);


void __RPC_STUB IDebugDocumentTextExternalAuthor_GetFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextExternalAuthor_NotifyChanged_Proxy( 
    IDebugDocumentTextExternalAuthor __RPC_FAR * This);


void __RPC_STUB IDebugDocumentTextExternalAuthor_NotifyChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentTextExternalAuthor_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentHelper_INTERFACE_DEFINED__
#define __IDebugDocumentHelper_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugDocumentHelper
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IDebugDocumentHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C26-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentHelper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
            /* [in] */ IDebugApplication __RPC_FAR *pda,
            /* [string][in] */ LPCOLESTR pszShortName,
            /* [string][in] */ LPCOLESTR pszLongName,
            /* [in] */ TEXT_DOC_ATTR docAttr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Attach( 
            /* [in] */ IDebugDocumentHelper __RPC_FAR *pddhParent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Detach( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddUnicodeText( 
            /* [string][in] */ LPCOLESTR pszText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddDBCSText( 
            /* [string][in] */ LPCSTR pszText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDebugDocumentHost( 
            /* [in] */ IDebugDocumentHost __RPC_FAR *pddh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddDeferredText( 
            /* [in] */ ULONG cChars,
            /* [in] */ DWORD dwTextStartCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineScriptBlock( 
            /* [in] */ ULONG ulCharOffset,
            /* [in] */ ULONG cChars,
            /* [in] */ IActiveScript __RPC_FAR *pas,
            /* [in] */ BOOL fScriptlet,
            /* [out] */ DWORD __RPC_FAR *pdwSourceContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDefaultTextAttr( 
            SOURCE_TEXT_ATTR staTextAttr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTextAttributes( 
            /* [in] */ ULONG ulCharOffset,
            /* [in] */ ULONG cChars,
            /* [size_is][length_is][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLongName( 
            /* [string][in] */ LPCOLESTR pszLongName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetShortName( 
            /* [string][in] */ LPCOLESTR pszShortName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDocumentAttr( 
            /* [in] */ TEXT_DOC_ATTR pszAttributes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDebugApplicationNode( 
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdan) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScriptBlockInfo( 
            /* [in] */ DWORD dwSourceContext,
            /* [out] */ IActiveScript __RPC_FAR *__RPC_FAR *ppasd,
            /* [out] */ ULONG __RPC_FAR *piCharPos,
            /* [out] */ ULONG __RPC_FAR *pcChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateDebugDocumentContext( 
            /* [in] */ ULONG iCharPos,
            /* [in] */ ULONG cChars,
            /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppddc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BringDocumentToTop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BringDocumentContextToTop( 
            IDebugDocumentContext __RPC_FAR *pddc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugDocumentHelper __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugDocumentHelper __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Init )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [in] */ IDebugApplication __RPC_FAR *pda,
            /* [string][in] */ LPCOLESTR pszShortName,
            /* [string][in] */ LPCOLESTR pszLongName,
            /* [in] */ TEXT_DOC_ATTR docAttr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Attach )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [in] */ IDebugDocumentHelper __RPC_FAR *pddhParent);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Detach )( 
            IDebugDocumentHelper __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddUnicodeText )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [string][in] */ LPCOLESTR pszText);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddDBCSText )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [string][in] */ LPCSTR pszText);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDebugDocumentHost )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [in] */ IDebugDocumentHost __RPC_FAR *pddh);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddDeferredText )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [in] */ ULONG cChars,
            /* [in] */ DWORD dwTextStartCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DefineScriptBlock )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [in] */ ULONG ulCharOffset,
            /* [in] */ ULONG cChars,
            /* [in] */ IActiveScript __RPC_FAR *pas,
            /* [in] */ BOOL fScriptlet,
            /* [out] */ DWORD __RPC_FAR *pdwSourceContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDefaultTextAttr )( 
            IDebugDocumentHelper __RPC_FAR * This,
            SOURCE_TEXT_ATTR staTextAttr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetTextAttributes )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [in] */ ULONG ulCharOffset,
            /* [in] */ ULONG cChars,
            /* [size_is][length_is][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLongName )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [string][in] */ LPCOLESTR pszLongName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetShortName )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [string][in] */ LPCOLESTR pszShortName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDocumentAttr )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [in] */ TEXT_DOC_ATTR pszAttributes);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDebugApplicationNode )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdan);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScriptBlockInfo )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [in] */ DWORD dwSourceContext,
            /* [out] */ IActiveScript __RPC_FAR *__RPC_FAR *ppasd,
            /* [out] */ ULONG __RPC_FAR *piCharPos,
            /* [out] */ ULONG __RPC_FAR *pcChars);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateDebugDocumentContext )( 
            IDebugDocumentHelper __RPC_FAR * This,
            /* [in] */ ULONG iCharPos,
            /* [in] */ ULONG cChars,
            /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppddc);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BringDocumentToTop )( 
            IDebugDocumentHelper __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BringDocumentContextToTop )( 
            IDebugDocumentHelper __RPC_FAR * This,
            IDebugDocumentContext __RPC_FAR *pddc);
        
        END_INTERFACE
    } IDebugDocumentHelperVtbl;

    interface IDebugDocumentHelper
    {
        CONST_VTBL struct IDebugDocumentHelperVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentHelper_Init(This,pda,pszShortName,pszLongName,docAttr)	\
    (This)->lpVtbl -> Init(This,pda,pszShortName,pszLongName,docAttr)

#define IDebugDocumentHelper_Attach(This,pddhParent)	\
    (This)->lpVtbl -> Attach(This,pddhParent)

#define IDebugDocumentHelper_Detach(This)	\
    (This)->lpVtbl -> Detach(This)

#define IDebugDocumentHelper_AddUnicodeText(This,pszText)	\
    (This)->lpVtbl -> AddUnicodeText(This,pszText)

#define IDebugDocumentHelper_AddDBCSText(This,pszText)	\
    (This)->lpVtbl -> AddDBCSText(This,pszText)

#define IDebugDocumentHelper_SetDebugDocumentHost(This,pddh)	\
    (This)->lpVtbl -> SetDebugDocumentHost(This,pddh)

#define IDebugDocumentHelper_AddDeferredText(This,cChars,dwTextStartCookie)	\
    (This)->lpVtbl -> AddDeferredText(This,cChars,dwTextStartCookie)

#define IDebugDocumentHelper_DefineScriptBlock(This,ulCharOffset,cChars,pas,fScriptlet,pdwSourceContext)	\
    (This)->lpVtbl -> DefineScriptBlock(This,ulCharOffset,cChars,pas,fScriptlet,pdwSourceContext)

#define IDebugDocumentHelper_SetDefaultTextAttr(This,staTextAttr)	\
    (This)->lpVtbl -> SetDefaultTextAttr(This,staTextAttr)

#define IDebugDocumentHelper_SetTextAttributes(This,ulCharOffset,cChars,pstaTextAttr)	\
    (This)->lpVtbl -> SetTextAttributes(This,ulCharOffset,cChars,pstaTextAttr)

#define IDebugDocumentHelper_SetLongName(This,pszLongName)	\
    (This)->lpVtbl -> SetLongName(This,pszLongName)

#define IDebugDocumentHelper_SetShortName(This,pszShortName)	\
    (This)->lpVtbl -> SetShortName(This,pszShortName)

#define IDebugDocumentHelper_SetDocumentAttr(This,pszAttributes)	\
    (This)->lpVtbl -> SetDocumentAttr(This,pszAttributes)

#define IDebugDocumentHelper_GetDebugApplicationNode(This,ppdan)	\
    (This)->lpVtbl -> GetDebugApplicationNode(This,ppdan)

#define IDebugDocumentHelper_GetScriptBlockInfo(This,dwSourceContext,ppasd,piCharPos,pcChars)	\
    (This)->lpVtbl -> GetScriptBlockInfo(This,dwSourceContext,ppasd,piCharPos,pcChars)

#define IDebugDocumentHelper_CreateDebugDocumentContext(This,iCharPos,cChars,ppddc)	\
    (This)->lpVtbl -> CreateDebugDocumentContext(This,iCharPos,cChars,ppddc)

#define IDebugDocumentHelper_BringDocumentToTop(This)	\
    (This)->lpVtbl -> BringDocumentToTop(This)

#define IDebugDocumentHelper_BringDocumentContextToTop(This,pddc)	\
    (This)->lpVtbl -> BringDocumentContextToTop(This,pddc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_Init_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [in] */ IDebugApplication __RPC_FAR *pda,
    /* [string][in] */ LPCOLESTR pszShortName,
    /* [string][in] */ LPCOLESTR pszLongName,
    /* [in] */ TEXT_DOC_ATTR docAttr);


void __RPC_STUB IDebugDocumentHelper_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_Attach_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [in] */ IDebugDocumentHelper __RPC_FAR *pddhParent);


void __RPC_STUB IDebugDocumentHelper_Attach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_Detach_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This);


void __RPC_STUB IDebugDocumentHelper_Detach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_AddUnicodeText_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [string][in] */ LPCOLESTR pszText);


void __RPC_STUB IDebugDocumentHelper_AddUnicodeText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_AddDBCSText_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [string][in] */ LPCSTR pszText);


void __RPC_STUB IDebugDocumentHelper_AddDBCSText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_SetDebugDocumentHost_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [in] */ IDebugDocumentHost __RPC_FAR *pddh);


void __RPC_STUB IDebugDocumentHelper_SetDebugDocumentHost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_AddDeferredText_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [in] */ ULONG cChars,
    /* [in] */ DWORD dwTextStartCookie);


void __RPC_STUB IDebugDocumentHelper_AddDeferredText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_DefineScriptBlock_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [in] */ ULONG ulCharOffset,
    /* [in] */ ULONG cChars,
    /* [in] */ IActiveScript __RPC_FAR *pas,
    /* [in] */ BOOL fScriptlet,
    /* [out] */ DWORD __RPC_FAR *pdwSourceContext);


void __RPC_STUB IDebugDocumentHelper_DefineScriptBlock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_SetDefaultTextAttr_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    SOURCE_TEXT_ATTR staTextAttr);


void __RPC_STUB IDebugDocumentHelper_SetDefaultTextAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_SetTextAttributes_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [in] */ ULONG ulCharOffset,
    /* [in] */ ULONG cChars,
    /* [size_is][length_is][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr);


void __RPC_STUB IDebugDocumentHelper_SetTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_SetLongName_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [string][in] */ LPCOLESTR pszLongName);


void __RPC_STUB IDebugDocumentHelper_SetLongName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_SetShortName_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [string][in] */ LPCOLESTR pszShortName);


void __RPC_STUB IDebugDocumentHelper_SetShortName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_SetDocumentAttr_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [in] */ TEXT_DOC_ATTR pszAttributes);


void __RPC_STUB IDebugDocumentHelper_SetDocumentAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_GetDebugApplicationNode_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdan);


void __RPC_STUB IDebugDocumentHelper_GetDebugApplicationNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_GetScriptBlockInfo_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [in] */ DWORD dwSourceContext,
    /* [out] */ IActiveScript __RPC_FAR *__RPC_FAR *ppasd,
    /* [out] */ ULONG __RPC_FAR *piCharPos,
    /* [out] */ ULONG __RPC_FAR *pcChars);


void __RPC_STUB IDebugDocumentHelper_GetScriptBlockInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_CreateDebugDocumentContext_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    /* [in] */ ULONG iCharPos,
    /* [in] */ ULONG cChars,
    /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppddc);


void __RPC_STUB IDebugDocumentHelper_CreateDebugDocumentContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_BringDocumentToTop_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This);


void __RPC_STUB IDebugDocumentHelper_BringDocumentToTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper_BringDocumentContextToTop_Proxy( 
    IDebugDocumentHelper __RPC_FAR * This,
    IDebugDocumentContext __RPC_FAR *pddc);


void __RPC_STUB IDebugDocumentHelper_BringDocumentContextToTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentHelper_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentHost_INTERFACE_DEFINED__
#define __IDebugDocumentHost_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugDocumentHost
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IDebugDocumentHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C27-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentHost : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDeferredText( 
            /* [in] */ DWORD dwTextStartCookie,
            /* [size_is][length_is][out][in] */ WCHAR __RPC_FAR *pcharText,
            /* [size_is][length_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr,
            /* [out][in] */ ULONG __RPC_FAR *pcNumChars,
            /* [in] */ ULONG cMaxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScriptTextAttributes( 
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pattr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCreateDocumentContext( 
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkOuter) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPathName( 
            /* [out] */ BSTR __RPC_FAR *pbstrLongName,
            /* [out] */ BOOL __RPC_FAR *pfIsOriginalFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFileName( 
            /* [out] */ BSTR __RPC_FAR *pbstrShortName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyChanged( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugDocumentHost __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugDocumentHost __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugDocumentHost __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDeferredText )( 
            IDebugDocumentHost __RPC_FAR * This,
            /* [in] */ DWORD dwTextStartCookie,
            /* [size_is][length_is][out][in] */ WCHAR __RPC_FAR *pcharText,
            /* [size_is][length_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr,
            /* [out][in] */ ULONG __RPC_FAR *pcNumChars,
            /* [in] */ ULONG cMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScriptTextAttributes )( 
            IDebugDocumentHost __RPC_FAR * This,
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pattr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnCreateDocumentContext )( 
            IDebugDocumentHost __RPC_FAR * This,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkOuter);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPathName )( 
            IDebugDocumentHost __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrLongName,
            /* [out] */ BOOL __RPC_FAR *pfIsOriginalFile);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFileName )( 
            IDebugDocumentHost __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrShortName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NotifyChanged )( 
            IDebugDocumentHost __RPC_FAR * This);
        
        END_INTERFACE
    } IDebugDocumentHostVtbl;

    interface IDebugDocumentHost
    {
        CONST_VTBL struct IDebugDocumentHostVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentHost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentHost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentHost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentHost_GetDeferredText(This,dwTextStartCookie,pcharText,pstaTextAttr,pcNumChars,cMaxChars)	\
    (This)->lpVtbl -> GetDeferredText(This,dwTextStartCookie,pcharText,pstaTextAttr,pcNumChars,cMaxChars)

#define IDebugDocumentHost_GetScriptTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)	\
    (This)->lpVtbl -> GetScriptTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)

#define IDebugDocumentHost_OnCreateDocumentContext(This,ppunkOuter)	\
    (This)->lpVtbl -> OnCreateDocumentContext(This,ppunkOuter)

#define IDebugDocumentHost_GetPathName(This,pbstrLongName,pfIsOriginalFile)	\
    (This)->lpVtbl -> GetPathName(This,pbstrLongName,pfIsOriginalFile)

#define IDebugDocumentHost_GetFileName(This,pbstrShortName)	\
    (This)->lpVtbl -> GetFileName(This,pbstrShortName)

#define IDebugDocumentHost_NotifyChanged(This)	\
    (This)->lpVtbl -> NotifyChanged(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentHost_GetDeferredText_Proxy( 
    IDebugDocumentHost __RPC_FAR * This,
    /* [in] */ DWORD dwTextStartCookie,
    /* [size_is][length_is][out][in] */ WCHAR __RPC_FAR *pcharText,
    /* [size_is][length_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr,
    /* [out][in] */ ULONG __RPC_FAR *pcNumChars,
    /* [in] */ ULONG cMaxChars);


void __RPC_STUB IDebugDocumentHost_GetDeferredText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHost_GetScriptTextAttributes_Proxy( 
    IDebugDocumentHost __RPC_FAR * This,
    /* [size_is][in] */ LPCOLESTR pstrCode,
    /* [in] */ ULONG uNumCodeChars,
    /* [in] */ LPCOLESTR pstrDelimiter,
    /* [in] */ DWORD dwFlags,
    /* [size_is][out][in] */ SOURCE_TEXT_ATTR __RPC_FAR *pattr);


void __RPC_STUB IDebugDocumentHost_GetScriptTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHost_OnCreateDocumentContext_Proxy( 
    IDebugDocumentHost __RPC_FAR * This,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkOuter);


void __RPC_STUB IDebugDocumentHost_OnCreateDocumentContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHost_GetPathName_Proxy( 
    IDebugDocumentHost __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrLongName,
    /* [out] */ BOOL __RPC_FAR *pfIsOriginalFile);


void __RPC_STUB IDebugDocumentHost_GetPathName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHost_GetFileName_Proxy( 
    IDebugDocumentHost __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrShortName);


void __RPC_STUB IDebugDocumentHost_GetFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHost_NotifyChanged_Proxy( 
    IDebugDocumentHost __RPC_FAR * This);


void __RPC_STUB IDebugDocumentHost_NotifyChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentHost_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentContext_INTERFACE_DEFINED__
#define __IDebugDocumentContext_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugDocumentContext
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugDocumentContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C28-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentContext : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocument( 
            /* [out] */ IDebugDocument __RPC_FAR *__RPC_FAR *ppsd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCodeContexts( 
            /* [out] */ IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugDocumentContext __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugDocumentContext __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugDocumentContext __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocument )( 
            IDebugDocumentContext __RPC_FAR * This,
            /* [out] */ IDebugDocument __RPC_FAR *__RPC_FAR *ppsd);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumCodeContexts )( 
            IDebugDocumentContext __RPC_FAR * This,
            /* [out] */ IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc);
        
        END_INTERFACE
    } IDebugDocumentContextVtbl;

    interface IDebugDocumentContext
    {
        CONST_VTBL struct IDebugDocumentContextVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentContext_GetDocument(This,ppsd)	\
    (This)->lpVtbl -> GetDocument(This,ppsd)

#define IDebugDocumentContext_EnumCodeContexts(This,ppescc)	\
    (This)->lpVtbl -> EnumCodeContexts(This,ppescc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentContext_GetDocument_Proxy( 
    IDebugDocumentContext __RPC_FAR * This,
    /* [out] */ IDebugDocument __RPC_FAR *__RPC_FAR *ppsd);


void __RPC_STUB IDebugDocumentContext_GetDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentContext_EnumCodeContexts_Proxy( 
    IDebugDocumentContext __RPC_FAR * This,
    /* [out] */ IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc);


void __RPC_STUB IDebugDocumentContext_EnumCodeContexts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentContext_INTERFACE_DEFINED__ */


#ifndef __IDebugSessionProvider_INTERFACE_DEFINED__
#define __IDebugSessionProvider_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugSessionProvider
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IDebugSessionProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C29-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugSessionProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE StartDebugSession( 
            /* [in] */ IRemoteDebugApplication __RPC_FAR *pda) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugSessionProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugSessionProvider __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugSessionProvider __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugSessionProvider __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StartDebugSession )( 
            IDebugSessionProvider __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplication __RPC_FAR *pda);
        
        END_INTERFACE
    } IDebugSessionProviderVtbl;

    interface IDebugSessionProvider
    {
        CONST_VTBL struct IDebugSessionProviderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugSessionProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugSessionProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugSessionProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugSessionProvider_StartDebugSession(This,pda)	\
    (This)->lpVtbl -> StartDebugSession(This,pda)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugSessionProvider_StartDebugSession_Proxy( 
    IDebugSessionProvider __RPC_FAR * This,
    /* [in] */ IRemoteDebugApplication __RPC_FAR *pda);


void __RPC_STUB IDebugSessionProvider_StartDebugSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugSessionProvider_INTERFACE_DEFINED__ */


#ifndef __IApplicationDebugger_INTERFACE_DEFINED__
#define __IApplicationDebugger_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IApplicationDebugger
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IApplicationDebugger;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C2a-CB0C-11d0-B5C9-00A0244A0E7A")
    IApplicationDebugger : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryAlive( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateInstanceAtDebugger( 
            /* [in] */ REFCLSID rclsid,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ DWORD dwClsContext,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onDebugOutput( 
            /* [in] */ LPCOLESTR pstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onHandleBreakPoint( 
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prpt,
            /* [in] */ BREAKREASON br,
            /* [in] */ IActiveScriptErrorDebug __RPC_FAR *pError) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onClose( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onDebuggerEvent( 
            /* [in] */ REFIID riid,
            /* [in] */ IUnknown __RPC_FAR *punk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IApplicationDebuggerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IApplicationDebugger __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IApplicationDebugger __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IApplicationDebugger __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryAlive )( 
            IApplicationDebugger __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateInstanceAtDebugger )( 
            IApplicationDebugger __RPC_FAR * This,
            /* [in] */ REFCLSID rclsid,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ DWORD dwClsContext,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onDebugOutput )( 
            IApplicationDebugger __RPC_FAR * This,
            /* [in] */ LPCOLESTR pstr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onHandleBreakPoint )( 
            IApplicationDebugger __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prpt,
            /* [in] */ BREAKREASON br,
            /* [in] */ IActiveScriptErrorDebug __RPC_FAR *pError);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onClose )( 
            IApplicationDebugger __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onDebuggerEvent )( 
            IApplicationDebugger __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [in] */ IUnknown __RPC_FAR *punk);
        
        END_INTERFACE
    } IApplicationDebuggerVtbl;

    interface IApplicationDebugger
    {
        CONST_VTBL struct IApplicationDebuggerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IApplicationDebugger_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IApplicationDebugger_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IApplicationDebugger_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IApplicationDebugger_QueryAlive(This)	\
    (This)->lpVtbl -> QueryAlive(This)

#define IApplicationDebugger_CreateInstanceAtDebugger(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)	\
    (This)->lpVtbl -> CreateInstanceAtDebugger(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)

#define IApplicationDebugger_onDebugOutput(This,pstr)	\
    (This)->lpVtbl -> onDebugOutput(This,pstr)

#define IApplicationDebugger_onHandleBreakPoint(This,prpt,br,pError)	\
    (This)->lpVtbl -> onHandleBreakPoint(This,prpt,br,pError)

#define IApplicationDebugger_onClose(This)	\
    (This)->lpVtbl -> onClose(This)

#define IApplicationDebugger_onDebuggerEvent(This,riid,punk)	\
    (This)->lpVtbl -> onDebuggerEvent(This,riid,punk)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IApplicationDebugger_QueryAlive_Proxy( 
    IApplicationDebugger __RPC_FAR * This);


void __RPC_STUB IApplicationDebugger_QueryAlive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationDebugger_CreateInstanceAtDebugger_Proxy( 
    IApplicationDebugger __RPC_FAR * This,
    /* [in] */ REFCLSID rclsid,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [in] */ DWORD dwClsContext,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObject);


void __RPC_STUB IApplicationDebugger_CreateInstanceAtDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationDebugger_onDebugOutput_Proxy( 
    IApplicationDebugger __RPC_FAR * This,
    /* [in] */ LPCOLESTR pstr);


void __RPC_STUB IApplicationDebugger_onDebugOutput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationDebugger_onHandleBreakPoint_Proxy( 
    IApplicationDebugger __RPC_FAR * This,
    /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prpt,
    /* [in] */ BREAKREASON br,
    /* [in] */ IActiveScriptErrorDebug __RPC_FAR *pError);


void __RPC_STUB IApplicationDebugger_onHandleBreakPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationDebugger_onClose_Proxy( 
    IApplicationDebugger __RPC_FAR * This);


void __RPC_STUB IApplicationDebugger_onClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationDebugger_onDebuggerEvent_Proxy( 
    IApplicationDebugger __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [in] */ IUnknown __RPC_FAR *punk);


void __RPC_STUB IApplicationDebugger_onDebuggerEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IApplicationDebugger_INTERFACE_DEFINED__ */


#ifndef __IApplicationDebuggerUI_INTERFACE_DEFINED__
#define __IApplicationDebuggerUI_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IApplicationDebuggerUI
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IApplicationDebuggerUI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C2b-CB0C-11d0-B5C9-00A0244A0E7A")
    IApplicationDebuggerUI : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BringDocumentToTop( 
            /* [in] */ IDebugDocumentText __RPC_FAR *pddt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BringDocumentContextToTop( 
            /* [in] */ IDebugDocumentContext __RPC_FAR *pddc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IApplicationDebuggerUIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IApplicationDebuggerUI __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IApplicationDebuggerUI __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IApplicationDebuggerUI __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BringDocumentToTop )( 
            IApplicationDebuggerUI __RPC_FAR * This,
            /* [in] */ IDebugDocumentText __RPC_FAR *pddt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BringDocumentContextToTop )( 
            IApplicationDebuggerUI __RPC_FAR * This,
            /* [in] */ IDebugDocumentContext __RPC_FAR *pddc);
        
        END_INTERFACE
    } IApplicationDebuggerUIVtbl;

    interface IApplicationDebuggerUI
    {
        CONST_VTBL struct IApplicationDebuggerUIVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IApplicationDebuggerUI_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IApplicationDebuggerUI_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IApplicationDebuggerUI_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IApplicationDebuggerUI_BringDocumentToTop(This,pddt)	\
    (This)->lpVtbl -> BringDocumentToTop(This,pddt)

#define IApplicationDebuggerUI_BringDocumentContextToTop(This,pddc)	\
    (This)->lpVtbl -> BringDocumentContextToTop(This,pddc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IApplicationDebuggerUI_BringDocumentToTop_Proxy( 
    IApplicationDebuggerUI __RPC_FAR * This,
    /* [in] */ IDebugDocumentText __RPC_FAR *pddt);


void __RPC_STUB IApplicationDebuggerUI_BringDocumentToTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationDebuggerUI_BringDocumentContextToTop_Proxy( 
    IApplicationDebuggerUI __RPC_FAR * This,
    /* [in] */ IDebugDocumentContext __RPC_FAR *pddc);


void __RPC_STUB IApplicationDebuggerUI_BringDocumentContextToTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IApplicationDebuggerUI_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL_itf_activdbg_0191
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local] */ 


EXTERN_C const CLSID CLSID_MachineDebugManager;


extern RPC_IF_HANDLE __MIDL_itf_activdbg_0191_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0191_v0_0_s_ifspec;

#ifndef __IMachineDebugManager_INTERFACE_DEFINED__
#define __IMachineDebugManager_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMachineDebugManager
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IMachineDebugManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C2c-CB0C-11d0-B5C9-00A0244A0E7A")
    IMachineDebugManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddApplication( 
            /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
            /* [out] */ DWORD __RPC_FAR *pdwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveApplication( 
            /* [in] */ DWORD dwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumApplications( 
            /* [out] */ IEnumRemoteDebugApplications __RPC_FAR *__RPC_FAR *ppeda) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMachineDebugManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMachineDebugManager __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMachineDebugManager __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMachineDebugManager __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddApplication )( 
            IMachineDebugManager __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
            /* [out] */ DWORD __RPC_FAR *pdwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveApplication )( 
            IMachineDebugManager __RPC_FAR * This,
            /* [in] */ DWORD dwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumApplications )( 
            IMachineDebugManager __RPC_FAR * This,
            /* [out] */ IEnumRemoteDebugApplications __RPC_FAR *__RPC_FAR *ppeda);
        
        END_INTERFACE
    } IMachineDebugManagerVtbl;

    interface IMachineDebugManager
    {
        CONST_VTBL struct IMachineDebugManagerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMachineDebugManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMachineDebugManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMachineDebugManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMachineDebugManager_AddApplication(This,pda,pdwAppCookie)	\
    (This)->lpVtbl -> AddApplication(This,pda,pdwAppCookie)

#define IMachineDebugManager_RemoveApplication(This,dwAppCookie)	\
    (This)->lpVtbl -> RemoveApplication(This,dwAppCookie)

#define IMachineDebugManager_EnumApplications(This,ppeda)	\
    (This)->lpVtbl -> EnumApplications(This,ppeda)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMachineDebugManager_AddApplication_Proxy( 
    IMachineDebugManager __RPC_FAR * This,
    /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
    /* [out] */ DWORD __RPC_FAR *pdwAppCookie);


void __RPC_STUB IMachineDebugManager_AddApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMachineDebugManager_RemoveApplication_Proxy( 
    IMachineDebugManager __RPC_FAR * This,
    /* [in] */ DWORD dwAppCookie);


void __RPC_STUB IMachineDebugManager_RemoveApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMachineDebugManager_EnumApplications_Proxy( 
    IMachineDebugManager __RPC_FAR * This,
    /* [out] */ IEnumRemoteDebugApplications __RPC_FAR *__RPC_FAR *ppeda);


void __RPC_STUB IMachineDebugManager_EnumApplications_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMachineDebugManager_INTERFACE_DEFINED__ */


#ifndef __IMachineDebugManagerCookie_INTERFACE_DEFINED__
#define __IMachineDebugManagerCookie_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMachineDebugManagerCookie
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IMachineDebugManagerCookie;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C2d-CB0C-11d0-B5C9-00A0244A0E7A")
    IMachineDebugManagerCookie : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddApplication( 
            /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
            /* [in] */ DWORD dwDebugAppCookie,
            /* [out] */ DWORD __RPC_FAR *pdwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveApplication( 
            /* [in] */ DWORD dwDebugAppCookie,
            /* [in] */ DWORD dwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumApplications( 
            /* [out] */ IEnumRemoteDebugApplications __RPC_FAR *__RPC_FAR *ppeda) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMachineDebugManagerCookieVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMachineDebugManagerCookie __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMachineDebugManagerCookie __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMachineDebugManagerCookie __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddApplication )( 
            IMachineDebugManagerCookie __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
            /* [in] */ DWORD dwDebugAppCookie,
            /* [out] */ DWORD __RPC_FAR *pdwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveApplication )( 
            IMachineDebugManagerCookie __RPC_FAR * This,
            /* [in] */ DWORD dwDebugAppCookie,
            /* [in] */ DWORD dwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumApplications )( 
            IMachineDebugManagerCookie __RPC_FAR * This,
            /* [out] */ IEnumRemoteDebugApplications __RPC_FAR *__RPC_FAR *ppeda);
        
        END_INTERFACE
    } IMachineDebugManagerCookieVtbl;

    interface IMachineDebugManagerCookie
    {
        CONST_VTBL struct IMachineDebugManagerCookieVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMachineDebugManagerCookie_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMachineDebugManagerCookie_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMachineDebugManagerCookie_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMachineDebugManagerCookie_AddApplication(This,pda,dwDebugAppCookie,pdwAppCookie)	\
    (This)->lpVtbl -> AddApplication(This,pda,dwDebugAppCookie,pdwAppCookie)

#define IMachineDebugManagerCookie_RemoveApplication(This,dwDebugAppCookie,dwAppCookie)	\
    (This)->lpVtbl -> RemoveApplication(This,dwDebugAppCookie,dwAppCookie)

#define IMachineDebugManagerCookie_EnumApplications(This,ppeda)	\
    (This)->lpVtbl -> EnumApplications(This,ppeda)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMachineDebugManagerCookie_AddApplication_Proxy( 
    IMachineDebugManagerCookie __RPC_FAR * This,
    /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
    /* [in] */ DWORD dwDebugAppCookie,
    /* [out] */ DWORD __RPC_FAR *pdwAppCookie);


void __RPC_STUB IMachineDebugManagerCookie_AddApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMachineDebugManagerCookie_RemoveApplication_Proxy( 
    IMachineDebugManagerCookie __RPC_FAR * This,
    /* [in] */ DWORD dwDebugAppCookie,
    /* [in] */ DWORD dwAppCookie);


void __RPC_STUB IMachineDebugManagerCookie_RemoveApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMachineDebugManagerCookie_EnumApplications_Proxy( 
    IMachineDebugManagerCookie __RPC_FAR * This,
    /* [out] */ IEnumRemoteDebugApplications __RPC_FAR *__RPC_FAR *ppeda);


void __RPC_STUB IMachineDebugManagerCookie_EnumApplications_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMachineDebugManagerCookie_INTERFACE_DEFINED__ */


#ifndef __IMachineDebugManagerEvents_INTERFACE_DEFINED__
#define __IMachineDebugManagerEvents_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMachineDebugManagerEvents
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IMachineDebugManagerEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C2e-CB0C-11d0-B5C9-00A0244A0E7A")
    IMachineDebugManagerEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE onAddApplication( 
            /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
            /* [in] */ DWORD dwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onRemoveApplication( 
            /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
            /* [in] */ DWORD dwAppCookie) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMachineDebugManagerEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMachineDebugManagerEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMachineDebugManagerEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMachineDebugManagerEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onAddApplication )( 
            IMachineDebugManagerEvents __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
            /* [in] */ DWORD dwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onRemoveApplication )( 
            IMachineDebugManagerEvents __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
            /* [in] */ DWORD dwAppCookie);
        
        END_INTERFACE
    } IMachineDebugManagerEventsVtbl;

    interface IMachineDebugManagerEvents
    {
        CONST_VTBL struct IMachineDebugManagerEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMachineDebugManagerEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMachineDebugManagerEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMachineDebugManagerEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMachineDebugManagerEvents_onAddApplication(This,pda,dwAppCookie)	\
    (This)->lpVtbl -> onAddApplication(This,pda,dwAppCookie)

#define IMachineDebugManagerEvents_onRemoveApplication(This,pda,dwAppCookie)	\
    (This)->lpVtbl -> onRemoveApplication(This,pda,dwAppCookie)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMachineDebugManagerEvents_onAddApplication_Proxy( 
    IMachineDebugManagerEvents __RPC_FAR * This,
    /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
    /* [in] */ DWORD dwAppCookie);


void __RPC_STUB IMachineDebugManagerEvents_onAddApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMachineDebugManagerEvents_onRemoveApplication_Proxy( 
    IMachineDebugManagerEvents __RPC_FAR * This,
    /* [in] */ IRemoteDebugApplication __RPC_FAR *pda,
    /* [in] */ DWORD dwAppCookie);


void __RPC_STUB IMachineDebugManagerEvents_onRemoveApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMachineDebugManagerEvents_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL_itf_activdbg_0194
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local] */ 


EXTERN_C const CLSID CLSID_ProcessDebugManager;


extern RPC_IF_HANDLE __MIDL_itf_activdbg_0194_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0194_v0_0_s_ifspec;

#ifndef __IProcessDebugManager_INTERFACE_DEFINED__
#define __IProcessDebugManager_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IProcessDebugManager
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IProcessDebugManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C2f-CB0C-11d0-B5C9-00A0244A0E7A")
    IProcessDebugManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateApplication( 
            /* [out] */ IDebugApplication __RPC_FAR *__RPC_FAR *ppda) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultApplication( 
            /* [out] */ IDebugApplication __RPC_FAR *__RPC_FAR *ppda) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddApplication( 
            /* [in] */ IDebugApplication __RPC_FAR *pda,
            /* [out] */ DWORD __RPC_FAR *pdwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveApplication( 
            /* [in] */ DWORD dwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateDebugDocumentHelper( 
            /* [in] */ IUnknown __RPC_FAR *punkOuter,
            /* [out] */ IDebugDocumentHelper __RPC_FAR *__RPC_FAR *pddh) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IProcessDebugManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IProcessDebugManager __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IProcessDebugManager __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IProcessDebugManager __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateApplication )( 
            IProcessDebugManager __RPC_FAR * This,
            /* [out] */ IDebugApplication __RPC_FAR *__RPC_FAR *ppda);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDefaultApplication )( 
            IProcessDebugManager __RPC_FAR * This,
            /* [out] */ IDebugApplication __RPC_FAR *__RPC_FAR *ppda);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddApplication )( 
            IProcessDebugManager __RPC_FAR * This,
            /* [in] */ IDebugApplication __RPC_FAR *pda,
            /* [out] */ DWORD __RPC_FAR *pdwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveApplication )( 
            IProcessDebugManager __RPC_FAR * This,
            /* [in] */ DWORD dwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateDebugDocumentHelper )( 
            IProcessDebugManager __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *punkOuter,
            /* [out] */ IDebugDocumentHelper __RPC_FAR *__RPC_FAR *pddh);
        
        END_INTERFACE
    } IProcessDebugManagerVtbl;

    interface IProcessDebugManager
    {
        CONST_VTBL struct IProcessDebugManagerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProcessDebugManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProcessDebugManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProcessDebugManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProcessDebugManager_CreateApplication(This,ppda)	\
    (This)->lpVtbl -> CreateApplication(This,ppda)

#define IProcessDebugManager_GetDefaultApplication(This,ppda)	\
    (This)->lpVtbl -> GetDefaultApplication(This,ppda)

#define IProcessDebugManager_AddApplication(This,pda,pdwAppCookie)	\
    (This)->lpVtbl -> AddApplication(This,pda,pdwAppCookie)

#define IProcessDebugManager_RemoveApplication(This,dwAppCookie)	\
    (This)->lpVtbl -> RemoveApplication(This,dwAppCookie)

#define IProcessDebugManager_CreateDebugDocumentHelper(This,punkOuter,pddh)	\
    (This)->lpVtbl -> CreateDebugDocumentHelper(This,punkOuter,pddh)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IProcessDebugManager_CreateApplication_Proxy( 
    IProcessDebugManager __RPC_FAR * This,
    /* [out] */ IDebugApplication __RPC_FAR *__RPC_FAR *ppda);


void __RPC_STUB IProcessDebugManager_CreateApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProcessDebugManager_GetDefaultApplication_Proxy( 
    IProcessDebugManager __RPC_FAR * This,
    /* [out] */ IDebugApplication __RPC_FAR *__RPC_FAR *ppda);


void __RPC_STUB IProcessDebugManager_GetDefaultApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProcessDebugManager_AddApplication_Proxy( 
    IProcessDebugManager __RPC_FAR * This,
    /* [in] */ IDebugApplication __RPC_FAR *pda,
    /* [out] */ DWORD __RPC_FAR *pdwAppCookie);


void __RPC_STUB IProcessDebugManager_AddApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProcessDebugManager_RemoveApplication_Proxy( 
    IProcessDebugManager __RPC_FAR * This,
    /* [in] */ DWORD dwAppCookie);


void __RPC_STUB IProcessDebugManager_RemoveApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProcessDebugManager_CreateDebugDocumentHelper_Proxy( 
    IProcessDebugManager __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *punkOuter,
    /* [out] */ IDebugDocumentHelper __RPC_FAR *__RPC_FAR *pddh);


void __RPC_STUB IProcessDebugManager_CreateDebugDocumentHelper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IProcessDebugManager_INTERFACE_DEFINED__ */


#ifndef __IRemoteDebugApplication_INTERFACE_DEFINED__
#define __IRemoteDebugApplication_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRemoteDebugApplication
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IRemoteDebugApplication;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C30-CB0C-11d0-B5C9-00A0244A0E7A")
    IRemoteDebugApplication : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ResumeFromBreakPoint( 
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prptFocus,
            /* [in] */ BREAKRESUMEACTION bra,
            /* [in] */ ERRORRESUMEACTION era) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CauseBreak( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectDebugger( 
            /* [in] */ IApplicationDebugger __RPC_FAR *pad) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisconnectDebugger( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDebugger( 
            /* [out] */ IApplicationDebugger __RPC_FAR *__RPC_FAR *pad) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateInstanceAtApplication( 
            /* [in] */ REFCLSID rclsid,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ DWORD dwClsContext,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryAlive( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumThreads( 
            /* [out] */ IEnumRemoteDebugApplicationThreads __RPC_FAR *__RPC_FAR *pperdat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
            /* [out] */ BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRootNode( 
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdanRoot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumGlobalExpressionContexts( 
            /* [out] */ IEnumDebugExpressionContexts __RPC_FAR *__RPC_FAR *ppedec) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRemoteDebugApplicationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRemoteDebugApplication __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRemoteDebugApplication __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRemoteDebugApplication __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResumeFromBreakPoint )( 
            IRemoteDebugApplication __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prptFocus,
            /* [in] */ BREAKRESUMEACTION bra,
            /* [in] */ ERRORRESUMEACTION era);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CauseBreak )( 
            IRemoteDebugApplication __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ConnectDebugger )( 
            IRemoteDebugApplication __RPC_FAR * This,
            /* [in] */ IApplicationDebugger __RPC_FAR *pad);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DisconnectDebugger )( 
            IRemoteDebugApplication __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDebugger )( 
            IRemoteDebugApplication __RPC_FAR * This,
            /* [out] */ IApplicationDebugger __RPC_FAR *__RPC_FAR *pad);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateInstanceAtApplication )( 
            IRemoteDebugApplication __RPC_FAR * This,
            /* [in] */ REFCLSID rclsid,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ DWORD dwClsContext,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryAlive )( 
            IRemoteDebugApplication __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumThreads )( 
            IRemoteDebugApplication __RPC_FAR * This,
            /* [out] */ IEnumRemoteDebugApplicationThreads __RPC_FAR *__RPC_FAR *pperdat);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            IRemoteDebugApplication __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRootNode )( 
            IRemoteDebugApplication __RPC_FAR * This,
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdanRoot);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumGlobalExpressionContexts )( 
            IRemoteDebugApplication __RPC_FAR * This,
            /* [out] */ IEnumDebugExpressionContexts __RPC_FAR *__RPC_FAR *ppedec);
        
        END_INTERFACE
    } IRemoteDebugApplicationVtbl;

    interface IRemoteDebugApplication
    {
        CONST_VTBL struct IRemoteDebugApplicationVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteDebugApplication_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteDebugApplication_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteDebugApplication_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteDebugApplication_ResumeFromBreakPoint(This,prptFocus,bra,era)	\
    (This)->lpVtbl -> ResumeFromBreakPoint(This,prptFocus,bra,era)

#define IRemoteDebugApplication_CauseBreak(This)	\
    (This)->lpVtbl -> CauseBreak(This)

#define IRemoteDebugApplication_ConnectDebugger(This,pad)	\
    (This)->lpVtbl -> ConnectDebugger(This,pad)

#define IRemoteDebugApplication_DisconnectDebugger(This)	\
    (This)->lpVtbl -> DisconnectDebugger(This)

#define IRemoteDebugApplication_GetDebugger(This,pad)	\
    (This)->lpVtbl -> GetDebugger(This,pad)

#define IRemoteDebugApplication_CreateInstanceAtApplication(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)	\
    (This)->lpVtbl -> CreateInstanceAtApplication(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)

#define IRemoteDebugApplication_QueryAlive(This)	\
    (This)->lpVtbl -> QueryAlive(This)

#define IRemoteDebugApplication_EnumThreads(This,pperdat)	\
    (This)->lpVtbl -> EnumThreads(This,pperdat)

#define IRemoteDebugApplication_GetName(This,pbstrName)	\
    (This)->lpVtbl -> GetName(This,pbstrName)

#define IRemoteDebugApplication_GetRootNode(This,ppdanRoot)	\
    (This)->lpVtbl -> GetRootNode(This,ppdanRoot)

#define IRemoteDebugApplication_EnumGlobalExpressionContexts(This,ppedec)	\
    (This)->lpVtbl -> EnumGlobalExpressionContexts(This,ppedec)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_ResumeFromBreakPoint_Proxy( 
    IRemoteDebugApplication __RPC_FAR * This,
    /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prptFocus,
    /* [in] */ BREAKRESUMEACTION bra,
    /* [in] */ ERRORRESUMEACTION era);


void __RPC_STUB IRemoteDebugApplication_ResumeFromBreakPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_CauseBreak_Proxy( 
    IRemoteDebugApplication __RPC_FAR * This);


void __RPC_STUB IRemoteDebugApplication_CauseBreak_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_ConnectDebugger_Proxy( 
    IRemoteDebugApplication __RPC_FAR * This,
    /* [in] */ IApplicationDebugger __RPC_FAR *pad);


void __RPC_STUB IRemoteDebugApplication_ConnectDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_DisconnectDebugger_Proxy( 
    IRemoteDebugApplication __RPC_FAR * This);


void __RPC_STUB IRemoteDebugApplication_DisconnectDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_GetDebugger_Proxy( 
    IRemoteDebugApplication __RPC_FAR * This,
    /* [out] */ IApplicationDebugger __RPC_FAR *__RPC_FAR *pad);


void __RPC_STUB IRemoteDebugApplication_GetDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_CreateInstanceAtApplication_Proxy( 
    IRemoteDebugApplication __RPC_FAR * This,
    /* [in] */ REFCLSID rclsid,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [in] */ DWORD dwClsContext,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObject);


void __RPC_STUB IRemoteDebugApplication_CreateInstanceAtApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_QueryAlive_Proxy( 
    IRemoteDebugApplication __RPC_FAR * This);


void __RPC_STUB IRemoteDebugApplication_QueryAlive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_EnumThreads_Proxy( 
    IRemoteDebugApplication __RPC_FAR * This,
    /* [out] */ IEnumRemoteDebugApplicationThreads __RPC_FAR *__RPC_FAR *pperdat);


void __RPC_STUB IRemoteDebugApplication_EnumThreads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_GetName_Proxy( 
    IRemoteDebugApplication __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IRemoteDebugApplication_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_GetRootNode_Proxy( 
    IRemoteDebugApplication __RPC_FAR * This,
    /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdanRoot);


void __RPC_STUB IRemoteDebugApplication_GetRootNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_EnumGlobalExpressionContexts_Proxy( 
    IRemoteDebugApplication __RPC_FAR * This,
    /* [out] */ IEnumDebugExpressionContexts __RPC_FAR *__RPC_FAR *ppedec);


void __RPC_STUB IRemoteDebugApplication_EnumGlobalExpressionContexts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRemoteDebugApplication_INTERFACE_DEFINED__ */


#ifndef __IDebugApplication_INTERFACE_DEFINED__
#define __IDebugApplication_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugApplication
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IDebugApplication;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C32-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugApplication : public IRemoteDebugApplication
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetName( 
            /* [in] */ LPCOLESTR pstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StepOutComplete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DebugOutput( 
            /* [in] */ LPCOLESTR pstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StartDebugSession( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleBreakPoint( 
            /* [in] */ BREAKREASON br,
            /* [out] */ BREAKRESUMEACTION __RPC_FAR *pbra) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBreakFlags( 
            /* [out] */ APPBREAKFLAGS __RPC_FAR *pabf,
            /* [out] */ IRemoteDebugApplicationThread __RPC_FAR *__RPC_FAR *pprdatSteppingThread) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentThread( 
            /* [out] */ IDebugApplicationThread __RPC_FAR *__RPC_FAR *pat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateAsyncDebugOperation( 
            /* [in] */ IDebugSyncOperation __RPC_FAR *psdo,
            /* [out] */ IDebugAsyncOperation __RPC_FAR *__RPC_FAR *ppado) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddStackFrameSniffer( 
            /* [in] */ IDebugStackFrameSniffer __RPC_FAR *pdsfs,
            /* [out] */ DWORD __RPC_FAR *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveStackFrameSniffer( 
            /* [in] */ DWORD dwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryCurrentThreadIsDebuggerThread( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SynchronousCallInDebuggerThread( 
            /* [in] */ IDebugThreadCall __RPC_FAR *pptc,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2,
            /* [in] */ DWORD dwParam3) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateApplicationNode( 
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdanNew) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FireDebuggerEvent( 
            /* [in] */ REFGUID riid,
            /* [in] */ IUnknown __RPC_FAR *punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleRuntimeError( 
            /* [in] */ IActiveScriptErrorDebug __RPC_FAR *pErrorDebug,
            /* [in] */ IActiveScriptSite __RPC_FAR *pScriptSite,
            /* [out] */ BREAKRESUMEACTION __RPC_FAR *pbra,
            /* [out] */ ERRORRESUMEACTION __RPC_FAR *perra,
            /* [out] */ BOOL __RPC_FAR *pfCallOnScriptError) = 0;
        
        virtual BOOL STDMETHODCALLTYPE FCanJitDebug( void) = 0;
        
        virtual BOOL STDMETHODCALLTYPE FIsAutoJitDebugEnabled( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddGlobalExpressionContextProvider( 
            /* [in] */ IProvideExpressionContexts __RPC_FAR *pdsfs,
            /* [out] */ DWORD __RPC_FAR *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveGlobalExpressionContextProvider( 
            /* [in] */ DWORD dwCookie) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugApplicationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugApplication __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugApplication __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResumeFromBreakPoint )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prptFocus,
            /* [in] */ BREAKRESUMEACTION bra,
            /* [in] */ ERRORRESUMEACTION era);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CauseBreak )( 
            IDebugApplication __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ConnectDebugger )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ IApplicationDebugger __RPC_FAR *pad);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DisconnectDebugger )( 
            IDebugApplication __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDebugger )( 
            IDebugApplication __RPC_FAR * This,
            /* [out] */ IApplicationDebugger __RPC_FAR *__RPC_FAR *pad);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateInstanceAtApplication )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ REFCLSID rclsid,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ DWORD dwClsContext,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryAlive )( 
            IDebugApplication __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumThreads )( 
            IDebugApplication __RPC_FAR * This,
            /* [out] */ IEnumRemoteDebugApplicationThreads __RPC_FAR *__RPC_FAR *pperdat);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            IDebugApplication __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRootNode )( 
            IDebugApplication __RPC_FAR * This,
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdanRoot);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumGlobalExpressionContexts )( 
            IDebugApplication __RPC_FAR * This,
            /* [out] */ IEnumDebugExpressionContexts __RPC_FAR *__RPC_FAR *ppedec);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetName )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ LPCOLESTR pstrName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StepOutComplete )( 
            IDebugApplication __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DebugOutput )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ LPCOLESTR pstr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StartDebugSession )( 
            IDebugApplication __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HandleBreakPoint )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ BREAKREASON br,
            /* [out] */ BREAKRESUMEACTION __RPC_FAR *pbra);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IDebugApplication __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBreakFlags )( 
            IDebugApplication __RPC_FAR * This,
            /* [out] */ APPBREAKFLAGS __RPC_FAR *pabf,
            /* [out] */ IRemoteDebugApplicationThread __RPC_FAR *__RPC_FAR *pprdatSteppingThread);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrentThread )( 
            IDebugApplication __RPC_FAR * This,
            /* [out] */ IDebugApplicationThread __RPC_FAR *__RPC_FAR *pat);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateAsyncDebugOperation )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ IDebugSyncOperation __RPC_FAR *psdo,
            /* [out] */ IDebugAsyncOperation __RPC_FAR *__RPC_FAR *ppado);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddStackFrameSniffer )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ IDebugStackFrameSniffer __RPC_FAR *pdsfs,
            /* [out] */ DWORD __RPC_FAR *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveStackFrameSniffer )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ DWORD dwCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryCurrentThreadIsDebuggerThread )( 
            IDebugApplication __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SynchronousCallInDebuggerThread )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ IDebugThreadCall __RPC_FAR *pptc,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2,
            /* [in] */ DWORD dwParam3);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateApplicationNode )( 
            IDebugApplication __RPC_FAR * This,
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdanNew);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FireDebuggerEvent )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ REFGUID riid,
            /* [in] */ IUnknown __RPC_FAR *punk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HandleRuntimeError )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ IActiveScriptErrorDebug __RPC_FAR *pErrorDebug,
            /* [in] */ IActiveScriptSite __RPC_FAR *pScriptSite,
            /* [out] */ BREAKRESUMEACTION __RPC_FAR *pbra,
            /* [out] */ ERRORRESUMEACTION __RPC_FAR *perra,
            /* [out] */ BOOL __RPC_FAR *pfCallOnScriptError);
        
        BOOL ( STDMETHODCALLTYPE __RPC_FAR *FCanJitDebug )( 
            IDebugApplication __RPC_FAR * This);
        
        BOOL ( STDMETHODCALLTYPE __RPC_FAR *FIsAutoJitDebugEnabled )( 
            IDebugApplication __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddGlobalExpressionContextProvider )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ IProvideExpressionContexts __RPC_FAR *pdsfs,
            /* [out] */ DWORD __RPC_FAR *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveGlobalExpressionContextProvider )( 
            IDebugApplication __RPC_FAR * This,
            /* [in] */ DWORD dwCookie);
        
        END_INTERFACE
    } IDebugApplicationVtbl;

    interface IDebugApplication
    {
        CONST_VTBL struct IDebugApplicationVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugApplication_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugApplication_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugApplication_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugApplication_ResumeFromBreakPoint(This,prptFocus,bra,era)	\
    (This)->lpVtbl -> ResumeFromBreakPoint(This,prptFocus,bra,era)

#define IDebugApplication_CauseBreak(This)	\
    (This)->lpVtbl -> CauseBreak(This)

#define IDebugApplication_ConnectDebugger(This,pad)	\
    (This)->lpVtbl -> ConnectDebugger(This,pad)

#define IDebugApplication_DisconnectDebugger(This)	\
    (This)->lpVtbl -> DisconnectDebugger(This)

#define IDebugApplication_GetDebugger(This,pad)	\
    (This)->lpVtbl -> GetDebugger(This,pad)

#define IDebugApplication_CreateInstanceAtApplication(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)	\
    (This)->lpVtbl -> CreateInstanceAtApplication(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)

#define IDebugApplication_QueryAlive(This)	\
    (This)->lpVtbl -> QueryAlive(This)

#define IDebugApplication_EnumThreads(This,pperdat)	\
    (This)->lpVtbl -> EnumThreads(This,pperdat)

#define IDebugApplication_GetName(This,pbstrName)	\
    (This)->lpVtbl -> GetName(This,pbstrName)

#define IDebugApplication_GetRootNode(This,ppdanRoot)	\
    (This)->lpVtbl -> GetRootNode(This,ppdanRoot)

#define IDebugApplication_EnumGlobalExpressionContexts(This,ppedec)	\
    (This)->lpVtbl -> EnumGlobalExpressionContexts(This,ppedec)


#define IDebugApplication_SetName(This,pstrName)	\
    (This)->lpVtbl -> SetName(This,pstrName)

#define IDebugApplication_StepOutComplete(This)	\
    (This)->lpVtbl -> StepOutComplete(This)

#define IDebugApplication_DebugOutput(This,pstr)	\
    (This)->lpVtbl -> DebugOutput(This,pstr)

#define IDebugApplication_StartDebugSession(This)	\
    (This)->lpVtbl -> StartDebugSession(This)

#define IDebugApplication_HandleBreakPoint(This,br,pbra)	\
    (This)->lpVtbl -> HandleBreakPoint(This,br,pbra)

#define IDebugApplication_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IDebugApplication_GetBreakFlags(This,pabf,pprdatSteppingThread)	\
    (This)->lpVtbl -> GetBreakFlags(This,pabf,pprdatSteppingThread)

#define IDebugApplication_GetCurrentThread(This,pat)	\
    (This)->lpVtbl -> GetCurrentThread(This,pat)

#define IDebugApplication_CreateAsyncDebugOperation(This,psdo,ppado)	\
    (This)->lpVtbl -> CreateAsyncDebugOperation(This,psdo,ppado)

#define IDebugApplication_AddStackFrameSniffer(This,pdsfs,pdwCookie)	\
    (This)->lpVtbl -> AddStackFrameSniffer(This,pdsfs,pdwCookie)

#define IDebugApplication_RemoveStackFrameSniffer(This,dwCookie)	\
    (This)->lpVtbl -> RemoveStackFrameSniffer(This,dwCookie)

#define IDebugApplication_QueryCurrentThreadIsDebuggerThread(This)	\
    (This)->lpVtbl -> QueryCurrentThreadIsDebuggerThread(This)

#define IDebugApplication_SynchronousCallInDebuggerThread(This,pptc,dwParam1,dwParam2,dwParam3)	\
    (This)->lpVtbl -> SynchronousCallInDebuggerThread(This,pptc,dwParam1,dwParam2,dwParam3)

#define IDebugApplication_CreateApplicationNode(This,ppdanNew)	\
    (This)->lpVtbl -> CreateApplicationNode(This,ppdanNew)

#define IDebugApplication_FireDebuggerEvent(This,riid,punk)	\
    (This)->lpVtbl -> FireDebuggerEvent(This,riid,punk)

#define IDebugApplication_HandleRuntimeError(This,pErrorDebug,pScriptSite,pbra,perra,pfCallOnScriptError)	\
    (This)->lpVtbl -> HandleRuntimeError(This,pErrorDebug,pScriptSite,pbra,perra,pfCallOnScriptError)

#define IDebugApplication_FCanJitDebug(This)	\
    (This)->lpVtbl -> FCanJitDebug(This)

#define IDebugApplication_FIsAutoJitDebugEnabled(This)	\
    (This)->lpVtbl -> FIsAutoJitDebugEnabled(This)

#define IDebugApplication_AddGlobalExpressionContextProvider(This,pdsfs,pdwCookie)	\
    (This)->lpVtbl -> AddGlobalExpressionContextProvider(This,pdsfs,pdwCookie)

#define IDebugApplication_RemoveGlobalExpressionContextProvider(This,dwCookie)	\
    (This)->lpVtbl -> RemoveGlobalExpressionContextProvider(This,dwCookie)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugApplication_SetName_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [in] */ LPCOLESTR pstrName);


void __RPC_STUB IDebugApplication_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_StepOutComplete_Proxy( 
    IDebugApplication __RPC_FAR * This);


void __RPC_STUB IDebugApplication_StepOutComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_DebugOutput_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [in] */ LPCOLESTR pstr);


void __RPC_STUB IDebugApplication_DebugOutput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_StartDebugSession_Proxy( 
    IDebugApplication __RPC_FAR * This);


void __RPC_STUB IDebugApplication_StartDebugSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_HandleBreakPoint_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [in] */ BREAKREASON br,
    /* [out] */ BREAKRESUMEACTION __RPC_FAR *pbra);


void __RPC_STUB IDebugApplication_HandleBreakPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_Close_Proxy( 
    IDebugApplication __RPC_FAR * This);


void __RPC_STUB IDebugApplication_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_GetBreakFlags_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [out] */ APPBREAKFLAGS __RPC_FAR *pabf,
    /* [out] */ IRemoteDebugApplicationThread __RPC_FAR *__RPC_FAR *pprdatSteppingThread);


void __RPC_STUB IDebugApplication_GetBreakFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_GetCurrentThread_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [out] */ IDebugApplicationThread __RPC_FAR *__RPC_FAR *pat);


void __RPC_STUB IDebugApplication_GetCurrentThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_CreateAsyncDebugOperation_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [in] */ IDebugSyncOperation __RPC_FAR *psdo,
    /* [out] */ IDebugAsyncOperation __RPC_FAR *__RPC_FAR *ppado);


void __RPC_STUB IDebugApplication_CreateAsyncDebugOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_AddStackFrameSniffer_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [in] */ IDebugStackFrameSniffer __RPC_FAR *pdsfs,
    /* [out] */ DWORD __RPC_FAR *pdwCookie);


void __RPC_STUB IDebugApplication_AddStackFrameSniffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_RemoveStackFrameSniffer_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [in] */ DWORD dwCookie);


void __RPC_STUB IDebugApplication_RemoveStackFrameSniffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_QueryCurrentThreadIsDebuggerThread_Proxy( 
    IDebugApplication __RPC_FAR * This);


void __RPC_STUB IDebugApplication_QueryCurrentThreadIsDebuggerThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_SynchronousCallInDebuggerThread_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [in] */ IDebugThreadCall __RPC_FAR *pptc,
    /* [in] */ DWORD dwParam1,
    /* [in] */ DWORD dwParam2,
    /* [in] */ DWORD dwParam3);


void __RPC_STUB IDebugApplication_SynchronousCallInDebuggerThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_CreateApplicationNode_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdanNew);


void __RPC_STUB IDebugApplication_CreateApplicationNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_FireDebuggerEvent_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [in] */ REFGUID riid,
    /* [in] */ IUnknown __RPC_FAR *punk);


void __RPC_STUB IDebugApplication_FireDebuggerEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_HandleRuntimeError_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [in] */ IActiveScriptErrorDebug __RPC_FAR *pErrorDebug,
    /* [in] */ IActiveScriptSite __RPC_FAR *pScriptSite,
    /* [out] */ BREAKRESUMEACTION __RPC_FAR *pbra,
    /* [out] */ ERRORRESUMEACTION __RPC_FAR *perra,
    /* [out] */ BOOL __RPC_FAR *pfCallOnScriptError);


void __RPC_STUB IDebugApplication_HandleRuntimeError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IDebugApplication_FCanJitDebug_Proxy( 
    IDebugApplication __RPC_FAR * This);


void __RPC_STUB IDebugApplication_FCanJitDebug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IDebugApplication_FIsAutoJitDebugEnabled_Proxy( 
    IDebugApplication __RPC_FAR * This);


void __RPC_STUB IDebugApplication_FIsAutoJitDebugEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_AddGlobalExpressionContextProvider_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [in] */ IProvideExpressionContexts __RPC_FAR *pdsfs,
    /* [out] */ DWORD __RPC_FAR *pdwCookie);


void __RPC_STUB IDebugApplication_AddGlobalExpressionContextProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication_RemoveGlobalExpressionContextProvider_Proxy( 
    IDebugApplication __RPC_FAR * This,
    /* [in] */ DWORD dwCookie);


void __RPC_STUB IDebugApplication_RemoveGlobalExpressionContextProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugApplication_INTERFACE_DEFINED__ */


#ifndef __IRemoteDebugApplicationEvents_INTERFACE_DEFINED__
#define __IRemoteDebugApplicationEvents_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRemoteDebugApplicationEvents
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IRemoteDebugApplicationEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C33-CB0C-11d0-B5C9-00A0244A0E7A")
    IRemoteDebugApplicationEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnConnectDebugger( 
            /* [in] */ IApplicationDebugger __RPC_FAR *pad) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDisconnectDebugger( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnSetName( 
            /* [in] */ LPCOLESTR pstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDebugOutput( 
            /* [in] */ LPCOLESTR pstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnClose( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnEnterBreakPoint( 
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnLeaveBreakPoint( 
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCreateThread( 
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDestroyThread( 
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnBreakFlagChange( 
            /* [in] */ APPBREAKFLAGS abf,
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdatSteppingThread) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRemoteDebugApplicationEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRemoteDebugApplicationEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRemoteDebugApplicationEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRemoteDebugApplicationEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnConnectDebugger )( 
            IRemoteDebugApplicationEvents __RPC_FAR * This,
            /* [in] */ IApplicationDebugger __RPC_FAR *pad);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnDisconnectDebugger )( 
            IRemoteDebugApplicationEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnSetName )( 
            IRemoteDebugApplicationEvents __RPC_FAR * This,
            /* [in] */ LPCOLESTR pstrName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnDebugOutput )( 
            IRemoteDebugApplicationEvents __RPC_FAR * This,
            /* [in] */ LPCOLESTR pstr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnClose )( 
            IRemoteDebugApplicationEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnEnterBreakPoint )( 
            IRemoteDebugApplicationEvents __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdat);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnLeaveBreakPoint )( 
            IRemoteDebugApplicationEvents __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdat);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnCreateThread )( 
            IRemoteDebugApplicationEvents __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdat);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnDestroyThread )( 
            IRemoteDebugApplicationEvents __RPC_FAR * This,
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdat);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnBreakFlagChange )( 
            IRemoteDebugApplicationEvents __RPC_FAR * This,
            /* [in] */ APPBREAKFLAGS abf,
            /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdatSteppingThread);
        
        END_INTERFACE
    } IRemoteDebugApplicationEventsVtbl;

    interface IRemoteDebugApplicationEvents
    {
        CONST_VTBL struct IRemoteDebugApplicationEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteDebugApplicationEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteDebugApplicationEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteDebugApplicationEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteDebugApplicationEvents_OnConnectDebugger(This,pad)	\
    (This)->lpVtbl -> OnConnectDebugger(This,pad)

#define IRemoteDebugApplicationEvents_OnDisconnectDebugger(This)	\
    (This)->lpVtbl -> OnDisconnectDebugger(This)

#define IRemoteDebugApplicationEvents_OnSetName(This,pstrName)	\
    (This)->lpVtbl -> OnSetName(This,pstrName)

#define IRemoteDebugApplicationEvents_OnDebugOutput(This,pstr)	\
    (This)->lpVtbl -> OnDebugOutput(This,pstr)

#define IRemoteDebugApplicationEvents_OnClose(This)	\
    (This)->lpVtbl -> OnClose(This)

#define IRemoteDebugApplicationEvents_OnEnterBreakPoint(This,prdat)	\
    (This)->lpVtbl -> OnEnterBreakPoint(This,prdat)

#define IRemoteDebugApplicationEvents_OnLeaveBreakPoint(This,prdat)	\
    (This)->lpVtbl -> OnLeaveBreakPoint(This,prdat)

#define IRemoteDebugApplicationEvents_OnCreateThread(This,prdat)	\
    (This)->lpVtbl -> OnCreateThread(This,prdat)

#define IRemoteDebugApplicationEvents_OnDestroyThread(This,prdat)	\
    (This)->lpVtbl -> OnDestroyThread(This,prdat)

#define IRemoteDebugApplicationEvents_OnBreakFlagChange(This,abf,prdatSteppingThread)	\
    (This)->lpVtbl -> OnBreakFlagChange(This,abf,prdatSteppingThread)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnConnectDebugger_Proxy( 
    IRemoteDebugApplicationEvents __RPC_FAR * This,
    /* [in] */ IApplicationDebugger __RPC_FAR *pad);


void __RPC_STUB IRemoteDebugApplicationEvents_OnConnectDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnDisconnectDebugger_Proxy( 
    IRemoteDebugApplicationEvents __RPC_FAR * This);


void __RPC_STUB IRemoteDebugApplicationEvents_OnDisconnectDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnSetName_Proxy( 
    IRemoteDebugApplicationEvents __RPC_FAR * This,
    /* [in] */ LPCOLESTR pstrName);


void __RPC_STUB IRemoteDebugApplicationEvents_OnSetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnDebugOutput_Proxy( 
    IRemoteDebugApplicationEvents __RPC_FAR * This,
    /* [in] */ LPCOLESTR pstr);


void __RPC_STUB IRemoteDebugApplicationEvents_OnDebugOutput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnClose_Proxy( 
    IRemoteDebugApplicationEvents __RPC_FAR * This);


void __RPC_STUB IRemoteDebugApplicationEvents_OnClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnEnterBreakPoint_Proxy( 
    IRemoteDebugApplicationEvents __RPC_FAR * This,
    /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdat);


void __RPC_STUB IRemoteDebugApplicationEvents_OnEnterBreakPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnLeaveBreakPoint_Proxy( 
    IRemoteDebugApplicationEvents __RPC_FAR * This,
    /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdat);


void __RPC_STUB IRemoteDebugApplicationEvents_OnLeaveBreakPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnCreateThread_Proxy( 
    IRemoteDebugApplicationEvents __RPC_FAR * This,
    /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdat);


void __RPC_STUB IRemoteDebugApplicationEvents_OnCreateThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnDestroyThread_Proxy( 
    IRemoteDebugApplicationEvents __RPC_FAR * This,
    /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdat);


void __RPC_STUB IRemoteDebugApplicationEvents_OnDestroyThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnBreakFlagChange_Proxy( 
    IRemoteDebugApplicationEvents __RPC_FAR * This,
    /* [in] */ APPBREAKFLAGS abf,
    /* [in] */ IRemoteDebugApplicationThread __RPC_FAR *prdatSteppingThread);


void __RPC_STUB IRemoteDebugApplicationEvents_OnBreakFlagChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRemoteDebugApplicationEvents_INTERFACE_DEFINED__ */


#ifndef __IDebugApplicationNode_INTERFACE_DEFINED__
#define __IDebugApplicationNode_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugApplicationNode
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugApplicationNode;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C34-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugApplicationNode : public IDebugDocumentProvider
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumChildren( 
            /* [out] */ IEnumDebugApplicationNodes __RPC_FAR *__RPC_FAR *pperddp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParent( 
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *pprddp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDocumentProvider( 
            /* [in] */ IDebugDocumentProvider __RPC_FAR *pddp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Attach( 
            /* [in] */ IDebugApplicationNode __RPC_FAR *pdanParent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Detach( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugApplicationNodeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugApplicationNode __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugApplicationNode __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugApplicationNode __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            IDebugApplicationNode __RPC_FAR * This,
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR __RPC_FAR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocumentClassId )( 
            IDebugApplicationNode __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pclsidDocument);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDocument )( 
            IDebugApplicationNode __RPC_FAR * This,
            /* [out] */ IDebugDocument __RPC_FAR *__RPC_FAR *ppssd);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumChildren )( 
            IDebugApplicationNode __RPC_FAR * This,
            /* [out] */ IEnumDebugApplicationNodes __RPC_FAR *__RPC_FAR *pperddp);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetParent )( 
            IDebugApplicationNode __RPC_FAR * This,
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *pprddp);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDocumentProvider )( 
            IDebugApplicationNode __RPC_FAR * This,
            /* [in] */ IDebugDocumentProvider __RPC_FAR *pddp);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IDebugApplicationNode __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Attach )( 
            IDebugApplicationNode __RPC_FAR * This,
            /* [in] */ IDebugApplicationNode __RPC_FAR *pdanParent);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Detach )( 
            IDebugApplicationNode __RPC_FAR * This);
        
        END_INTERFACE
    } IDebugApplicationNodeVtbl;

    interface IDebugApplicationNode
    {
        CONST_VTBL struct IDebugApplicationNodeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugApplicationNode_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugApplicationNode_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugApplicationNode_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugApplicationNode_GetName(This,dnt,pbstrName)	\
    (This)->lpVtbl -> GetName(This,dnt,pbstrName)

#define IDebugApplicationNode_GetDocumentClassId(This,pclsidDocument)	\
    (This)->lpVtbl -> GetDocumentClassId(This,pclsidDocument)


#define IDebugApplicationNode_GetDocument(This,ppssd)	\
    (This)->lpVtbl -> GetDocument(This,ppssd)


#define IDebugApplicationNode_EnumChildren(This,pperddp)	\
    (This)->lpVtbl -> EnumChildren(This,pperddp)

#define IDebugApplicationNode_GetParent(This,pprddp)	\
    (This)->lpVtbl -> GetParent(This,pprddp)

#define IDebugApplicationNode_SetDocumentProvider(This,pddp)	\
    (This)->lpVtbl -> SetDocumentProvider(This,pddp)

#define IDebugApplicationNode_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IDebugApplicationNode_Attach(This,pdanParent)	\
    (This)->lpVtbl -> Attach(This,pdanParent)

#define IDebugApplicationNode_Detach(This)	\
    (This)->lpVtbl -> Detach(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugApplicationNode_EnumChildren_Proxy( 
    IDebugApplicationNode __RPC_FAR * This,
    /* [out] */ IEnumDebugApplicationNodes __RPC_FAR *__RPC_FAR *pperddp);


void __RPC_STUB IDebugApplicationNode_EnumChildren_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNode_GetParent_Proxy( 
    IDebugApplicationNode __RPC_FAR * This,
    /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *pprddp);


void __RPC_STUB IDebugApplicationNode_GetParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNode_SetDocumentProvider_Proxy( 
    IDebugApplicationNode __RPC_FAR * This,
    /* [in] */ IDebugDocumentProvider __RPC_FAR *pddp);


void __RPC_STUB IDebugApplicationNode_SetDocumentProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNode_Close_Proxy( 
    IDebugApplicationNode __RPC_FAR * This);


void __RPC_STUB IDebugApplicationNode_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNode_Attach_Proxy( 
    IDebugApplicationNode __RPC_FAR * This,
    /* [in] */ IDebugApplicationNode __RPC_FAR *pdanParent);


void __RPC_STUB IDebugApplicationNode_Attach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNode_Detach_Proxy( 
    IDebugApplicationNode __RPC_FAR * This);


void __RPC_STUB IDebugApplicationNode_Detach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugApplicationNode_INTERFACE_DEFINED__ */


#ifndef __IDebugApplicationNodeEvents_INTERFACE_DEFINED__
#define __IDebugApplicationNodeEvents_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugApplicationNodeEvents
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugApplicationNodeEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C35-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugApplicationNodeEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE onAddChild( 
            /* [in] */ IDebugApplicationNode __RPC_FAR *prddpChild) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onRemoveChild( 
            /* [in] */ IDebugApplicationNode __RPC_FAR *prddpChild) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onDetach( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onAttach( 
            /* [in] */ IDebugApplicationNode __RPC_FAR *prddpParent) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugApplicationNodeEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugApplicationNodeEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugApplicationNodeEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugApplicationNodeEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onAddChild )( 
            IDebugApplicationNodeEvents __RPC_FAR * This,
            /* [in] */ IDebugApplicationNode __RPC_FAR *prddpChild);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onRemoveChild )( 
            IDebugApplicationNodeEvents __RPC_FAR * This,
            /* [in] */ IDebugApplicationNode __RPC_FAR *prddpChild);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onDetach )( 
            IDebugApplicationNodeEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onAttach )( 
            IDebugApplicationNodeEvents __RPC_FAR * This,
            /* [in] */ IDebugApplicationNode __RPC_FAR *prddpParent);
        
        END_INTERFACE
    } IDebugApplicationNodeEventsVtbl;

    interface IDebugApplicationNodeEvents
    {
        CONST_VTBL struct IDebugApplicationNodeEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugApplicationNodeEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugApplicationNodeEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugApplicationNodeEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugApplicationNodeEvents_onAddChild(This,prddpChild)	\
    (This)->lpVtbl -> onAddChild(This,prddpChild)

#define IDebugApplicationNodeEvents_onRemoveChild(This,prddpChild)	\
    (This)->lpVtbl -> onRemoveChild(This,prddpChild)

#define IDebugApplicationNodeEvents_onDetach(This)	\
    (This)->lpVtbl -> onDetach(This)

#define IDebugApplicationNodeEvents_onAttach(This,prddpParent)	\
    (This)->lpVtbl -> onAttach(This,prddpParent)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugApplicationNodeEvents_onAddChild_Proxy( 
    IDebugApplicationNodeEvents __RPC_FAR * This,
    /* [in] */ IDebugApplicationNode __RPC_FAR *prddpChild);


void __RPC_STUB IDebugApplicationNodeEvents_onAddChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNodeEvents_onRemoveChild_Proxy( 
    IDebugApplicationNodeEvents __RPC_FAR * This,
    /* [in] */ IDebugApplicationNode __RPC_FAR *prddpChild);


void __RPC_STUB IDebugApplicationNodeEvents_onRemoveChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNodeEvents_onDetach_Proxy( 
    IDebugApplicationNodeEvents __RPC_FAR * This);


void __RPC_STUB IDebugApplicationNodeEvents_onDetach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNodeEvents_onAttach_Proxy( 
    IDebugApplicationNodeEvents __RPC_FAR * This,
    /* [in] */ IDebugApplicationNode __RPC_FAR *prddpParent);


void __RPC_STUB IDebugApplicationNodeEvents_onAttach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugApplicationNodeEvents_INTERFACE_DEFINED__ */


#ifndef __IDebugThreadCall_INTERFACE_DEFINED__
#define __IDebugThreadCall_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugThreadCall
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugThreadCall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C36-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugThreadCall : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ThreadCallHandler( 
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2,
            /* [in] */ DWORD dwParam3) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugThreadCallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugThreadCall __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugThreadCall __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugThreadCall __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ThreadCallHandler )( 
            IDebugThreadCall __RPC_FAR * This,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2,
            /* [in] */ DWORD dwParam3);
        
        END_INTERFACE
    } IDebugThreadCallVtbl;

    interface IDebugThreadCall
    {
        CONST_VTBL struct IDebugThreadCallVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugThreadCall_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugThreadCall_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugThreadCall_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugThreadCall_ThreadCallHandler(This,dwParam1,dwParam2,dwParam3)	\
    (This)->lpVtbl -> ThreadCallHandler(This,dwParam1,dwParam2,dwParam3)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugThreadCall_ThreadCallHandler_Proxy( 
    IDebugThreadCall __RPC_FAR * This,
    /* [in] */ DWORD dwParam1,
    /* [in] */ DWORD dwParam2,
    /* [in] */ DWORD dwParam3);


void __RPC_STUB IDebugThreadCall_ThreadCallHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugThreadCall_INTERFACE_DEFINED__ */


#ifndef __IRemoteDebugApplicationThread_INTERFACE_DEFINED__
#define __IRemoteDebugApplicationThread_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRemoteDebugApplicationThread
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 


typedef DWORD THREAD_STATE;

#define	THREAD_STATE_RUNNING	( 0x1 )

#define	THREAD_STATE_SUSPENDED	( 0x2 )

#define	THREAD_BLOCKED	( 0x4 )

#define	THREAD_OUT_OF_CONTEXT	( 0x8 )


EXTERN_C const IID IID_IRemoteDebugApplicationThread;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C37-CB0C-11d0-B5C9-00A0244A0E7A")
    IRemoteDebugApplicationThread : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSystemThreadId( 
            /* [out] */ DWORD __RPC_FAR *dwThreadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetApplication( 
            /* [out] */ IRemoteDebugApplication __RPC_FAR *__RPC_FAR *pprda) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumStackFrames( 
            /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
            /* [out] */ BSTR __RPC_FAR *pbstrDescription,
            /* [out] */ BSTR __RPC_FAR *pbstrState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNextStatement( 
            /* [in] */ IDebugStackFrame __RPC_FAR *pStackFrame,
            /* [in] */ IDebugCodeContext __RPC_FAR *pCodeContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetState( 
            /* [out] */ DWORD __RPC_FAR *pState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Suspend( 
            /* [out] */ DWORD __RPC_FAR *pdwCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resume( 
            /* [out] */ DWORD __RPC_FAR *pdwCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSuspendCount( 
            /* [out] */ DWORD __RPC_FAR *pdwCount) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRemoteDebugApplicationThreadVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRemoteDebugApplicationThread __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRemoteDebugApplicationThread __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRemoteDebugApplicationThread __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSystemThreadId )( 
            IRemoteDebugApplicationThread __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *dwThreadId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetApplication )( 
            IRemoteDebugApplicationThread __RPC_FAR * This,
            /* [out] */ IRemoteDebugApplication __RPC_FAR *__RPC_FAR *pprda);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumStackFrames )( 
            IRemoteDebugApplicationThread __RPC_FAR * This,
            /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDescription )( 
            IRemoteDebugApplicationThread __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrDescription,
            /* [out] */ BSTR __RPC_FAR *pbstrState);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetNextStatement )( 
            IRemoteDebugApplicationThread __RPC_FAR * This,
            /* [in] */ IDebugStackFrame __RPC_FAR *pStackFrame,
            /* [in] */ IDebugCodeContext __RPC_FAR *pCodeContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetState )( 
            IRemoteDebugApplicationThread __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pState);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Suspend )( 
            IRemoteDebugApplicationThread __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Resume )( 
            IRemoteDebugApplicationThread __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSuspendCount )( 
            IRemoteDebugApplicationThread __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwCount);
        
        END_INTERFACE
    } IRemoteDebugApplicationThreadVtbl;

    interface IRemoteDebugApplicationThread
    {
        CONST_VTBL struct IRemoteDebugApplicationThreadVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteDebugApplicationThread_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteDebugApplicationThread_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteDebugApplicationThread_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteDebugApplicationThread_GetSystemThreadId(This,dwThreadId)	\
    (This)->lpVtbl -> GetSystemThreadId(This,dwThreadId)

#define IRemoteDebugApplicationThread_GetApplication(This,pprda)	\
    (This)->lpVtbl -> GetApplication(This,pprda)

#define IRemoteDebugApplicationThread_EnumStackFrames(This,ppedsf)	\
    (This)->lpVtbl -> EnumStackFrames(This,ppedsf)

#define IRemoteDebugApplicationThread_GetDescription(This,pbstrDescription,pbstrState)	\
    (This)->lpVtbl -> GetDescription(This,pbstrDescription,pbstrState)

#define IRemoteDebugApplicationThread_SetNextStatement(This,pStackFrame,pCodeContext)	\
    (This)->lpVtbl -> SetNextStatement(This,pStackFrame,pCodeContext)

#define IRemoteDebugApplicationThread_GetState(This,pState)	\
    (This)->lpVtbl -> GetState(This,pState)

#define IRemoteDebugApplicationThread_Suspend(This,pdwCount)	\
    (This)->lpVtbl -> Suspend(This,pdwCount)

#define IRemoteDebugApplicationThread_Resume(This,pdwCount)	\
    (This)->lpVtbl -> Resume(This,pdwCount)

#define IRemoteDebugApplicationThread_GetSuspendCount(This,pdwCount)	\
    (This)->lpVtbl -> GetSuspendCount(This,pdwCount)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_GetSystemThreadId_Proxy( 
    IRemoteDebugApplicationThread __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *dwThreadId);


void __RPC_STUB IRemoteDebugApplicationThread_GetSystemThreadId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_GetApplication_Proxy( 
    IRemoteDebugApplicationThread __RPC_FAR * This,
    /* [out] */ IRemoteDebugApplication __RPC_FAR *__RPC_FAR *pprda);


void __RPC_STUB IRemoteDebugApplicationThread_GetApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_EnumStackFrames_Proxy( 
    IRemoteDebugApplicationThread __RPC_FAR * This,
    /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf);


void __RPC_STUB IRemoteDebugApplicationThread_EnumStackFrames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_GetDescription_Proxy( 
    IRemoteDebugApplicationThread __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrDescription,
    /* [out] */ BSTR __RPC_FAR *pbstrState);


void __RPC_STUB IRemoteDebugApplicationThread_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_SetNextStatement_Proxy( 
    IRemoteDebugApplicationThread __RPC_FAR * This,
    /* [in] */ IDebugStackFrame __RPC_FAR *pStackFrame,
    /* [in] */ IDebugCodeContext __RPC_FAR *pCodeContext);


void __RPC_STUB IRemoteDebugApplicationThread_SetNextStatement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_GetState_Proxy( 
    IRemoteDebugApplicationThread __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pState);


void __RPC_STUB IRemoteDebugApplicationThread_GetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_Suspend_Proxy( 
    IRemoteDebugApplicationThread __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwCount);


void __RPC_STUB IRemoteDebugApplicationThread_Suspend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_Resume_Proxy( 
    IRemoteDebugApplicationThread __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwCount);


void __RPC_STUB IRemoteDebugApplicationThread_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_GetSuspendCount_Proxy( 
    IRemoteDebugApplicationThread __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwCount);


void __RPC_STUB IRemoteDebugApplicationThread_GetSuspendCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRemoteDebugApplicationThread_INTERFACE_DEFINED__ */


#ifndef __IDebugApplicationThread_INTERFACE_DEFINED__
#define __IDebugApplicationThread_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugApplicationThread
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][uuid][object] */ 



EXTERN_C const IID IID_IDebugApplicationThread;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C38-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugApplicationThread : public IRemoteDebugApplicationThread
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SynchronousCallIntoThread( 
            /* [in] */ IDebugThreadCall __RPC_FAR *pstcb,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2,
            /* [in] */ DWORD dwParam3) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryIsCurrentThread( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryIsDebuggerThread( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDescription( 
            /* [in] */ LPCOLESTR pstrDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStateString( 
            /* [in] */ LPCOLESTR pstrState) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugApplicationThreadVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugApplicationThread __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugApplicationThread __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugApplicationThread __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSystemThreadId )( 
            IDebugApplicationThread __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *dwThreadId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetApplication )( 
            IDebugApplicationThread __RPC_FAR * This,
            /* [out] */ IRemoteDebugApplication __RPC_FAR *__RPC_FAR *pprda);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumStackFrames )( 
            IDebugApplicationThread __RPC_FAR * This,
            /* [out] */ IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDescription )( 
            IDebugApplicationThread __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrDescription,
            /* [out] */ BSTR __RPC_FAR *pbstrState);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetNextStatement )( 
            IDebugApplicationThread __RPC_FAR * This,
            /* [in] */ IDebugStackFrame __RPC_FAR *pStackFrame,
            /* [in] */ IDebugCodeContext __RPC_FAR *pCodeContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetState )( 
            IDebugApplicationThread __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pState);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Suspend )( 
            IDebugApplicationThread __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Resume )( 
            IDebugApplicationThread __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSuspendCount )( 
            IDebugApplicationThread __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SynchronousCallIntoThread )( 
            IDebugApplicationThread __RPC_FAR * This,
            /* [in] */ IDebugThreadCall __RPC_FAR *pstcb,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2,
            /* [in] */ DWORD dwParam3);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryIsCurrentThread )( 
            IDebugApplicationThread __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryIsDebuggerThread )( 
            IDebugApplicationThread __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDescription )( 
            IDebugApplicationThread __RPC_FAR * This,
            /* [in] */ LPCOLESTR pstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetStateString )( 
            IDebugApplicationThread __RPC_FAR * This,
            /* [in] */ LPCOLESTR pstrState);
        
        END_INTERFACE
    } IDebugApplicationThreadVtbl;

    interface IDebugApplicationThread
    {
        CONST_VTBL struct IDebugApplicationThreadVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugApplicationThread_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugApplicationThread_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugApplicationThread_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugApplicationThread_GetSystemThreadId(This,dwThreadId)	\
    (This)->lpVtbl -> GetSystemThreadId(This,dwThreadId)

#define IDebugApplicationThread_GetApplication(This,pprda)	\
    (This)->lpVtbl -> GetApplication(This,pprda)

#define IDebugApplicationThread_EnumStackFrames(This,ppedsf)	\
    (This)->lpVtbl -> EnumStackFrames(This,ppedsf)

#define IDebugApplicationThread_GetDescription(This,pbstrDescription,pbstrState)	\
    (This)->lpVtbl -> GetDescription(This,pbstrDescription,pbstrState)

#define IDebugApplicationThread_SetNextStatement(This,pStackFrame,pCodeContext)	\
    (This)->lpVtbl -> SetNextStatement(This,pStackFrame,pCodeContext)

#define IDebugApplicationThread_GetState(This,pState)	\
    (This)->lpVtbl -> GetState(This,pState)

#define IDebugApplicationThread_Suspend(This,pdwCount)	\
    (This)->lpVtbl -> Suspend(This,pdwCount)

#define IDebugApplicationThread_Resume(This,pdwCount)	\
    (This)->lpVtbl -> Resume(This,pdwCount)

#define IDebugApplicationThread_GetSuspendCount(This,pdwCount)	\
    (This)->lpVtbl -> GetSuspendCount(This,pdwCount)


#define IDebugApplicationThread_SynchronousCallIntoThread(This,pstcb,dwParam1,dwParam2,dwParam3)	\
    (This)->lpVtbl -> SynchronousCallIntoThread(This,pstcb,dwParam1,dwParam2,dwParam3)

#define IDebugApplicationThread_QueryIsCurrentThread(This)	\
    (This)->lpVtbl -> QueryIsCurrentThread(This)

#define IDebugApplicationThread_QueryIsDebuggerThread(This)	\
    (This)->lpVtbl -> QueryIsDebuggerThread(This)

#define IDebugApplicationThread_SetDescription(This,pstrDescription)	\
    (This)->lpVtbl -> SetDescription(This,pstrDescription)

#define IDebugApplicationThread_SetStateString(This,pstrState)	\
    (This)->lpVtbl -> SetStateString(This,pstrState)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugApplicationThread_SynchronousCallIntoThread_Proxy( 
    IDebugApplicationThread __RPC_FAR * This,
    /* [in] */ IDebugThreadCall __RPC_FAR *pstcb,
    /* [in] */ DWORD dwParam1,
    /* [in] */ DWORD dwParam2,
    /* [in] */ DWORD dwParam3);


void __RPC_STUB IDebugApplicationThread_SynchronousCallIntoThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationThread_QueryIsCurrentThread_Proxy( 
    IDebugApplicationThread __RPC_FAR * This);


void __RPC_STUB IDebugApplicationThread_QueryIsCurrentThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationThread_QueryIsDebuggerThread_Proxy( 
    IDebugApplicationThread __RPC_FAR * This);


void __RPC_STUB IDebugApplicationThread_QueryIsDebuggerThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationThread_SetDescription_Proxy( 
    IDebugApplicationThread __RPC_FAR * This,
    /* [in] */ LPCOLESTR pstrDescription);


void __RPC_STUB IDebugApplicationThread_SetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationThread_SetStateString_Proxy( 
    IDebugApplicationThread __RPC_FAR * This,
    /* [in] */ LPCOLESTR pstrState);


void __RPC_STUB IDebugApplicationThread_SetStateString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugApplicationThread_INTERFACE_DEFINED__ */


#ifndef __IDebugCookie_INTERFACE_DEFINED__
#define __IDebugCookie_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugCookie
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][local][object] */ 



EXTERN_C const IID IID_IDebugCookie;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C39-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugCookie : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetDebugCookie( 
            /* [in] */ DWORD dwDebugAppCookie) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugCookieVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugCookie __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugCookie __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugCookie __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDebugCookie )( 
            IDebugCookie __RPC_FAR * This,
            /* [in] */ DWORD dwDebugAppCookie);
        
        END_INTERFACE
    } IDebugCookieVtbl;

    interface IDebugCookie
    {
        CONST_VTBL struct IDebugCookieVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugCookie_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugCookie_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugCookie_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugCookie_SetDebugCookie(This,dwDebugAppCookie)	\
    (This)->lpVtbl -> SetDebugCookie(This,dwDebugAppCookie)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugCookie_SetDebugCookie_Proxy( 
    IDebugCookie __RPC_FAR * This,
    /* [in] */ DWORD dwDebugAppCookie);


void __RPC_STUB IDebugCookie_SetDebugCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugCookie_INTERFACE_DEFINED__ */


#ifndef __IEnumDebugApplicationNodes_INTERFACE_DEFINED__
#define __IEnumDebugApplicationNodes_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumDebugApplicationNodes
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IEnumDebugApplicationNodes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C3a-CB0C-11d0-B5C9-00A0244A0E7A")
    IEnumDebugApplicationNodes : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *pprddp,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumDebugApplicationNodes __RPC_FAR *__RPC_FAR *pperddp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumDebugApplicationNodesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumDebugApplicationNodes __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumDebugApplicationNodes __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumDebugApplicationNodes __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumDebugApplicationNodes __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *pprddp,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumDebugApplicationNodes __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumDebugApplicationNodes __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumDebugApplicationNodes __RPC_FAR * This,
            /* [out] */ IEnumDebugApplicationNodes __RPC_FAR *__RPC_FAR *pperddp);
        
        END_INTERFACE
    } IEnumDebugApplicationNodesVtbl;

    interface IEnumDebugApplicationNodes
    {
        CONST_VTBL struct IEnumDebugApplicationNodesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDebugApplicationNodes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDebugApplicationNodes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDebugApplicationNodes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDebugApplicationNodes_Next(This,celt,pprddp,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,pprddp,pceltFetched)

#define IEnumDebugApplicationNodes_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumDebugApplicationNodes_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDebugApplicationNodes_Clone(This,pperddp)	\
    (This)->lpVtbl -> Clone(This,pperddp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumDebugApplicationNodes_RemoteNext_Proxy( 
    IEnumDebugApplicationNodes __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *pprddp,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumDebugApplicationNodes_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugApplicationNodes_Skip_Proxy( 
    IEnumDebugApplicationNodes __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumDebugApplicationNodes_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugApplicationNodes_Reset_Proxy( 
    IEnumDebugApplicationNodes __RPC_FAR * This);


void __RPC_STUB IEnumDebugApplicationNodes_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugApplicationNodes_Clone_Proxy( 
    IEnumDebugApplicationNodes __RPC_FAR * This,
    /* [out] */ IEnumDebugApplicationNodes __RPC_FAR *__RPC_FAR *pperddp);


void __RPC_STUB IEnumDebugApplicationNodes_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumDebugApplicationNodes_INTERFACE_DEFINED__ */


#ifndef __IEnumRemoteDebugApplications_INTERFACE_DEFINED__
#define __IEnumRemoteDebugApplications_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumRemoteDebugApplications
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IEnumRemoteDebugApplications;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C3b-CB0C-11d0-B5C9-00A0244A0E7A")
    IEnumRemoteDebugApplications : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IRemoteDebugApplication __RPC_FAR *__RPC_FAR *ppda,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumRemoteDebugApplications __RPC_FAR *__RPC_FAR *ppessd) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumRemoteDebugApplicationsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumRemoteDebugApplications __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumRemoteDebugApplications __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumRemoteDebugApplications __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumRemoteDebugApplications __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ IRemoteDebugApplication __RPC_FAR *__RPC_FAR *ppda,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumRemoteDebugApplications __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumRemoteDebugApplications __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumRemoteDebugApplications __RPC_FAR * This,
            /* [out] */ IEnumRemoteDebugApplications __RPC_FAR *__RPC_FAR *ppessd);
        
        END_INTERFACE
    } IEnumRemoteDebugApplicationsVtbl;

    interface IEnumRemoteDebugApplications
    {
        CONST_VTBL struct IEnumRemoteDebugApplicationsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumRemoteDebugApplications_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumRemoteDebugApplications_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumRemoteDebugApplications_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumRemoteDebugApplications_Next(This,celt,ppda,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppda,pceltFetched)

#define IEnumRemoteDebugApplications_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumRemoteDebugApplications_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumRemoteDebugApplications_Clone(This,ppessd)	\
    (This)->lpVtbl -> Clone(This,ppessd)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumRemoteDebugApplications_RemoteNext_Proxy( 
    IEnumRemoteDebugApplications __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IRemoteDebugApplication __RPC_FAR *__RPC_FAR *ppda,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumRemoteDebugApplications_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRemoteDebugApplications_Skip_Proxy( 
    IEnumRemoteDebugApplications __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumRemoteDebugApplications_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRemoteDebugApplications_Reset_Proxy( 
    IEnumRemoteDebugApplications __RPC_FAR * This);


void __RPC_STUB IEnumRemoteDebugApplications_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRemoteDebugApplications_Clone_Proxy( 
    IEnumRemoteDebugApplications __RPC_FAR * This,
    /* [out] */ IEnumRemoteDebugApplications __RPC_FAR *__RPC_FAR *ppessd);


void __RPC_STUB IEnumRemoteDebugApplications_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumRemoteDebugApplications_INTERFACE_DEFINED__ */


#ifndef __IEnumRemoteDebugApplicationThreads_INTERFACE_DEFINED__
#define __IEnumRemoteDebugApplicationThreads_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumRemoteDebugApplicationThreads
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IEnumRemoteDebugApplicationThreads;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C3c-CB0C-11d0-B5C9-00A0244A0E7A")
    IEnumRemoteDebugApplicationThreads : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IRemoteDebugApplicationThread __RPC_FAR *__RPC_FAR *pprdat,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumRemoteDebugApplicationThreads __RPC_FAR *__RPC_FAR *pperdat) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumRemoteDebugApplicationThreadsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumRemoteDebugApplicationThreads __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumRemoteDebugApplicationThreads __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumRemoteDebugApplicationThreads __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumRemoteDebugApplicationThreads __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ IRemoteDebugApplicationThread __RPC_FAR *__RPC_FAR *pprdat,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumRemoteDebugApplicationThreads __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumRemoteDebugApplicationThreads __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumRemoteDebugApplicationThreads __RPC_FAR * This,
            /* [out] */ IEnumRemoteDebugApplicationThreads __RPC_FAR *__RPC_FAR *pperdat);
        
        END_INTERFACE
    } IEnumRemoteDebugApplicationThreadsVtbl;

    interface IEnumRemoteDebugApplicationThreads
    {
        CONST_VTBL struct IEnumRemoteDebugApplicationThreadsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumRemoteDebugApplicationThreads_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumRemoteDebugApplicationThreads_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumRemoteDebugApplicationThreads_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumRemoteDebugApplicationThreads_Next(This,celt,pprdat,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,pprdat,pceltFetched)

#define IEnumRemoteDebugApplicationThreads_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumRemoteDebugApplicationThreads_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumRemoteDebugApplicationThreads_Clone(This,pperdat)	\
    (This)->lpVtbl -> Clone(This,pperdat)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumRemoteDebugApplicationThreads_RemoteNext_Proxy( 
    IEnumRemoteDebugApplicationThreads __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IRemoteDebugApplicationThread __RPC_FAR *__RPC_FAR *ppdat,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumRemoteDebugApplicationThreads_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRemoteDebugApplicationThreads_Skip_Proxy( 
    IEnumRemoteDebugApplicationThreads __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumRemoteDebugApplicationThreads_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRemoteDebugApplicationThreads_Reset_Proxy( 
    IEnumRemoteDebugApplicationThreads __RPC_FAR * This);


void __RPC_STUB IEnumRemoteDebugApplicationThreads_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRemoteDebugApplicationThreads_Clone_Proxy( 
    IEnumRemoteDebugApplicationThreads __RPC_FAR * This,
    /* [out] */ IEnumRemoteDebugApplicationThreads __RPC_FAR *__RPC_FAR *pperdat);


void __RPC_STUB IEnumRemoteDebugApplicationThreads_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumRemoteDebugApplicationThreads_INTERFACE_DEFINED__ */


#ifndef __IDebugFormatter_INTERFACE_DEFINED__
#define __IDebugFormatter_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugFormatter
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IDebugFormatter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C05-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugFormatter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStringForVariant( 
            /* [in] */ VARIANT __RPC_FAR *pvar,
            /* [in] */ ULONG nRadix,
            /* [out] */ BSTR __RPC_FAR *pbstrValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVariantForString( 
            /* [in] */ LPCOLESTR pwstrValue,
            /* [out] */ VARIANT __RPC_FAR *pvar) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStringForVarType( 
            /* [in] */ VARTYPE vt,
            /* [in] */ TYPEDESC __RPC_FAR *ptdescArrayType,
            /* [out] */ BSTR __RPC_FAR *pbstr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugFormatterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugFormatter __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugFormatter __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugFormatter __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStringForVariant )( 
            IDebugFormatter __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *pvar,
            /* [in] */ ULONG nRadix,
            /* [out] */ BSTR __RPC_FAR *pbstrValue);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVariantForString )( 
            IDebugFormatter __RPC_FAR * This,
            /* [in] */ LPCOLESTR pwstrValue,
            /* [out] */ VARIANT __RPC_FAR *pvar);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStringForVarType )( 
            IDebugFormatter __RPC_FAR * This,
            /* [in] */ VARTYPE vt,
            /* [in] */ TYPEDESC __RPC_FAR *ptdescArrayType,
            /* [out] */ BSTR __RPC_FAR *pbstr);
        
        END_INTERFACE
    } IDebugFormatterVtbl;

    interface IDebugFormatter
    {
        CONST_VTBL struct IDebugFormatterVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugFormatter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugFormatter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugFormatter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugFormatter_GetStringForVariant(This,pvar,nRadix,pbstrValue)	\
    (This)->lpVtbl -> GetStringForVariant(This,pvar,nRadix,pbstrValue)

#define IDebugFormatter_GetVariantForString(This,pwstrValue,pvar)	\
    (This)->lpVtbl -> GetVariantForString(This,pwstrValue,pvar)

#define IDebugFormatter_GetStringForVarType(This,vt,ptdescArrayType,pbstr)	\
    (This)->lpVtbl -> GetStringForVarType(This,vt,ptdescArrayType,pbstr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugFormatter_GetStringForVariant_Proxy( 
    IDebugFormatter __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *pvar,
    /* [in] */ ULONG nRadix,
    /* [out] */ BSTR __RPC_FAR *pbstrValue);


void __RPC_STUB IDebugFormatter_GetStringForVariant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugFormatter_GetVariantForString_Proxy( 
    IDebugFormatter __RPC_FAR * This,
    /* [in] */ LPCOLESTR pwstrValue,
    /* [out] */ VARIANT __RPC_FAR *pvar);


void __RPC_STUB IDebugFormatter_GetVariantForString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugFormatter_GetStringForVarType_Proxy( 
    IDebugFormatter __RPC_FAR * This,
    /* [in] */ VARTYPE vt,
    /* [in] */ TYPEDESC __RPC_FAR *ptdescArrayType,
    /* [out] */ BSTR __RPC_FAR *pbstr);


void __RPC_STUB IDebugFormatter_GetStringForVarType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugFormatter_INTERFACE_DEFINED__ */


#ifndef __ISimpleConnectionPoint_INTERFACE_DEFINED__
#define __ISimpleConnectionPoint_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ISimpleConnectionPoint
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_ISimpleConnectionPoint;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C3e-CB0C-11d0-B5C9-00A0244A0E7A")
    ISimpleConnectionPoint : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetEventCount( 
            /* [out] */ ULONG __RPC_FAR *pulCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DescribeEvents( 
            /* [in] */ ULONG iEvent,
            /* [in] */ ULONG cEvents,
            /* [length_is][size_is][out] */ DISPID __RPC_FAR *prgid,
            /* [length_is][size_is][out] */ BSTR __RPC_FAR *prgbstr,
            /* [out] */ ULONG __RPC_FAR *pcEventsFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Advise( 
            /* [in] */ IDispatch __RPC_FAR *pdisp,
            /* [out] */ DWORD __RPC_FAR *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unadvise( 
            /* [in] */ DWORD dwCookie) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISimpleConnectionPointVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISimpleConnectionPoint __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISimpleConnectionPoint __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISimpleConnectionPoint __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEventCount )( 
            ISimpleConnectionPoint __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pulCount);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DescribeEvents )( 
            ISimpleConnectionPoint __RPC_FAR * This,
            /* [in] */ ULONG iEvent,
            /* [in] */ ULONG cEvents,
            /* [length_is][size_is][out] */ DISPID __RPC_FAR *prgid,
            /* [length_is][size_is][out] */ BSTR __RPC_FAR *prgbstr,
            /* [out] */ ULONG __RPC_FAR *pcEventsFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Advise )( 
            ISimpleConnectionPoint __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *pdisp,
            /* [out] */ DWORD __RPC_FAR *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Unadvise )( 
            ISimpleConnectionPoint __RPC_FAR * This,
            /* [in] */ DWORD dwCookie);
        
        END_INTERFACE
    } ISimpleConnectionPointVtbl;

    interface ISimpleConnectionPoint
    {
        CONST_VTBL struct ISimpleConnectionPointVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISimpleConnectionPoint_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISimpleConnectionPoint_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISimpleConnectionPoint_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISimpleConnectionPoint_GetEventCount(This,pulCount)	\
    (This)->lpVtbl -> GetEventCount(This,pulCount)

#define ISimpleConnectionPoint_DescribeEvents(This,iEvent,cEvents,prgid,prgbstr,pcEventsFetched)	\
    (This)->lpVtbl -> DescribeEvents(This,iEvent,cEvents,prgid,prgbstr,pcEventsFetched)

#define ISimpleConnectionPoint_Advise(This,pdisp,pdwCookie)	\
    (This)->lpVtbl -> Advise(This,pdisp,pdwCookie)

#define ISimpleConnectionPoint_Unadvise(This,dwCookie)	\
    (This)->lpVtbl -> Unadvise(This,dwCookie)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISimpleConnectionPoint_GetEventCount_Proxy( 
    ISimpleConnectionPoint __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pulCount);


void __RPC_STUB ISimpleConnectionPoint_GetEventCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISimpleConnectionPoint_DescribeEvents_Proxy( 
    ISimpleConnectionPoint __RPC_FAR * This,
    /* [in] */ ULONG iEvent,
    /* [in] */ ULONG cEvents,
    /* [length_is][size_is][out] */ DISPID __RPC_FAR *prgid,
    /* [length_is][size_is][out] */ BSTR __RPC_FAR *prgbstr,
    /* [out] */ ULONG __RPC_FAR *pcEventsFetched);


void __RPC_STUB ISimpleConnectionPoint_DescribeEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISimpleConnectionPoint_Advise_Proxy( 
    ISimpleConnectionPoint __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *pdisp,
    /* [out] */ DWORD __RPC_FAR *pdwCookie);


void __RPC_STUB ISimpleConnectionPoint_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISimpleConnectionPoint_Unadvise_Proxy( 
    ISimpleConnectionPoint __RPC_FAR * This,
    /* [in] */ DWORD dwCookie);


void __RPC_STUB ISimpleConnectionPoint_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISimpleConnectionPoint_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL_itf_activdbg_0209
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local] */ 


EXTERN_C const CLSID CLSID_DebugHelper;


extern RPC_IF_HANDLE __MIDL_itf_activdbg_0209_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0209_v0_0_s_ifspec;

#ifndef __IDebugHelper_INTERFACE_DEFINED__
#define __IDebugHelper_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDebugHelper
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local][unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IDebugHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C3f-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugHelper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreatePropertyBrowser( 
            /* [in] */ VARIANT __RPC_FAR *pvar,
            /* [in] */ LPCOLESTR bstrName,
            /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreatePropertyBrowserEx( 
            /* [in] */ VARIANT __RPC_FAR *pvar,
            /* [in] */ LPCOLESTR bstrName,
            /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
            /* [in] */ IDebugFormatter __RPC_FAR *pdf,
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateSimpleConnectionPoint( 
            /* [in] */ IDispatch __RPC_FAR *pdisp,
            /* [out] */ ISimpleConnectionPoint __RPC_FAR *__RPC_FAR *ppscp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDebugHelper __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDebugHelper __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDebugHelper __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreatePropertyBrowser )( 
            IDebugHelper __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *pvar,
            /* [in] */ LPCOLESTR bstrName,
            /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdob);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreatePropertyBrowserEx )( 
            IDebugHelper __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *pvar,
            /* [in] */ LPCOLESTR bstrName,
            /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
            /* [in] */ IDebugFormatter __RPC_FAR *pdf,
            /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdob);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateSimpleConnectionPoint )( 
            IDebugHelper __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *pdisp,
            /* [out] */ ISimpleConnectionPoint __RPC_FAR *__RPC_FAR *ppscp);
        
        END_INTERFACE
    } IDebugHelperVtbl;

    interface IDebugHelper
    {
        CONST_VTBL struct IDebugHelperVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugHelper_CreatePropertyBrowser(This,pvar,bstrName,pdat,ppdob)	\
    (This)->lpVtbl -> CreatePropertyBrowser(This,pvar,bstrName,pdat,ppdob)

#define IDebugHelper_CreatePropertyBrowserEx(This,pvar,bstrName,pdat,pdf,ppdob)	\
    (This)->lpVtbl -> CreatePropertyBrowserEx(This,pvar,bstrName,pdat,pdf,ppdob)

#define IDebugHelper_CreateSimpleConnectionPoint(This,pdisp,ppscp)	\
    (This)->lpVtbl -> CreateSimpleConnectionPoint(This,pdisp,ppscp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugHelper_CreatePropertyBrowser_Proxy( 
    IDebugHelper __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *pvar,
    /* [in] */ LPCOLESTR bstrName,
    /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
    /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdob);


void __RPC_STUB IDebugHelper_CreatePropertyBrowser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugHelper_CreatePropertyBrowserEx_Proxy( 
    IDebugHelper __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *pvar,
    /* [in] */ LPCOLESTR bstrName,
    /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
    /* [in] */ IDebugFormatter __RPC_FAR *pdf,
    /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdob);


void __RPC_STUB IDebugHelper_CreatePropertyBrowserEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugHelper_CreateSimpleConnectionPoint_Proxy( 
    IDebugHelper __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *pdisp,
    /* [out] */ ISimpleConnectionPoint __RPC_FAR *__RPC_FAR *ppscp);


void __RPC_STUB IDebugHelper_CreateSimpleConnectionPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugHelper_INTERFACE_DEFINED__ */


#ifndef __IEnumDebugExpressionContexts_INTERFACE_DEFINED__
#define __IEnumDebugExpressionContexts_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumDebugExpressionContexts
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IEnumDebugExpressionContexts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C40-CB0C-11d0-B5C9-00A0244A0E7A")
    IEnumDebugExpressionContexts : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IDebugExpressionContext __RPC_FAR *__RPC_FAR *ppdec,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumDebugExpressionContexts __RPC_FAR *__RPC_FAR *ppedec) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumDebugExpressionContextsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumDebugExpressionContexts __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumDebugExpressionContexts __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumDebugExpressionContexts __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumDebugExpressionContexts __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ IDebugExpressionContext __RPC_FAR *__RPC_FAR *ppdec,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumDebugExpressionContexts __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumDebugExpressionContexts __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumDebugExpressionContexts __RPC_FAR * This,
            /* [out] */ IEnumDebugExpressionContexts __RPC_FAR *__RPC_FAR *ppedec);
        
        END_INTERFACE
    } IEnumDebugExpressionContextsVtbl;

    interface IEnumDebugExpressionContexts
    {
        CONST_VTBL struct IEnumDebugExpressionContextsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDebugExpressionContexts_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDebugExpressionContexts_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDebugExpressionContexts_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDebugExpressionContexts_Next(This,celt,ppdec,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppdec,pceltFetched)

#define IEnumDebugExpressionContexts_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumDebugExpressionContexts_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDebugExpressionContexts_Clone(This,ppedec)	\
    (This)->lpVtbl -> Clone(This,ppedec)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumDebugExpressionContexts_RemoteNext_Proxy( 
    IEnumDebugExpressionContexts __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IDebugExpressionContext __RPC_FAR *__RPC_FAR *pprgdec,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumDebugExpressionContexts_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugExpressionContexts_Skip_Proxy( 
    IEnumDebugExpressionContexts __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumDebugExpressionContexts_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugExpressionContexts_Reset_Proxy( 
    IEnumDebugExpressionContexts __RPC_FAR * This);


void __RPC_STUB IEnumDebugExpressionContexts_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugExpressionContexts_Clone_Proxy( 
    IEnumDebugExpressionContexts __RPC_FAR * This,
    /* [out] */ IEnumDebugExpressionContexts __RPC_FAR *__RPC_FAR *ppedec);


void __RPC_STUB IEnumDebugExpressionContexts_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumDebugExpressionContexts_INTERFACE_DEFINED__ */


#ifndef __IProvideExpressionContexts_INTERFACE_DEFINED__
#define __IProvideExpressionContexts_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IProvideExpressionContexts
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IProvideExpressionContexts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51973C41-CB0C-11d0-B5C9-00A0244A0E7A")
    IProvideExpressionContexts : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumExpressionContexts( 
            /* [out] */ IEnumDebugExpressionContexts __RPC_FAR *__RPC_FAR *ppedec) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IProvideExpressionContextsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IProvideExpressionContexts __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IProvideExpressionContexts __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IProvideExpressionContexts __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumExpressionContexts )( 
            IProvideExpressionContexts __RPC_FAR * This,
            /* [out] */ IEnumDebugExpressionContexts __RPC_FAR *__RPC_FAR *ppedec);
        
        END_INTERFACE
    } IProvideExpressionContextsVtbl;

    interface IProvideExpressionContexts
    {
        CONST_VTBL struct IProvideExpressionContextsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProvideExpressionContexts_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProvideExpressionContexts_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProvideExpressionContexts_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProvideExpressionContexts_EnumExpressionContexts(This,ppedec)	\
    (This)->lpVtbl -> EnumExpressionContexts(This,ppedec)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IProvideExpressionContexts_EnumExpressionContexts_Proxy( 
    IProvideExpressionContexts __RPC_FAR * This,
    /* [out] */ IEnumDebugExpressionContexts __RPC_FAR *__RPC_FAR *ppedec);


void __RPC_STUB IProvideExpressionContexts_EnumExpressionContexts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IProvideExpressionContexts_INTERFACE_DEFINED__ */



#ifndef __ProcessDebugManagerLib_LIBRARY_DEFINED__
#define __ProcessDebugManagerLib_LIBRARY_DEFINED__

/****************************************
 * Generated header for library: ProcessDebugManagerLib
 * at Fri Sep 18 16:27:25 1998
 * using MIDL 3.01.75
 ****************************************/
/* [helpstring][version][uuid] */ 



















































EXTERN_C const CLSID CLSID_CDebugDocumentHelper;

EXTERN_C const IID LIBID_ProcessDebugManagerLib;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_ProcessDebugManager;

class DECLSPEC_UUID("78a51822-51f4-11d0-8f20-00805f2cd064")
ProcessDebugManager;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_DebugHelper;

class DECLSPEC_UUID("0BFCC060-8C1D-11d0-ACCD-00AA0060275C")
DebugHelper;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_CDebugDocumentHelper;

class DECLSPEC_UUID("83B8BCA6-687C-11D0-A405-00AA0060275C")
CDebugDocumentHelper;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_MachineDebugManager;

class DECLSPEC_UUID("0C0A3666-30C9-11D0-8F20-00805F2CD064")
MachineDebugManager;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_DefaultDebugSessionProvider;

class DECLSPEC_UUID("834128a2-51f4-11d0-8f20-00805f2cd064")
DefaultDebugSessionProvider;
#endif
#endif /* __ProcessDebugManagerLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

/* [local] */ HRESULT __stdcall IEnumDebugCodeContexts_Next_Proxy( 
    IEnumDebugCodeContexts __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ IDebugCodeContext __RPC_FAR *__RPC_FAR *pscc,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumDebugCodeContexts_Next_Stub( 
    IEnumDebugCodeContexts __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IDebugCodeContext __RPC_FAR *__RPC_FAR *pscc,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT __stdcall IEnumDebugStackFrames_Next_Proxy( 
    IEnumDebugStackFrames __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ DebugStackFrameDescriptor __RPC_FAR *prgdsfd,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumDebugStackFrames_Next_Stub( 
    IEnumDebugStackFrames __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ DebugStackFrameDescriptor __RPC_FAR *prgdsfd,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT __stdcall IEnumDebugApplicationNodes_Next_Proxy( 
    IEnumDebugApplicationNodes __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *pprddp,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumDebugApplicationNodes_Next_Stub( 
    IEnumDebugApplicationNodes __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *pprddp,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT __stdcall IEnumRemoteDebugApplications_Next_Proxy( 
    IEnumRemoteDebugApplications __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ IRemoteDebugApplication __RPC_FAR *__RPC_FAR *ppda,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumRemoteDebugApplications_Next_Stub( 
    IEnumRemoteDebugApplications __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IRemoteDebugApplication __RPC_FAR *__RPC_FAR *ppda,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT __stdcall IEnumRemoteDebugApplicationThreads_Next_Proxy( 
    IEnumRemoteDebugApplicationThreads __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ IRemoteDebugApplicationThread __RPC_FAR *__RPC_FAR *pprdat,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumRemoteDebugApplicationThreads_Next_Stub( 
    IEnumRemoteDebugApplicationThreads __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IRemoteDebugApplicationThread __RPC_FAR *__RPC_FAR *ppdat,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT __stdcall IEnumDebugExpressionContexts_Next_Proxy( 
    IEnumDebugExpressionContexts __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ IDebugExpressionContext __RPC_FAR *__RPC_FAR *ppdec,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumDebugExpressionContexts_Next_Stub( 
    IEnumDebugExpressionContexts __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IDebugExpressionContext __RPC_FAR *__RPC_FAR *pprgdec,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
