#define MA_API static
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_CUSTOM
#define MA_ENABLE_NULL

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

ma_result wmp_ma_engine_init_null_device(ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 periodSizeInFrames,
   ma_engine_process_proc onProcess, void* pProcessUserData, ma_context* pContext, ma_engine* pEngine)
{
   // A null device gives us miniAudio's own realtime-paced audio thread (timing,
   // throttling and buffering) without ever touching the hardware. The mixed
   // output is delivered through onProcess and forwarded to the host.
   ma_backend backends[] = { ma_backend_null };
   ma_context_config contextConfig = ma_context_config_init();
   ma_result result = ma_context_init(backends, 1, &contextConfig, pContext);
   if (result != MA_SUCCESS)
      return result;

   ma_engine_config config = ma_engine_config_init();
   config.pContext = pContext;
   config.channels = channels;
   config.sampleRate = sampleRate;
   config.periodSizeInFrames = periodSizeInFrames;
   config.onProcess = onProcess;
   config.pProcessUserData = pProcessUserData;
   config.noAutoStart = MA_TRUE;

   result = ma_engine_init(&config, pEngine);
   if (result != MA_SUCCESS) {
      ma_context_uninit(pContext);
      return result;
   }
   return MA_SUCCESS;
}

void wmp_ma_engine_uninit(ma_engine* pEngine)
{
   ma_engine_uninit(pEngine);
}

void wmp_ma_context_uninit(ma_context* pContext)
{
   ma_context_uninit(pContext);
}

ma_result wmp_ma_engine_start(ma_engine* pEngine)
{
   return ma_engine_start(pEngine);
}

ma_result wmp_ma_engine_stop(ma_engine* pEngine)
{
   return ma_engine_stop(pEngine);
}

ma_result wmp_ma_sound_init_from_decoder(ma_engine* pEngine, ma_decoder* pDecoder, ma_uint32 flags, ma_sound* pSound)
{
   return ma_sound_init_from_data_source(pEngine, (ma_data_source*)pDecoder, flags, NULL, pSound);
}

void wmp_ma_sound_uninit(ma_sound* pSound)
{
   ma_sound_uninit(pSound);
}

ma_result wmp_ma_sound_start(ma_sound* pSound)
{
   return ma_sound_start(pSound);
}

ma_result wmp_ma_sound_stop(ma_sound* pSound)
{
   return ma_sound_stop(pSound);
}

void wmp_ma_sound_set_volume(ma_sound* pSound, float volume)
{
   ma_sound_set_volume(pSound, volume);
}

ma_result wmp_ma_sound_seek_to_pcm_frame(ma_sound* pSound, ma_uint64 frameIndex)
{
   return ma_sound_seek_to_pcm_frame(pSound, frameIndex);
}

ma_bool32 wmp_ma_sound_at_end(const ma_sound* pSound)
{
   return ma_sound_at_end(pSound);
}

void wmp_ma_sound_set_end_callback(ma_sound* pSound, ma_sound_end_proc callback, void* pUserData)
{
   ma_sound_set_end_callback(pSound, callback, pUserData);
}