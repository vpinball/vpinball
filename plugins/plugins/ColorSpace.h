// license:GPLv3+

#pragma once

// sRGB transfer function helpers, shared by VPX and the plugins that move DMD frames around.
//
// Display frame formats deliberately mix colorspaces: CTLPI_DISPLAY_FORMAT_LUM32F is linear luminance,
// SRGB888/SRGB565 is gamma encoded, and the legacy 0..100 brightness percentage of the VPinMAME controller is gamma encoded too.
//
// Every conversion here is a table lookup, exact and identical on every platform: the tables are precomputed from the sRGB curve in
// high precision (no powf, whose rounding varies between C libraries), each value correctly rounded to float. The encodes index a 4096
// entry table by linear value, landing on the right output code or one either side, then step once onto it exactly - see LUT_SIZE for
// why one step always suffices, and the sentinels for why it is branchless

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <limits>

namespace VPXColorSpace
{

namespace detail
{

// Generated with a 60 digit evaluation of the sRGB EOTF: 12.92 below 0.04045, ((c + 0.055) / 1.055)^2.4 above.
// The decodes are rounded to the nearest float, the thresholds up to the smallest float at or above the exact code boundary
// (an encode of exactly that float rounds to the upper code, as no float lies on a boundary)

// Linear value of each 8 bit code: EOTF(i / 255)
inline constexpr float kSRGBToLinear[256] = {
   0x0p+0f, 0x1.3e4568p-12f, 0x1.3e4568p-11f, 0x1.dd681cp-11f, 0x1.3e4568p-10f, 0x1.8dd6c2p-10f, 0x1.dd681cp-10f, 0x1.167cbap-9f,
   0x1.3e4568p-9f, 0x1.660e14p-9f, 0x1.8dd6c2p-9f, 0x1.b6a31cp-9f, 0x1.e1e31ep-9f, 0x1.07c38cp-8f, 0x1.1fcc2cp-8f, 0x1.390ffap-8f,
   0x1.53936cp-8f, 0x1.6f5adep-8f, 0x1.8c6a94p-8f, 0x1.aac6c0p-8f, 0x1.ca7382p-8f, 0x1.eb74e2p-8f, 0x1.06e76cp-7f, 0x1.18c2a6p-7f,
   0x1.2b4e0ap-7f, 0x1.3e8b7cp-7f, 0x1.527cd6p-7f, 0x1.6723eep-7f, 0x1.7c8292p-7f, 0x1.929a88p-7f, 0x1.a96d92p-7f, 0x1.c0fd68p-7f,
   0x1.d94bbep-7f, 0x1.f25a44p-7f, 0x1.061552p-6f, 0x1.135f3ep-6f, 0x1.210bb8p-6f, 0x1.2f1b8cp-6f, 0x1.3d8f84p-6f, 0x1.4c6866p-6f,
   0x1.5ba6fap-6f, 0x1.6b4c04p-6f, 0x1.7b5842p-6f, 0x1.8bcc74p-6f, 0x1.9ca958p-6f, 0x1.adefaap-6f, 0x1.bfa020p-6f, 0x1.d1bb74p-6f,
   0x1.e4425ap-6f, 0x1.f73586p-6f, 0x1.054ad4p-5f, 0x1.0f31bap-5f, 0x1.194fccp-5f, 0x1.23a55ep-5f, 0x1.2e32c8p-5f, 0x1.38f860p-5f,
   0x1.43f678p-5f, 0x1.4f2d64p-5f, 0x1.5a9d76p-5f, 0x1.664702p-5f, 0x1.722a56p-5f, 0x1.7e47c8p-5f, 0x1.8a9fa4p-5f, 0x1.97323ap-5f,
   0x1.a3ffd8p-5f, 0x1.b108d0p-5f, 0x1.be4d6cp-5f, 0x1.cbcdfap-5f, 0x1.d98ac6p-5f, 0x1.e7841cp-5f, 0x1.f5ba48p-5f, 0x1.0216cap-4f,
   0x1.096f26p-4f, 0x1.10e65cp-4f, 0x1.187c90p-4f, 0x1.2031e8p-4f, 0x1.280688p-4f, 0x1.2ffa92p-4f, 0x1.380e2ap-4f, 0x1.404174p-4f,
   0x1.489494p-4f, 0x1.5107acp-4f, 0x1.599adep-4f, 0x1.624e4ep-4f, 0x1.6b221ep-4f, 0x1.741670p-4f, 0x1.7d2b66p-4f, 0x1.866120p-4f,
   0x1.8fb7c0p-4f, 0x1.992f68p-4f, 0x1.a2c83ap-4f, 0x1.ac8256p-4f, 0x1.b65ddcp-4f, 0x1.c05aecp-4f, 0x1.ca79a8p-4f, 0x1.d4ba30p-4f,
   0x1.df1ca2p-4f, 0x1.e9a120p-4f, 0x1.f447cap-4f, 0x1.ff10bcp-4f, 0x1.04fe0cp-3f, 0x1.0a84fep-3f, 0x1.101d44p-3f, 0x1.15c6eep-3f,
   0x1.1b8208p-3f, 0x1.214ea6p-3f, 0x1.272cd4p-3f, 0x1.2d1ca2p-3f, 0x1.331e1ep-3f, 0x1.393158p-3f, 0x1.3f5660p-3f, 0x1.458d42p-3f,
   0x1.4bd60ep-3f, 0x1.5230d4p-3f, 0x1.589da0p-3f, 0x1.5f1c84p-3f, 0x1.65ad8ap-3f, 0x1.6c50c4p-3f, 0x1.73063ep-3f, 0x1.79ce06p-3f,
   0x1.80a82ep-3f, 0x1.8794c0p-3f, 0x1.8e93ccp-3f, 0x1.95a55ep-3f, 0x1.9cc986p-3f, 0x1.a40052p-3f, 0x1.ab49cep-3f, 0x1.b2a60ap-3f,
   0x1.ba1512p-3f, 0x1.c196f4p-3f, 0x1.c92bbep-3f, 0x1.d0d37cp-3f, 0x1.d88e3ep-3f, 0x1.e05c0ep-3f, 0x1.e83cfcp-3f, 0x1.f03116p-3f,
   0x1.f83866p-3f, 0x1.00297ep-2f, 0x1.044072p-2f, 0x1.086116p-2f, 0x1.0c8b70p-2f, 0x1.10bf86p-2f, 0x1.14fd60p-2f, 0x1.194502p-2f,
   0x1.1d9676p-2f, 0x1.21f1bep-2f, 0x1.2656e4p-2f, 0x1.2ac5ecp-2f, 0x1.2f3edep-2f, 0x1.33c1c0p-2f, 0x1.384e98p-2f, 0x1.3ce56cp-2f,
   0x1.418642p-2f, 0x1.463120p-2f, 0x1.4ae60ep-2f, 0x1.4fa510p-2f, 0x1.546e2cp-2f, 0x1.59416cp-2f, 0x1.5e1ed0p-2f, 0x1.630664p-2f,
   0x1.67f82ap-2f, 0x1.6cf428p-2f, 0x1.71fa68p-2f, 0x1.770aecp-2f, 0x1.7c25bcp-2f, 0x1.814adcp-2f, 0x1.867a54p-2f, 0x1.8bb428p-2f,
   0x1.90f860p-2f, 0x1.964700p-2f, 0x1.9ba010p-2f, 0x1.a10394p-2f, 0x1.a67192p-2f, 0x1.abea10p-2f, 0x1.b16d14p-2f, 0x1.b6faa4p-2f,
   0x1.bc92c6p-2f, 0x1.c2357ep-2f, 0x1.c7e2d2p-2f, 0x1.cd9acap-2f, 0x1.d35d6ap-2f, 0x1.d92ab6p-2f, 0x1.df02b8p-2f, 0x1.e4e570p-2f,
   0x1.ead2e8p-2f, 0x1.f0cb26p-2f, 0x1.f6ce2cp-2f, 0x1.fcdc00p-2f, 0x1.017a56p-1f, 0x1.048c18p-1f, 0x1.07a34ap-1f, 0x1.0abfeep-1f,
   0x1.0de208p-1f, 0x1.11099ap-1f, 0x1.1436a8p-1f, 0x1.176932p-1f, 0x1.1aa13ep-1f, 0x1.1ddecap-1f, 0x1.2121dep-1f, 0x1.246a7ap-1f,
   0x1.27b8a0p-1f, 0x1.2b0c54p-1f, 0x1.2e6598p-1f, 0x1.31c46ep-1f, 0x1.3528dcp-1f, 0x1.3892e0p-1f, 0x1.3c0280p-1f, 0x1.3f77bcp-1f,
   0x1.42f29ap-1f, 0x1.467318p-1f, 0x1.49f93ep-1f, 0x1.4d850ap-1f, 0x1.511682p-1f, 0x1.54ada4p-1f, 0x1.584a78p-1f, 0x1.5becfep-1f,
   0x1.5f9538p-1f, 0x1.634328p-1f, 0x1.66f6d4p-1f, 0x1.6ab03ap-1f, 0x1.6e6f60p-1f, 0x1.723448p-1f, 0x1.75fef4p-1f, 0x1.79cf64p-1f,
   0x1.7da59ep-1f, 0x1.8181a4p-1f, 0x1.856378p-1f, 0x1.894b1cp-1f, 0x1.8d3892p-1f, 0x1.912bdep-1f, 0x1.952500p-1f, 0x1.9923fep-1f,
   0x1.9d28d8p-1f, 0x1.a13392p-1f, 0x1.a5442cp-1f, 0x1.a95aacp-1f, 0x1.ad7710p-1f, 0x1.b1995ep-1f, 0x1.b5c198p-1f, 0x1.b9efbep-1f,
   0x1.be23d4p-1f, 0x1.c25ddep-1f, 0x1.c69ddcp-1f, 0x1.cae3d2p-1f, 0x1.cf2fc0p-1f, 0x1.d381aap-1f, 0x1.d7d994p-1f, 0x1.dc377ep-1f,
   0x1.e09b6ap-1f, 0x1.e5055cp-1f, 0x1.e97556p-1f, 0x1.edeb5cp-1f, 0x1.f2676cp-1f, 0x1.f6e98cp-1f, 0x1.fb71bcp-1f, 0x1.000000p+0f
};

// Linear value of each 0..255 percentage: EOTF(i / 100), extrapolated above 100
inline constexpr float kPercentToLinear[256] = {
   0x0p+0f, 0x1.95cbb0p-11f, 0x1.95cbb0p-10f, 0x1.3058c4p-9f, 0x1.95cbb0p-9f, 0x1.01f21cp-8f, 0x1.40e274p-8f, 0x1.87f988p-8f,
   0x1.d77e28p-8f, 0x1.17d9eap-7f, 0x1.486d8ep-7f, 0x1.7d98f4p-7f, 0x1.b779e8p-7f, 0x1.f62d20p-7f, 0x1.1ce72ep-6f, 0x1.413c3ep-6f,
   0x1.6822c6p-6f, 0x1.91a76cp-6f, 0x1.bdd67cp-6f, 0x1.ecbbf0p-6f, 0x1.0f31bap-5f, 0x1.296c38p-5f, 0x1.4512fep-5f, 0x1.622b82p-5f,
   0x1.80bb12p-5f, 0x1.a0c6e4p-5f, 0x1.c25414p-5f, 0x1.e567a4p-5f, 0x1.05033ep-4f, 0x1.181ab8p-4f, 0x1.2bfc9cp-4f, 0x1.40ab40p-4f,
   0x1.5628eep-4f, 0x1.6c77e4p-4f, 0x1.839a5ap-4f, 0x1.9b9280p-4f, 0x1.b4627ap-4f, 0x1.ce0c64p-4f, 0x1.e89258p-4f, 0x1.01fb30p-3f,
   0x1.101d44p-3f, 0x1.1eb066p-3f, 0x1.2db596p-3f, 0x1.3d2dcap-3f, 0x1.4d19f8p-3f, 0x1.5d7b18p-3f, 0x1.6e5216p-3f, 0x1.7f9fe2p-3f,
   0x1.916566p-3f, 0x1.a3a38ap-3f, 0x1.b65b34p-3f, 0x1.c98d46p-3f, 0x1.dd3aa4p-3f, 0x1.f16428p-3f, 0x1.030558p-2f, 0x1.0d978cp-2f,
   0x1.18691ap-2f, 0x1.237a6cp-2f, 0x1.2ecbeep-2f, 0x1.3a5e08p-2f, 0x1.463120p-2f, 0x1.5245a0p-2f, 0x1.5e9beap-2f, 0x1.6b3466p-2f,
   0x1.780f76p-2f, 0x1.852d7cp-2f, 0x1.928edep-2f, 0x1.a033fcp-2f, 0x1.ae1d34p-2f, 0x1.bc4aeap-2f, 0x1.cabd7ap-2f, 0x1.d97542p-2f,
   0x1.e872a2p-2f, 0x1.f7b5f4p-2f, 0x1.039fccp-1f, 0x1.0b87f2p-1f, 0x1.13939cp-1f, 0x1.1bc2f4p-1f, 0x1.24162ap-1f, 0x1.2c8d68p-1f,
   0x1.3528dcp-1f, 0x1.3de8b0p-1f, 0x1.46cd0ep-1f, 0x1.4fd626p-1f, 0x1.59041ep-1f, 0x1.625724p-1f, 0x1.6bcf5ep-1f, 0x1.756cfcp-1f,
   0x1.7f3022p-1f, 0x1.8918fcp-1f, 0x1.9327b4p-1f, 0x1.9d5c72p-1f, 0x1.a7b75ep-1f, 0x1.b238a0p-1f, 0x1.bce062p-1f, 0x1.c7aecap-1f,
   0x1.d2a400p-1f, 0x1.ddc02ep-1f, 0x1.e90378p-1f, 0x1.f46e06p-1f, 0x1.000000p+0f, 0x1.05dcc6p+0f, 0x1.0bcd66p+0f, 0x1.11d1f6p+0f,
   0x1.17ea88p+0f, 0x1.1e1730p+0f, 0x1.2457fcp+0f, 0x1.2aad04p+0f, 0x1.31165ap+0f, 0x1.37940cp+0f, 0x1.3e2632p+0f, 0x1.44ccdap+0f,
   0x1.4b8818p+0f, 0x1.5257fep+0f, 0x1.593c9cp+0f, 0x1.603608p+0f, 0x1.674450p+0f, 0x1.6e6786p+0f, 0x1.759fbep+0f, 0x1.7ced08p+0f,
   0x1.844f76p+0f, 0x1.8bc718p+0f, 0x1.935400p+0f, 0x1.9af640p+0f, 0x1.a2ade8p+0f, 0x1.aa7b0ap+0f, 0x1.b25db6p+0f, 0x1.ba55fcp+0f,
   0x1.c263f0p+0f, 0x1.ca87a0p+0f, 0x1.d2c11cp+0f, 0x1.db1078p+0f, 0x1.e375c2p+0f, 0x1.ebf10ap+0f, 0x1.f48262p+0f, 0x1.fd29dap+0f,
   0x1.02f3c0p+1f, 0x1.075db4p+1f, 0x1.0bd2cep+1f, 0x1.10531ap+1f, 0x1.14de9ep+1f, 0x1.197560p+1f, 0x1.1e176ap+1f, 0x1.22c4c6p+1f,
   0x1.277d76p+1f, 0x1.2c4188p+1f, 0x1.311100p+1f, 0x1.35ebe6p+1f, 0x1.3ad244p+1f, 0x1.3fc420p+1f, 0x1.44c180p+1f, 0x1.49ca70p+1f,
   0x1.4edef4p+1f, 0x1.53ff14p+1f, 0x1.592ad8p+1f, 0x1.5e6248p+1f, 0x1.63a56cp+1f, 0x1.68f448p+1f, 0x1.6e4ee8p+1f, 0x1.73b552p+1f,
   0x1.79278cp+1f, 0x1.7ea59ep+1f, 0x1.842f8ep+1f, 0x1.89c566p+1f, 0x1.8f672cp+1f, 0x1.9514e8p+1f, 0x1.9acea0p+1f, 0x1.a0945ap+1f,
   0x1.a66620p+1f, 0x1.ac43f8p+1f, 0x1.b22deap+1f, 0x1.b823fap+1f, 0x1.be2632p+1f, 0x1.c43498p+1f, 0x1.ca4f34p+1f, 0x1.d0760cp+1f,
   0x1.d6a926p+1f, 0x1.dce88cp+1f, 0x1.e33442p+1f, 0x1.e98c50p+1f, 0x1.eff0bcp+1f, 0x1.f6618ep+1f, 0x1.fcdecep+1f, 0x1.01b440p+2f,
   0x1.04ff56p+2f, 0x1.0850acp+2f, 0x1.0ba846p+2f, 0x1.0f0628p+2f, 0x1.126a54p+2f, 0x1.15d4cep+2f, 0x1.19459ap+2f, 0x1.1cbcb8p+2f,
   0x1.203a30p+2f, 0x1.23be02p+2f, 0x1.274834p+2f, 0x1.2ad8c6p+2f, 0x1.2e6fbep+2f, 0x1.320d1ep+2f, 0x1.35b0eap+2f, 0x1.395b24p+2f,
   0x1.3d0bd0p+2f, 0x1.40c2f0p+2f, 0x1.44808ap+2f, 0x1.48449ep+2f, 0x1.4c0f32p+2f, 0x1.4fe048p+2f, 0x1.53b7e4p+2f, 0x1.579606p+2f,
   0x1.5b7ab6p+2f, 0x1.5f65f4p+2f, 0x1.6357c2p+2f, 0x1.675026p+2f, 0x1.6b4f22p+2f, 0x1.6f54bap+2f, 0x1.7360f0p+2f, 0x1.7773c6p+2f,
   0x1.7b8d40p+2f, 0x1.7fad64p+2f, 0x1.83d430p+2f, 0x1.8801acp+2f, 0x1.8c35d8p+2f, 0x1.9070b6p+2f, 0x1.94b24cp+2f, 0x1.98fa9cp+2f,
   0x1.9d49aap+2f, 0x1.a19f76p+2f, 0x1.a5fc06p+2f, 0x1.aa5f5cp+2f, 0x1.aec97ap+2f, 0x1.b33a64p+2f, 0x1.b7b21cp+2f, 0x1.bc30a6p+2f,
   0x1.c0b606p+2f, 0x1.c5423ep+2f, 0x1.c9d54ep+2f, 0x1.ce6f3ep+2f, 0x1.d3100ep+2f, 0x1.d7b7c0p+2f, 0x1.dc665ap+2f, 0x1.e11bdep+2f,
   0x1.e5d84cp+2f, 0x1.ea9baap+2f, 0x1.ef65fap+2f, 0x1.f43740p+2f, 0x1.f90f7cp+2f, 0x1.fdeeb4p+2f, 0x1.016a74p+3f, 0x1.03e10ep+3f,
   0x1.065b2ap+3f, 0x1.08d8cap+3f, 0x1.0b59ecp+3f, 0x1.0dde96p+3f, 0x1.1066c6p+3f, 0x1.12f280p+3f, 0x1.1581c2p+3f, 0x1.181492p+3f
};

// Smallest linear value encoding to code i + 1: EOTF((i + 0.5) / 255) rounded up
inline constexpr float kByteThreshold[255] = {
   0x1.3e4568p-13f, 0x1.dd681cp-12f, 0x1.8dd6c2p-11f, 0x1.167cbcp-10f, 0x1.660e16p-10f, 0x1.b59f70p-10f, 0x1.029866p-9f, 0x1.2a6112p-9f,
   0x1.5229bep-9f, 0x1.79f26cp-9f, 0x1.a1e5a2p-9f, 0x1.cbf736p-9f, 0x1.f86806p-9f, 0x1.13a0c0p-8f, 0x1.2c4666p-8f, 0x1.46297ap-8f,
   0x1.614e62p-8f, 0x1.7db96ep-8f, 0x1.9b6edap-8f, 0x1.ba72cep-8f, 0x1.dac95ep-8f, 0x1.fc768cp-8f, 0x1.0fbf22p-7f, 0x1.21f236p-7f,
   0x1.34d664p-7f, 0x1.486d90p-7f, 0x1.5cb98ep-7f, 0x1.71bc34p-7f, 0x1.877748p-7f, 0x1.9dec90p-7f, 0x1.b51dc8p-7f, 0x1.cd0ca8p-7f,
   0x1.e5bae2p-7f, 0x1.ff2a1ep-7f, 0x1.0cae04p-6f, 0x1.1a291ep-6f, 0x1.28072cp-6f, 0x1.3648f8p-6f, 0x1.44ef4cp-6f, 0x1.53faf0p-6f,
   0x1.636ca6p-6f, 0x1.734530p-6f, 0x1.838552p-6f, 0x1.942dc6p-6f, 0x1.a53f48p-6f, 0x1.b6ba96p-6f, 0x1.c8a064p-6f, 0x1.daf16ap-6f,
   0x1.edae5cp-6f, 0x1.006bf8p-5f, 0x1.0a3768p-5f, 0x1.1439d8p-5f, 0x1.1e73a0p-5f, 0x1.28e514p-5f, 0x1.338e8ap-5f, 0x1.3e7058p-5f,
   0x1.498acep-5f, 0x1.54de44p-5f, 0x1.606b08p-5f, 0x1.6c316ep-5f, 0x1.7831c8p-5f, 0x1.846c64p-5f, 0x1.90e194p-5f, 0x1.9d91a4p-5f,
   0x1.aa7ce6p-5f, 0x1.b7a3a6p-5f, 0x1.c50630p-5f, 0x1.d2a4d4p-5f, 0x1.e07fdcp-5f, 0x1.ee9794p-5f, 0x1.fcec48p-5f, 0x1.05bf20p-4f,
   0x1.0d26e4p-4f, 0x1.14ad96p-4f, 0x1.1c5356p-4f, 0x1.24184ep-4f, 0x1.2bfc9ep-4f, 0x1.34006ap-4f, 0x1.3c23d8p-4f, 0x1.446708p-4f,
   0x1.4cca20p-4f, 0x1.554d40p-4f, 0x1.5df08ep-4f, 0x1.66b42ap-4f, 0x1.6f9836p-4f, 0x1.789cd6p-4f, 0x1.81c228p-4f, 0x1.8b0852p-4f,
   0x1.946f72p-4f, 0x1.9df7acp-4f, 0x1.a7a11ep-4f, 0x1.b16beap-4f, 0x1.bb5832p-4f, 0x1.c56614p-4f, 0x1.cf95b2p-4f, 0x1.d9e72ap-4f,
   0x1.e45aa0p-4f, 0x1.eef02ep-4f, 0x1.f9a7f8p-4f, 0x1.02410ep-3f, 0x1.07bf5cp-3f, 0x1.0d4ef6p-3f, 0x1.12efecp-3f, 0x1.18a24cp-3f,
   0x1.1e6628p-3f, 0x1.243b8ap-3f, 0x1.2a2286p-3f, 0x1.301b2ap-3f, 0x1.362584p-3f, 0x1.3c41a2p-3f, 0x1.426f96p-3f, 0x1.48af6cp-3f,
   0x1.4f0132p-3f, 0x1.5564fap-3f, 0x1.5bdad0p-3f, 0x1.6262c2p-3f, 0x1.68fce0p-3f, 0x1.6fa938p-3f, 0x1.7667d8p-3f, 0x1.7d38cep-3f,
   0x1.841c2ap-3f, 0x1.8b11f6p-3f, 0x1.921a44p-3f, 0x1.993520p-3f, 0x1.a06298p-3f, 0x1.a7a2bap-3f, 0x1.aef594p-3f, 0x1.b65b34p-3f,
   0x1.bdd3a8p-3f, 0x1.c55efcp-3f, 0x1.ccfd3ep-3f, 0x1.d4ae7cp-3f, 0x1.dc72c4p-3f, 0x1.e44a22p-3f, 0x1.ec34a4p-3f, 0x1.f43258p-3f,
   0x1.fc434ap-3f, 0x1.0233c4p-2f, 0x1.064f8ep-2f, 0x1.0a750cp-2f, 0x1.0ea444p-2f, 0x1.12dd3ap-2f, 0x1.171ff8p-2f, 0x1.1b6c82p-2f,
   0x1.1fc2e0p-2f, 0x1.242316p-2f, 0x1.288d2cp-2f, 0x1.2d012ap-2f, 0x1.317f12p-2f, 0x1.3606eep-2f, 0x1.3a98c4p-2f, 0x1.3f3498p-2f,
   0x1.43da70p-2f, 0x1.488a56p-2f, 0x1.4d444cp-2f, 0x1.52085cp-2f, 0x1.56d688p-2f, 0x1.5baedap-2f, 0x1.609154p-2f, 0x1.657e00p-2f,
   0x1.6a74e2p-2f, 0x1.6f7600p-2f, 0x1.748162p-2f, 0x1.79970ap-2f, 0x1.7eb702p-2f, 0x1.83e14ep-2f, 0x1.8915f2p-2f, 0x1.8e54f8p-2f,
   0x1.939e64p-2f, 0x1.98f23cp-2f, 0x1.9e5084p-2f, 0x1.a3b944p-2f, 0x1.a92c82p-2f, 0x1.aeaa42p-2f, 0x1.b4328cp-2f, 0x1.b9c564p-2f,
   0x1.bf62d0p-2f, 0x1.c50ad4p-2f, 0x1.cabd7ap-2f, 0x1.d07ac6p-2f, 0x1.d642bap-2f, 0x1.dc1562p-2f, 0x1.e1f2bep-2f, 0x1.e7dad6p-2f,
   0x1.edcdb0p-2f, 0x1.f3cb50p-2f, 0x1.f9d3bcp-2f, 0x1.ffe6fcp-2f, 0x1.03028ap-1f, 0x1.061704p-1f, 0x1.0930eep-1f, 0x1.0c504ep-1f,
   0x1.0f7524p-1f, 0x1.129f72p-1f, 0x1.15cf3ep-1f, 0x1.190488p-1f, 0x1.1c3f54p-1f, 0x1.1f7fa4p-1f, 0x1.22c57cp-1f, 0x1.2610dcp-1f,
   0x1.2961c8p-1f, 0x1.2cb844p-1f, 0x1.301452p-1f, 0x1.3375f4p-1f, 0x1.36dd2cp-1f, 0x1.3a49fep-1f, 0x1.3dbc6cp-1f, 0x1.413478p-1f,
   0x1.44b226p-1f, 0x1.483578p-1f, 0x1.4bbe70p-1f, 0x1.4f4d12p-1f, 0x1.52e15ep-1f, 0x1.567b5ap-1f, 0x1.5a1b06p-1f, 0x1.5dc064p-1f,
   0x1.616b7ap-1f, 0x1.651c48p-1f, 0x1.68d2d0p-1f, 0x1.6c8f16p-1f, 0x1.70511ep-1f, 0x1.7418e6p-1f, 0x1.77e674p-1f, 0x1.7bb9cap-1f,
   0x1.7f92eap-1f, 0x1.8371d6p-1f, 0x1.875690p-1f, 0x1.8b411ep-1f, 0x1.8f317ep-1f, 0x1.9327b6p-1f, 0x1.9723c6p-1f, 0x1.9b25b0p-1f,
   0x1.9f2d7ap-1f, 0x1.a33b24p-1f, 0x1.a74eb0p-1f, 0x1.ab6822p-1f, 0x1.af877cp-1f, 0x1.b3acbep-1f, 0x1.b7d7eep-1f, 0x1.bc090cp-1f,
   0x1.c0401cp-1f, 0x1.c47d20p-1f, 0x1.c8c018p-1f, 0x1.cd090ap-1f, 0x1.d157f6p-1f, 0x1.d5ace0p-1f, 0x1.da07cap-1f, 0x1.de68b4p-1f,
   0x1.e2cfa4p-1f, 0x1.e73c9ap-1f, 0x1.ebaf98p-1f, 0x1.f028a2p-1f, 0x1.f4a7bap-1f, 0x1.f92ce2p-1f, 0x1.fdb81cp-1f
};

// Smallest linear value encoding to percentage i + 1: EOTF((i + 0.5) / 100) rounded up
inline constexpr float kPercentThreshold[100] = {
   0x1.95cbb2p-12f, 0x1.3058c6p-10f, 0x1.fb3e9ep-10f, 0x1.63123cp-9f, 0x1.cae29cp-9f, 0x1.206a02p-8f, 0x1.6364a4p-8f, 0x1.aea9dcp-8f,
   0x1.013f64p-7f, 0x1.2f92aep-7f, 0x1.626e66p-7f, 0x1.99f0ecp-7f, 0x1.d6377ap-7f, 0x1.0baf26p-6f, 0x1.2ec056p-6f, 0x1.545c88p-6f,
   0x1.7c9090p-6f, 0x1.a768e6p-6f, 0x1.d4f1aep-6f, 0x1.029b5cp-5f, 0x1.1c21c8p-5f, 0x1.3711bcp-5f, 0x1.5370b4p-5f, 0x1.714414p-5f,
   0x1.909120p-5f, 0x1.b15d00p-5f, 0x1.d3acc2p-5f, 0x1.f7855ap-5f, 0x1.0e75d4p-4f, 0x1.21f238p-4f, 0x1.363a32p-4f, 0x1.4b5014p-4f,
   0x1.61361ep-4f, 0x1.77ee8ep-4f, 0x1.8f7b96p-4f, 0x1.a7df60p-4f, 0x1.c11c10p-4f, 0x1.db33bcp-4f, 0x1.f6287ap-4f, 0x1.08fe2ap-3f,
   0x1.1758a4p-3f, 0x1.2624aep-3f, 0x1.356340p-3f, 0x1.451554p-3f, 0x1.553bdcp-3f, 0x1.65d7ccp-3f, 0x1.76ea14p-3f, 0x1.88739ep-3f,
   0x1.9a7556p-3f, 0x1.acf020p-3f, 0x1.bfe4e2p-3f, 0x1.d3547ep-3f, 0x1.e73fd4p-3f, 0x1.fba7c0p-3f, 0x1.08468ep-2f, 0x1.12f862p-2f,
   0x1.1de9c4p-2f, 0x1.291b22p-2f, 0x1.348ce2p-2f, 0x1.403f6ep-2f, 0x1.4c332ep-2f, 0x1.586886p-2f, 0x1.64dfdcp-2f, 0x1.719996p-2f,
   0x1.7e9616p-2f, 0x1.8bd5bep-2f, 0x1.9958f0p-2f, 0x1.a72010p-2f, 0x1.b52b7ap-2f, 0x1.c37b92p-2f, 0x1.d210b2p-2f, 0x1.e0eb3ap-2f,
   0x1.f00b88p-2f, 0x1.ff71f6p-2f, 0x1.078f72p-1f, 0x1.0f8954p-1f, 0x1.17a6d0p-1f, 0x1.1fe812p-1f, 0x1.284d46p-1f, 0x1.30d69ap-1f,
   0x1.398438p-1f, 0x1.42564cp-1f, 0x1.4b4d02p-1f, 0x1.546884p-1f, 0x1.5da8fep-1f, 0x1.670e98p-1f, 0x1.709980p-1f, 0x1.7a49dcp-1f,
   0x1.841fd8p-1f, 0x1.8e1b9cp-1f, 0x1.983d50p-1f, 0x1.a28520p-1f, 0x1.acf332p-1f, 0x1.b787b0p-1f, 0x1.c242c0p-1f, 0x1.cd248ap-1f,
   0x1.d82d38p-1f, 0x1.e35ceep-1f, 0x1.eeb3d6p-1f, 0x1.fa3214p-1f
};

// Bins of the linear indexed encode tables. A bin must be narrower than the narrowest gap
// between two output codes, so that it can straddle at most one code boundary and a single
// +-1 step is guaranteed to reach the exact answer. The narrowest gap is at black, where the
// transfer function is at its steepest: 1/(255*12.92) = 1/3295 for the 8 bit codes and
// 1/(100*12.92) = 1/1292 for the 0..100 ones, both wider than the 1/4095 bins below
constexpr int LUT_SIZE = 4096;

// Clamp to 0..1, for out of range and NaN inputs (NaN should fail both comparisons -> 0) (maps to minss/maxss on x86)
inline float Clamp01(const float linear)
{
   const float c = (linear > 0.f) ? linear : 0.f; // NaN -> 0
   return (c < 1.f) ? c : 1.f;
}

// For a value already clamped to 0..1
inline int LutIndex(const float c) { return static_cast<int>(c * static_cast<float>(LUT_SIZE - 1) + 0.5f); }

// The encode tables, built at compile time from the thresholds
struct Tables
{
   // The thresholds with a saturating sentinel at either end, so the correction step can read one entry past each end of the
   // real range - that is what lets it fold the bounds test into the comparison instead of branching on it
   float byteThresholdPad[1 + 255 + 1] {};
   float percentThresholdPad[1 + 100 + 1] {};

   uint8_t linearToByte[LUT_SIZE] {};
   uint8_t linearToPercent[LUT_SIZE] {};

   // Thresholds indexed by code, valid for -1 .. count
   constexpr const float *ByteThreshold() const { return byteThresholdPad + 1; }
   constexpr const float *PercentThreshold() const { return percentThresholdPad + 1; }

   constexpr Tables()
   {
      byteThresholdPad[0] = percentThresholdPad[0] = -std::numeric_limits<float>::infinity(); // No value is below it, so the step down never fires
      for (int i = 0; i < 255; i++)
         byteThresholdPad[1 + i] = kByteThreshold[i];
      for (int i = 0; i < 100; i++)
         percentThresholdPad[1 + i] = kPercentThreshold[i];
      byteThresholdPad[1 + 255] = percentThresholdPad[1 + 100] = std::numeric_limits<float>::infinity(); // No value reaches it, so the step up never fires

      // The thresholds are monotonic, so the code for each bin comes from walking them forward once rather than searching per entry
      for (int i = 0, b = 0, p = 0; i < LUT_SIZE; i++)
      {
         const float v = static_cast<float>(i) * (float)(1. / (LUT_SIZE - 1));
         while (b < 255 && v >= kByteThreshold[b])
            b++;
         while (p < 100 && v >= kPercentThreshold[p])
            p++;
         linearToByte[i] = static_cast<uint8_t>(b);
         linearToPercent[i] = static_cast<uint8_t>(p);
      }
   }
};

inline constexpr Tables g_tables;

// One step towards the exact code. At most one of the two comparisons can hold, since that
// would need threshold[c] <= linear < threshold[c-1] on a monotonic table, so both are
// evaluated and summed rather than branched on. NaN fails both and stays put
inline int Correct(const int c, const float linear, const float *const threshold)
{
   return c + static_cast<int>(linear >= threshold[c]) - static_cast<int>(linear < threshold[c - 1]);
}

}

// Linear luminance of an 8 bit sRGB component. Exact
inline float SRGBToLinear(const uint8_t component) { return detail::kSRGBToLinear[component]; }

// Linear luminance of a gamma encoded 0..100 brightness percentage (above 100 the curve is extrapolated). Exact
inline float PercentToLinear(const uint8_t percent) { return detail::kPercentToLinear[percent]; }

// Linear luminance of an sRGB triplet
inline float SRGBToLuminance(const uint8_t r, const uint8_t g, const uint8_t b)
{
   return 0.2126f * SRGBToLinear(r) + 0.7152f * SRGBToLinear(g) + 0.0722f * SRGBToLinear(b);
}

// Nearest 8 bit sRGB code for a linear value, clamped to 0..1. Exact
inline uint8_t LinearToSRGB(const float linear)
{
   const detail::Tables &t = detail::g_tables;
   const float c = detail::Clamp01(linear); // Correct() must see the clamped value too, or +INF steps past the end sentinel
   return static_cast<uint8_t>(detail::Correct(t.linearToByte[detail::LutIndex(c)], c, t.ByteThreshold()));
}

// Nearest 0..100 brightness percentage for a linear value, clamped to 0..1. Exact.
// The percentage is gamma encoded, so this is the same curve as LinearToSRGB, only
// quantized to the 101 codes the controller interface carries
inline uint8_t LinearToPercent(const float linear)
{
   const detail::Tables &t = detail::g_tables;
   const float c = detail::Clamp01(linear); // See LinearToSRGB
   return static_cast<uint8_t>(detail::Correct(t.linearToPercent[detail::LutIndex(c)], c, t.PercentThreshold()));
}

}
