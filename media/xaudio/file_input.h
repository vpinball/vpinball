/*****************************************************************
|
|      File Input Module
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
****************************************************************/

#ifndef _FILE_INPUT_H_
#define _FILE_INPUT_H_

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
 
#endif /* _FILE_INPUT_H_ */
