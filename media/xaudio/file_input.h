/*****************************************************************
|
|      File Input Module
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
 ****************************************************************/

#ifndef __FILE_INPUT_H__
#define __FILE_INPUT_H__

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "decoder.h"

/*----------------------------------------------------------------------
|       prototypes
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

int XA_EXPORT file_input_module_register(XA_InputModule *module);

#ifdef __cplusplus
}
#endif
 
#endif /* __FILE_INPUT_H__ */
