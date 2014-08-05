/*****************************************************************
|
|      resampling_output_filter 
|
|      (c) 2000 Xaudio Corporation
|
 ****************************************************************/

#ifndef _RESAMPLING_OUTPUT_FILTER_H_
#define _RESAMPLING_OUTPUT_FILTER_H_

 /*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "decoder.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define XA_RESAMPLING_OUTPUT_FILTER_RATE_ENVIRONMENT "OUTPUT.RESAMPLER.RATE"
#define XA_RESAMPLING_OUTPUT_FILTER_MODE_ENVIRONMENT "OUTPUT.RESAMPLER.MODE"
#define XA_RESAMPLING_FILTER_MODE_CHANGE_OUTPUT_RATE  0
#define XA_RESAMPLING_FILTER_MODE_KEEP_OUTPUT_RATE    1

/*----------------------------------------------------------------------
|       prototypes
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" 
{
#endif

int XA_EXPORT resampling_output_filter_module_register(XA_OutputFilterModule *module);

#ifdef __cplusplus
}
#endif

#endif /* _RESAMPLING_OUTPUT_FILTER_H_ */

