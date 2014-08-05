/*****************************************************************
|
|      File Output Modules
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
 ****************************************************************/

#ifndef _FILE_OUTPUT_H_
#define _FILE_OUTPUT_H_

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

int XA_EXPORT file_output_module_register(XA_OutputModule *module);

#ifdef __cplusplus
}
#endif
 
#endif /* _FILE_OUTPUT_H_ */
