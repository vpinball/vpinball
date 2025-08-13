#define MA_API static
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_CUSTOM

#define STB_VORBIS_HEADER_ONLY
#include <miniaudio/extras/stb_vorbis_static.c>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

#undef STB_VORBIS_HEADER_ONLY
#include <miniaudio/extras/stb_vorbis_static.c>

ma_decoder_config wmp_ma_decoder_config_init(ma_format outputFormat, ma_uint32 outputChannels, ma_uint32 outputSampleRate)
{
   return ma_decoder_config_init(outputFormat, outputChannels, outputSampleRate);
}

ma_result wmp_ma_decoder_init_file(const char* pFilePath, const ma_decoder_config* pConfig, ma_decoder* pDecoder)
{
   return ma_decoder_init_file(pFilePath, pConfig, pDecoder);
}

ma_result wmp_ma_decoder_get_data_format(ma_decoder* pDecoder, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap)
{
   return ma_decoder_get_data_format(pDecoder, pFormat, pChannels, pSampleRate, pChannelMap, channelMapCap);
}

ma_result wmp_ma_decoder_read_pcm_frames(ma_decoder* pDecoder, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead)
{
   return ma_decoder_read_pcm_frames(pDecoder, pFramesOut, frameCount, pFramesRead);
}

ma_result wmp_ma_decoder_get_cursor_in_pcm_frames(ma_decoder* pDecoder, ma_uint64* pCursor)
{
   return ma_decoder_get_cursor_in_pcm_frames(pDecoder, pCursor);
}

ma_result wmp_ma_decoder_seek_to_pcm_frame(ma_decoder* pDecoder, ma_uint64 frameIndex)
{
   return ma_decoder_seek_to_pcm_frame(pDecoder, frameIndex);
}

void wmp_ma_decoder_uninit(ma_decoder* pDecoder)
{
   ma_decoder_uninit(pDecoder);
}