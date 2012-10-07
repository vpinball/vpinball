/*****************************************************************
|
|      XAudio Player GUI. Control Driver, Win32 implementation
|
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
 ****************************************************************/

#ifndef __CONTROL_WIN32_H__
#define __CONTROL_WIN32_H__

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#ifndef STRICT
#define STRICT
#endif /* STRICT */
#include <windows.h>

#include "control.h"
#include "xaudio.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define XA_MSG_OFFSET 0x1000
#define XA_MSG_BASE	(0x8000 + XA_MSG_OFFSET)

/*----------------------------------------------------------------------
|       prototypes
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

int XA_EXPORT control_message_to_win32_params(const XA_Message *message,
                                              UINT *win32_message,
                                              WPARAM *win32_wparam,
                                              LPARAM *win32_lparam);
    
int XA_EXPORT control_win32_params_to_message(void *control,
                                              XA_Message *message,
                                              UINT win32_message,
                                              WPARAM win32_wparam,
                                              LPARAM win32_lparam);

#ifdef __cplusplus
}
#endif

#endif /* __CONTROL_WIN32_H__    */

