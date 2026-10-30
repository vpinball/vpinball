// license:GPLv3+

#pragma once

// Opus sounds (Ogg Opus), decoded by libopusfile. miniaudio has no built-in Opus
// decoder: this adds one as a custom decoding backend, tried after its own WAV, FLAC,
// MP3 and Vorbis decoders. Include it after miniaudio.h.

namespace VPX
{

// Adds the Opus decoding backend to a decoder configuration
void AddOpusDecoder(ma_decoder_config& config);

}
