/*****************************************************************
|
|      Memory Buffer Input Module
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
 ****************************************************************/

#ifndef _MEMORY_INPUT_H_
#define _MEMORY_INPUT_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "decoder.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define XA_MEMORY_INPUT_MESSAGE_BASE            0x1000
#define XA_MEMORY_INPUT_MESSAGE_FEED            0x1001
#define XA_MEMORY_INPUT_MESSAGE_FLUSH           0x1002
#define XA_MEMORY_INPUT_MESSAGE_SET_MIME_TYPE   0x1003
#define XA_MEMORY_INPUT_MESSAGE_SET_STREAM_SIZE 0x1004

/*----------------------------------------------------------------------
|       prototypes
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

int XA_EXPORT memory_input_module_register(XA_InputModule *module);

#ifdef __cplusplus
}
#endif
 
#endif /* _MEMORY_INPUT_H_ */
