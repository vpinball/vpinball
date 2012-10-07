/*****************************************************************
|
|      MPEG Codec for Xaudio
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
****************************************************************/

#ifndef _MPEG_CODEC_H_
#define _MPEG_CODEC_H_

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define XA_MPEG_CODEC_PROPERTY_STREAM_LEVEL "mpeg/level"
#define XA_MPEG_CODEC_PROPERTY_STREAM_LAYER "mpeg/layer"
#define XA_MPEG_CODEC_PROPERTY_STREAM_MODE  "mpeg/mode"

/*----------------------------------------------------------------------
|       prototypes
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

int XA_EXPORT mpeg_codec_module_register(XA_CodecModule *module);

#ifdef __cplusplus
}
#endif


#endif /* _MPEG_CODEC_H_ */
