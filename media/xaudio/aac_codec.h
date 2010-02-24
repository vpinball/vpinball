/*****************************************************************
|
|      AAC Codec for Xaudio
|      This is a wrapper for the FHG AAC implementation
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
 ****************************************************************/

#ifndef __AAC_CODEC_H__
#define __AAC_CODEC_H__

/*----------------------------------------------------------------------
|       prototypes
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

int XA_EXPORT aac_codec_module_register(XA_CodecModule *module);

#ifdef __cplusplus
}
#endif


#endif /* __AAC_CODEC_H__ */
