/*****************************************************************
|
|      Xaudio SDK. FFT Utilities
|
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
 ****************************************************************/

#ifndef _FFT_H_
#define _FFT_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "fft.h"

/*----------------------------------------------------------------------
|       types
+---------------------------------------------------------------------*/
typedef struct XA_FftAnalyzer XA_FftAnalyzer;

/*----------------------------------------------------------------------
|       prototypes
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

int XA_EXPORT fft_analyzer_new(XA_FftAnalyzer **analyzer);
int XA_EXPORT fft_analyzer_delete(XA_FftAnalyzer *analyzer);
int XA_EXPORT fft_analyzer_set_samples(XA_FftAnalyzer *analyzer, 
                                       const signed char *samples,
                                       unsigned int nb_samples);
int XA_EXPORT fft_analyzer_get_spectrum(XA_FftAnalyzer *analyzer,
                                        unsigned char *bands,
                                        unsigned int nb_bands);
int XA_EXPORT fft_analyzer_interpolate_samples_signed(const signed char *src,
                                                      unsigned int src_size,
                                                      signed char *dst,
                                                      unsigned int dst_size);
int XA_EXPORT fft_analyzer_interpolate_samples_unsigned(const unsigned char *src,
                                                        unsigned int src_size,
                                                        unsigned char *dst,
                                                        unsigned int dst_size);

#ifdef __cplusplus
}
#endif


#endif /* _FFT_H_ */
