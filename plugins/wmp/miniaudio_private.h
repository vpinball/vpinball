#pragma once

#include <miniaudio/miniaudio.h>

#ifdef __cplusplus
extern "C" {
#endif

ma_decoder_config wmp_ma_decoder_config_init(ma_format outputFormat, ma_uint32 outputChannels, ma_uint32 outputSampleRate);
ma_result wmp_ma_decoder_init_file(const char* pFilePath, const ma_decoder_config* pConfig, ma_decoder* pDecoder);
ma_result wmp_ma_decoder_get_data_format(ma_decoder* pDecoder, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap);
ma_result wmp_ma_decoder_read_pcm_frames(ma_decoder* pDecoder, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead);
ma_result wmp_ma_decoder_get_cursor_in_pcm_frames(ma_decoder* pDecoder, ma_uint64* pCursor);
ma_result wmp_ma_decoder_seek_to_pcm_frame(ma_decoder* pDecoder, ma_uint64 frameIndex);
void wmp_ma_decoder_uninit(ma_decoder* pDecoder);

ma_result wmp_ma_engine_init_null_device(ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 periodSizeInFrames,
   ma_engine_process_proc onProcess, void* pProcessUserData, ma_context* pContext, ma_engine* pEngine);
void wmp_ma_engine_uninit(ma_engine* pEngine);
void wmp_ma_context_uninit(ma_context* pContext);
ma_result wmp_ma_engine_start(ma_engine* pEngine);
ma_result wmp_ma_engine_stop(ma_engine* pEngine);

ma_result wmp_ma_sound_init_from_decoder(ma_engine* pEngine, ma_decoder* pDecoder, ma_uint32 flags, ma_sound* pSound);
void wmp_ma_sound_uninit(ma_sound* pSound);
ma_result wmp_ma_sound_start(ma_sound* pSound);
ma_result wmp_ma_sound_stop(ma_sound* pSound);
void wmp_ma_sound_set_volume(ma_sound* pSound, float volume);
ma_result wmp_ma_sound_seek_to_pcm_frame(ma_sound* pSound, ma_uint64 frameIndex);
ma_bool32 wmp_ma_sound_at_end(const ma_sound* pSound);
void wmp_ma_sound_set_end_callback(ma_sound* pSound, ma_sound_end_proc callback, void* pUserData);

#ifdef __cplusplus
}
#endif