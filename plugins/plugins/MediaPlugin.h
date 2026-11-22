// license:GPLv3+

#pragma once

#include "MsgPlugin.h"

///////////////////////////////////////////////////////////////////////////////
// Media decoding API
//
// This API is independent of the host application. It is acquired through a
// message query: broadcast MEDIAPI_MSG_GET_API under the MEDIAPI_NAMESPACE
// namespace with a pointer to a MediaAPI* to receive the function table. It
// gives plugins access to the host's media decoding. A decoding session is
// created by calling Open with the UTF-8 path of a media file, then driven by
// calling Read which returns the next decoded unit (a video frame or an audio
// sample chunk), in demux order, until the end of the stream is reached.
//
// Sessions are synchronous and single-threaded: no decoding thread is created
// by the host, and the plugin is expected to call all session functions from a
// single thread of its own (Open/Close may be called from a different thread
// than Read calls).
//
// Decoded video frames are returned as opaque MediaFrame handles. The frame
// content may then be materialized either as raw pixels for custom processing
// (GetFramePixels), or through the host's rendering API for display (for
// example a texture-from-frame function, suitable for render commands).
// The session owns the materialization, which allows the host to eventually
// provide GPU-resident frames, only paying the GPU to CPU transfer price when
// GetFramePixels is actually used.

#define MEDIAPI_NAMESPACE "Media" // Namespace used for all media API message definitions
#define MEDIAPI_MSG_GET_API "GetAPI:1" // Get the media plugin API

typedef void* Media; // Opaque decoding session, owned by the host
typedef void* MediaFrame; // Opaque decoded video frame, owned by the session.
// Backing is implementation-defined; always ReleaseFrame when done.

#define MEDIA_OPEN_DEFAULT 0 // Reserved Open flags (e.g. GPU decoding preference), must be 0

typedef struct MediaInfo
{
   double duration; // [s], 0 if unknown
   double startTime; // [s] start offset of the clock stream (video if present, otherwise audio)
   double videoDuration; // [s] video stream duration, 0 if unknown or no video stream
   unsigned int videoWidth; // Native frame size, 0 when there is no video stream
   unsigned int videoHeight;
   double frameRate; // Average frame rate, 0 when unknown or no video stream
   unsigned int audioSampleRate; // 0 when there is no audio stream
   unsigned int audioChannels;
} MediaInfo;

// MediaOutput::type values
#define MEDIA_OUT_NONE 0
#define MEDIA_OUT_VIDEO 1 // videoFrame is set
#define MEDIA_OUT_AUDIO 2 // audio* fields are set
#define MEDIA_OUT_EOF 3 // End of stream, after both decoders have been fully drained
#define MEDIA_OUT_ERROR 4 // Unrecoverable error

typedef struct MediaOutput
{
   int type;
   double pts; // Presentation timestamp [s]
   MediaFrame videoFrame; // MEDIA_OUT_VIDEO only; release with ReleaseFrame
   const uint8_t* audioData; // MEDIA_OUT_AUDIO only; session owned, valid until next Read
   unsigned int audioSize; // [bytes]
   int audioSampleFormat; // Same as CTLPI_AUDIO_FORMAT_SAMPLE_xxx (0 = int16, 1 = float32)
   unsigned int audioChannels; // Always 2 (stereo)
   unsigned int audioSampleRate; // Stream native rate
} MediaOutput;

typedef struct MediaAPI
{
   int version; // Must be 1. Included to allow extending the API with new functions at a later point in time

   // Open a media file, selecting the best video and audio streams.
   // Returns nullptr on failure (file not found, no decodable stream).
   Media(MSGPIAPI* Open)(const char* path, unsigned int flags);
   void(MSGPIAPI* GetInfo)(Media media, MediaInfo* info);
   // Read demuxed packets until one decoded unit is available. After reaching the
   // end of the container, both decoders are drained before MEDIA_OUT_EOF is
   // reported. Returns 0 on success (check output.type), a negative value on error.
   int(MSGPIAPI* Read)(Media media, MediaOutput* output);
   // Seek backward to a position <= position [s], flushing both decoders.
   // Returns 0 on success, a negative value on error.
   int(MSGPIAPI* Seek)(Media media, double position);
   // Override the decoded frame output size (0,0 = native size). May be called at
   // any time, taking effect on the next materialized frame.
   void(MSGPIAPI* SetVideoFrameSize)(Media media, unsigned int width, unsigned int height);
   // Return a frame to the session. Frames must not outlive their session.
   void(MSGPIAPI* ReleaseFrame)(Media media, MediaFrame frame);
   // Get a decoded frame as tightly packed sRGBA8 pixels (width * 4 bytes per row).
   // Meant for further processing; for GPU-resident frames this forces a GPU to
   // CPU readback, therefore display-only users should use the host's texture
   // materialization path.
   // The returned pointer is session owned and valid until the frame is released
   // or the session closed. Returns 0 on success.
   int(MSGPIAPI* GetFramePixels)(Media media, MediaFrame frame, const uint8_t** data, unsigned int* width, unsigned int* height);
   // Close the session. Outstanding frames and audio pointers become invalid.
   void(MSGPIAPI* Close)(Media media);
} MediaAPI;
