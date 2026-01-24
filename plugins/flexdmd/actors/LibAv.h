#pragma once

extern "C" {
   #include "libavutil/imgutils.h"
   #include "libavcodec/avcodec.h"
   #include "libavformat/avformat.h"
   #include "libswscale/swscale.h"
   #include "libswresample/swresample.h"
}

namespace LibAV
{

#ifdef _WIN32
// As LibAvCodec is fairly heavy, we only load it when used to limit startup time impact
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <tchar.h>
#endif

#ifdef CDECL
#undef CDECL
#endif
#ifdef _WIN64
   // Windows x64 always uses standard calling convention (implicit)
   #define CDECL
#else
   // Other platforms, uses C calling convention
   #if defined(__GNUC__)
      // TODO GCC: #define CDECL __attribute__((__cdecl))
      #define CDECL
   #else
      #define CDECL __cdecl
   #endif
#endif

class LibAV
{
public:
   static const LibAV& GetInstance()
   {
      static LibAV lib;
      lib.Load();
      return lib;
   }

   bool isLoaded = false;

   typedef int(CDECL* fn_av_get_bytes_per_sample)(enum AVSampleFormat sample_fmt);
   typedef int(CDECL* fn_av_find_best_stream)(AVFormatContext* ic, enum AVMediaType type, int wanted_stream_nb, int related_stream, const struct AVCodec** decoder_ret, int flags);
   typedef int(CDECL* fn_av_read_frame)(AVFormatContext* s, AVPacket* pkt);
   typedef int(CDECL* fn_av_seek_frame)(AVFormatContext* s, int stream_index, int64_t timestamp, int flags);
   typedef int(CDECL* fn_avformat_open_input)(AVFormatContext** ps, const char* url, const AVInputFormat* fmt, AVDictionary** options);
   typedef void(CDECL* fn_avformat_close_input)(AVFormatContext** s);
   typedef int(CDECL* fn_avformat_find_stream_info)(AVFormatContext* ic, AVDictionary** options);

   typedef AVPacket*(CDECL* fn_av_packet_alloc)(void);
   typedef void(CDECL* fn_av_packet_free)(AVPacket** pkt);
   typedef void(CDECL* fn_av_packet_unref)(AVPacket* pkt);
   typedef AVCodecContext*(CDECL* fn_avcodec_alloc_context3)(const AVCodec* codec);
   typedef const AVCodec*(CDECL* fn_avcodec_find_decoder)(enum AVCodecID id);
   typedef void(CDECL* fn_avcodec_flush_buffers)(AVCodecContext* avctx);
   typedef void(CDECL* fn_avcodec_free_context)(AVCodecContext** avctx);
   typedef const char*(CDECL* fn_avcodec_get_name)(enum AVCodecID id);
   typedef int(CDECL* fn_avcodec_open2)(AVCodecContext* avctx, const AVCodec* codec, AVDictionary** options);
   typedef int(CDECL* fn_avcodec_parameters_to_context)(AVCodecContext* codec, const struct AVCodecParameters* par);
   typedef int(CDECL* fn_avcodec_receive_frame)(AVCodecContext* avctx, AVFrame* frame);
   typedef int(CDECL* fn_avcodec_send_packet)(AVCodecContext* avctx, const AVPacket* avpkt);

   typedef void(CDECL* fn_av_fast_malloc)(void* ptr, unsigned int* size, size_t min_size);
   typedef AVFrame*(CDECL* fn_av_frame_alloc)(void);
   typedef int(CDECL* fn_av_frame_copy_props)(AVFrame* dst, const AVFrame* src);
   typedef void(CDECL* fn_av_frame_free)(AVFrame** frame);
   typedef void(CDECL* fn_av_free)(void* ptr);
   typedef int(CDECL* fn_av_image_fill_arrays)(uint8_t* dst_data[4], int dst_linesize[4], const uint8_t* src, enum AVPixelFormat pix_fmt, int width, int height, int align);
   typedef int(CDECL* fn_av_image_get_buffer_size)(enum AVPixelFormat pix_fmt, int width, int height, int align);
   typedef void*(CDECL* fn_av_malloc)(size_t size) av_alloc_size(1);
   typedef int(CDECL* fn_av_samples_get_buffer_size)(int* linesize, int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt, int align);

   typedef void(CDECL* fn_swr_free)(struct SwrContext** s);
   typedef int(CDECL* fn_swr_alloc_set_opts2)(struct SwrContext** ps, const AVChannelLayout* out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate, const AVChannelLayout* in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate, int log_offset, void* log_ctx);
   typedef int(CDECL* fn_swr_init)(struct SwrContext* s);
   typedef int(CDECL* fn_swr_set_compensation)(struct SwrContext* s, int sample_delta, int compensation_distance);
   typedef int(CDECL* fn_swr_convert)(struct SwrContext* s, uint8_t* const* out, int out_count, const uint8_t* const* in, int in_count);

   typedef struct SwsContext*(CDECL* fn_sws_getCachedContext)(struct SwsContext* context, int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH, enum AVPixelFormat dstFormat, int flags, SwsFilter* srcFilter, SwsFilter* dstFilter, const double* param);
   typedef int(CDECL* fn_sws_scale)(struct SwsContext* c, const uint8_t* const srcSlice[], const int srcStride[], int srcSliceY, int srcSliceH, uint8_t* const dst[], const int dstStride[]);
   typedef void(CDECL* fn_sws_freeContext)(struct SwsContext* swsContext);

   fn_av_get_bytes_per_sample _av_get_bytes_per_sample = nullptr;
   fn_av_find_best_stream _av_find_best_stream = nullptr;
   fn_av_read_frame _av_read_frame = nullptr;
   fn_av_seek_frame _av_seek_frame = nullptr;
   fn_avformat_open_input _avformat_open_input = nullptr;
   fn_avformat_close_input _avformat_close_input = nullptr;
   fn_avformat_find_stream_info _avformat_find_stream_info = nullptr;

   fn_av_packet_alloc _av_packet_alloc = nullptr;
   fn_av_packet_free _av_packet_free = nullptr;
   fn_av_packet_unref _av_packet_unref = nullptr;
   fn_avcodec_alloc_context3 _avcodec_alloc_context3 = nullptr;
   fn_avcodec_find_decoder _avcodec_find_decoder = nullptr;
   fn_avcodec_flush_buffers _avcodec_flush_buffers = nullptr;
   fn_avcodec_free_context _avcodec_free_context = nullptr;
   fn_avcodec_get_name _avcodec_get_name = nullptr;
   fn_avcodec_open2 _avcodec_open2 = nullptr;
   fn_avcodec_parameters_to_context _avcodec_parameters_to_context = nullptr;
   fn_avcodec_receive_frame _avcodec_receive_frame = nullptr;
   fn_avcodec_send_packet _avcodec_send_packet = nullptr;

   fn_av_fast_malloc _av_fast_malloc = nullptr;
   fn_av_free _av_free = nullptr;
   fn_av_frame_alloc _av_frame_alloc = nullptr;
   fn_av_frame_copy_props _av_frame_copy_props = nullptr;
   fn_av_frame_free _av_frame_free = nullptr;
   fn_av_image_fill_arrays _av_image_fill_arrays = nullptr;
   fn_av_image_get_buffer_size _av_image_get_buffer_size = nullptr;
   fn_av_malloc _av_malloc = nullptr;
   fn_av_samples_get_buffer_size _av_samples_get_buffer_size = nullptr;

   fn_swr_alloc_set_opts2 _swr_alloc_set_opts2 = nullptr;
   fn_swr_convert _swr_convert = nullptr;
   fn_swr_free _swr_free = nullptr;
   fn_swr_init _swr_init = nullptr;
   fn_swr_set_compensation _swr_set_compensation = nullptr;

   fn_sws_freeContext _sws_freeContext = nullptr;
   fn_sws_getCachedContext _sws_getCachedContext = nullptr;
   fn_sws_scale _sws_scale = nullptr;

private:
   LibAV() {
   }

   void Load()
   {
      if (isLoaded)
         return;

      isLoaded = true;

   #ifdef _WIN32
      HINSTANCE hinstLib;
      #if (INTPTR_MAX == INT32_MAX)
      bool x64 = false;
      #else
      bool x64 = true;
      #endif

      HMODULE hm = nullptr;
      if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, _T("PluginLoad"), &hm) == 0)
         return;
      TCHAR path[MAX_PATH];
      if (GetModuleFileName(hm, path, MAX_PATH) == 0)
         return;
      #ifdef _UNICODE
      std::wstring basepath(path);
      #else
      std::string basepath(path);
      #endif
      basepath.erase(basepath.find_last_of(_T("\\/")));
      basepath += _T('\\');

      hinstLib = LoadLibraryEx((basepath + (x64 ? _T("avcodec64-61.dll"): _T("avcodec-61.dll"))).c_str(), NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
      if (hinstLib)
      {
         _av_packet_alloc = reinterpret_cast<fn_av_packet_alloc>(GetProcAddress(hinstLib, "av_packet_alloc"));
         _av_packet_free = reinterpret_cast<fn_av_packet_free>(GetProcAddress(hinstLib, "av_packet_free"));
         _av_packet_unref = reinterpret_cast<fn_av_packet_unref>(GetProcAddress(hinstLib, "av_packet_unref"));
         _avcodec_alloc_context3 = reinterpret_cast<fn_avcodec_alloc_context3>(GetProcAddress(hinstLib, "avcodec_alloc_context3"));
         _avcodec_find_decoder = reinterpret_cast<fn_avcodec_find_decoder>(GetProcAddress(hinstLib, "avcodec_find_decoder"));
         _avcodec_flush_buffers = reinterpret_cast<fn_avcodec_flush_buffers>(GetProcAddress(hinstLib, "avcodec_flush_buffers"));
         _avcodec_free_context = reinterpret_cast<fn_avcodec_free_context>(GetProcAddress(hinstLib, "avcodec_free_context"));
         _avcodec_get_name = reinterpret_cast<fn_avcodec_get_name>(GetProcAddress(hinstLib, "avcodec_get_name"));
         _avcodec_open2 = reinterpret_cast<fn_avcodec_open2>(GetProcAddress(hinstLib, "avcodec_open2"));
         _avcodec_parameters_to_context = reinterpret_cast<fn_avcodec_parameters_to_context>(GetProcAddress(hinstLib, "avcodec_parameters_to_context"));
         _avcodec_receive_frame = reinterpret_cast<fn_avcodec_receive_frame>(GetProcAddress(hinstLib, "avcodec_receive_frame"));
         _avcodec_send_packet = reinterpret_cast<fn_avcodec_send_packet>(GetProcAddress(hinstLib, "avcodec_send_packet"));
         isLoaded &= _av_packet_alloc && _av_packet_free && _av_packet_unref && _avcodec_alloc_context3 && _avcodec_find_decoder && _avcodec_flush_buffers && _avcodec_free_context
            && _avcodec_get_name && _avcodec_open2 && _avcodec_parameters_to_context && _avcodec_receive_frame && _avcodec_send_packet;
      }

      hinstLib = LoadLibraryEx((basepath + (x64 ? _T("avformat64-61.dll") : _T("avformat-61.dll"))).c_str(), NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
      if (hinstLib)
      {
         _av_find_best_stream = reinterpret_cast<fn_av_find_best_stream>(GetProcAddress(hinstLib, "av_find_best_stream"));
         _av_read_frame = reinterpret_cast<fn_av_read_frame>(GetProcAddress(hinstLib, "av_read_frame"));
         _av_seek_frame = reinterpret_cast<fn_av_seek_frame>(GetProcAddress(hinstLib, "av_seek_frame"));
         _avformat_open_input = reinterpret_cast<fn_avformat_open_input>(GetProcAddress(hinstLib, "avformat_open_input"));
         _avformat_close_input = reinterpret_cast<fn_avformat_close_input>(GetProcAddress(hinstLib, "avformat_close_input"));
         _avformat_find_stream_info = reinterpret_cast<fn_avformat_find_stream_info>(GetProcAddress(hinstLib, "avformat_find_stream_info"));
         isLoaded &= _av_find_best_stream && _av_read_frame && _av_seek_frame && _avformat_open_input && _avformat_close_input;
      }

      hinstLib = LoadLibraryEx((basepath + (x64 ? _T("avutil64-59.dll") : _T("avutil-59.dll"))).c_str(), NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
      if (hinstLib)
      {
         _av_get_bytes_per_sample = reinterpret_cast<fn_av_get_bytes_per_sample>(GetProcAddress(hinstLib, "av_get_bytes_per_sample"));
         _av_fast_malloc = reinterpret_cast<fn_av_fast_malloc>(GetProcAddress(hinstLib, "av_fast_malloc"));
         _av_frame_alloc = reinterpret_cast<fn_av_frame_alloc>(GetProcAddress(hinstLib, "av_frame_alloc"));
         _av_frame_copy_props = reinterpret_cast<fn_av_frame_copy_props>(GetProcAddress(hinstLib, "av_frame_copy_props"));
         _av_frame_free = reinterpret_cast<fn_av_frame_free>(GetProcAddress(hinstLib, "av_frame_free"));
         _av_free = reinterpret_cast<fn_av_free>(GetProcAddress(hinstLib, "av_free"));
         _av_image_fill_arrays = reinterpret_cast<fn_av_image_fill_arrays>(GetProcAddress(hinstLib, "av_image_fill_arrays"));
         _av_image_get_buffer_size = reinterpret_cast<fn_av_image_get_buffer_size>(GetProcAddress(hinstLib, "av_image_get_buffer_size"));
         _av_malloc = reinterpret_cast<fn_av_malloc>(GetProcAddress(hinstLib, "av_malloc"));
         _av_samples_get_buffer_size = reinterpret_cast<fn_av_samples_get_buffer_size>(GetProcAddress(hinstLib, "av_samples_get_buffer_size"));
         isLoaded &= _av_get_bytes_per_sample && _av_fast_malloc && _av_frame_alloc && _av_frame_copy_props && _av_frame_free && _av_free && _av_image_fill_arrays && _av_image_get_buffer_size && _av_malloc && _av_samples_get_buffer_size;
      }

      hinstLib = LoadLibraryEx((basepath + (x64 ? _T("swresample64-5.dll") : _T("swresample-5.dll"))).c_str(), NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
      if (hinstLib)
      {
         _swr_alloc_set_opts2 = reinterpret_cast<fn_swr_alloc_set_opts2>(GetProcAddress(hinstLib, "swr_alloc_set_opts2"));
         _swr_convert = reinterpret_cast<fn_swr_convert>(GetProcAddress(hinstLib, "swr_convert"));
         _swr_free = reinterpret_cast<fn_swr_free>(GetProcAddress(hinstLib, "swr_free"));
         _swr_init = reinterpret_cast<fn_swr_init>(GetProcAddress(hinstLib, "swr_init"));
         _swr_set_compensation = reinterpret_cast<fn_swr_set_compensation>(GetProcAddress(hinstLib, "swr_set_compensation"));
         isLoaded &= _swr_alloc_set_opts2 && _swr_convert && _swr_free && _swr_init && _swr_set_compensation;
      }

      hinstLib = LoadLibraryEx((basepath + (x64 ? _T("swscale64-8.dll") : _T("swscale-8.dll"))).c_str(), NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
      if (hinstLib)
      {
         _sws_freeContext = reinterpret_cast<fn_sws_freeContext>(GetProcAddress(hinstLib, "sws_freeContext"));
         _sws_getCachedContext = reinterpret_cast<fn_sws_getCachedContext>(GetProcAddress(hinstLib, "sws_getCachedContext"));
         _sws_scale = reinterpret_cast<fn_sws_scale>(GetProcAddress(hinstLib, "sws_scale"));
         isLoaded &= _sws_freeContext && _sws_getCachedContext && _sws_scale;
      }
   #else
      _av_packet_alloc = &av_packet_alloc;
      _av_packet_free = &av_packet_free;
      _av_packet_unref = &av_packet_unref;
      _avcodec_alloc_context3 = &avcodec_alloc_context3;
      _avcodec_find_decoder = &avcodec_find_decoder;
      _avcodec_flush_buffers = &avcodec_flush_buffers;
      _avcodec_free_context = &avcodec_free_context;
      _avcodec_get_name = &avcodec_get_name;
      _avcodec_open2 = &avcodec_open2;
      _avcodec_parameters_to_context = &avcodec_parameters_to_context;
      _avcodec_receive_frame = &avcodec_receive_frame;
      _avcodec_send_packet = &avcodec_send_packet;

      _av_get_bytes_per_sample = &av_get_bytes_per_sample;
      _av_find_best_stream = &av_find_best_stream;
      _av_read_frame = &av_read_frame;
      _av_seek_frame = &av_seek_frame;
      _avformat_open_input = &avformat_open_input;
      _avformat_close_input = &avformat_close_input;
      _avformat_find_stream_info = &avformat_find_stream_info;

      _av_fast_malloc = &av_fast_malloc;
      _av_frame_alloc = &av_frame_alloc;
      _av_frame_copy_props = &av_frame_copy_props;
      _av_frame_free = &av_frame_free;
      _av_free = &av_free;
      _av_image_fill_arrays = &av_image_fill_arrays;
      _av_image_get_buffer_size = &av_image_get_buffer_size;
      _av_malloc = &av_malloc;
      _av_samples_get_buffer_size = &av_samples_get_buffer_size;

      _swr_alloc_set_opts2 = &swr_alloc_set_opts2;
      _swr_convert = &swr_convert;
      _swr_free = &swr_free;
      _swr_init = &swr_init;
      _swr_set_compensation = &swr_set_compensation;

      _sws_freeContext = &sws_freeContext;
      _sws_getCachedContext = &sws_getCachedContext;
      _sws_scale = &sws_scale;
   #endif
   }

};

}
