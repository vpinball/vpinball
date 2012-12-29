#include <emmintrin.h>

static const __m128i t0123 = _mm_set_epi32(3,2,1,0);
static const __m128i t4444 = _mm_set1_epi32(4);
static const __m128i FF128 = _mm_set1_epi32(0xFF);
static const __m128i FF00FF128 = _mm_set1_epi32(0xFF00FFu);
static const __m128i FF00128 = _mm_set1_epi32(0xFF00u);
static const __m128i FF00FF00128 = _mm_set1_epi32(0xFF00FF00);
static const __m128i FF0000128 = _mm_set1_epi32(0xFF0000u);
static const __m128i FEFEFE128 = _mm_set1_epi32(0xFEFEFEu);
static const __m128i FEFEFE128_16 = _mm_set1_epi32((0x1Eu | 0x3Eu<<5 | 0x1Eu<<11) | ((0x1Eu | 0x3Eu<<5 | 0x1Eu<<11)<<16));
static const __m128i mask_565_rb = _mm_set1_epi32(0x00F800F8);
static const __m128i mask_565_pack_multiplier = _mm_set1_epi32(0x20000004);
static const __m128i mask_red = _mm_set1_epi32(0x000000F8);
static const __m128i mask_green = _mm_set1_epi32(0x0000FC00);
static const __m128i mask_blue = _mm_set1_epi32(0x00F80000);

__forceinline __m128i pack_565(const __m128i &lo)
{
    __m128i t0 = _mm_or_si128(_mm_madd_epi16(_mm_and_si128(lo, mask_565_rb), mask_565_pack_multiplier), _mm_and_si128(lo, mask_green));

#if defined(__SSE4__) || defined(__SSE4_1__) || defined(__SSE4_2__)
#warning uses SSE4
    return _mm_packus_epi32(t0, t0);
#else
    t0 = _mm_srai_epi32(_mm_slli_epi32(t0, 16 - 5), 16);
    return _mm_packs_epi32(t0, t0);
#endif
}

__forceinline __m128i unpack_565(const __m128i& lo)
{
    return _mm_or_si128(_mm_or_si128(_mm_and_si128(_mm_slli_epi32(lo, 3), mask_red), _mm_and_si128(_mm_slli_epi32(lo, 5), mask_green)), _mm_and_si128(_mm_slli_epi32(lo, 8), mask_blue));
}

__forceinline __m128i _mm_mul_int(const __m128i& a, const __m128i& b)
{
#if defined(__SSE4_1__) || defined(__SSE4_2__)
#warning uses SSE4.1
	return _mm_mullo_epi32(a,b);
#else
#if defined(__SSE4__) || defined(__SSE3__)
#warning uses SSE3
	const __m128i tmpa = (__m128i&)_mm_movehdup_ps((__m128&)a);
	const __m128i tmpb = (__m128i&)_mm_movehdup_ps((__m128&)b);
#else
	const __m128i tmpa = _mm_shuffle_epi32(a,_MM_SHUFFLE(3,3,1,1));
	const __m128i tmpb = _mm_shuffle_epi32(b,_MM_SHUFFLE(3,3,1,1));
#endif
	return _mm_shuffle_epi32((__m128i&)_mm_shuffle_ps((__m128&)_mm_mul_epu32(a,b),(__m128&)_mm_mul_epu32(tmpa,tmpb),_MM_SHUFFLE(2,0,2,0)),_MM_SHUFFLE(3,1,2,0));
#endif
}

__forceinline __m128i _mm_mul_int_i(const __m128i& a, const __m128i& b) // b needs same values in all 4x32bits
{
#if defined(__SSE4_1__) || defined(__SSE4_2__)
#warning uses SSE4.1
	return _mm_mullo_epi32(a,b);
#else
#if defined(__SSE4__) || defined(__SSE3__)
#warning uses SSE3
	const __m128i tmp = (__m128i&)_mm_movehdup_ps((__m128&)a);
#else
	const __m128i tmp = _mm_shuffle_epi32(a,_MM_SHUFFLE(3,3,1,1));
#endif
	return _mm_shuffle_epi32((__m128i&)_mm_shuffle_ps((__m128&)_mm_mul_epu32(a,b),(__m128&)_mm_mul_epu32(tmp,b),_MM_SHUFFLE(2,0,2,0)),_MM_SHUFFLE(3,1,2,0));
#endif
}

inline void memcpy_sse2(void * const __restrict dst, const void * const __restrict src, const unsigned int nBytes) //!! seems to be even faster than intel c builtin version?!
{
    __asm
    {
        mov         ecx, nBytes
        mov         edi, dst
        mov         esi, src
        add         ecx, edi

        prefetchnta [esi]
        prefetchnta [esi+32]
        prefetchnta [esi+64]
        prefetchnta [esi+96]

        // handle nBytes lower than 128
        cmp         nBytes, 512
        jge         fast
slow:
        mov         bl, [esi]
        mov         [edi], bl
        inc         edi
        inc         esi
        cmp         ecx, edi
        jnz         slow
        jmp         endc
fast:
        // align dstEnd to 128 bytes
        and         ecx, 0xFFFFFF80

        // get srcEnd aligned to dstEnd aligned to 128 bytes
        mov         ebx, esi
        sub         ebx, edi
        add         ebx, ecx
                
        // skip unaligned copy if dst is aligned
        mov         eax, edi
        and         edi, 0xFFFFFF80
        cmp         eax, edi
        jne         first
        jmp         more
first:
        // copy the first 128 bytes unaligned
        movdqu      xmm0, [esi]
        movdqu      xmm1, [esi+16]
        movdqu      xmm2, [esi+32]
        movdqu      xmm3, [esi+48]
                
        movdqu      xmm4, [esi+64]
        movdqu      xmm5, [esi+80]
        movdqu      xmm6, [esi+96]
        movdqu      xmm7, [esi+112]
                
        movdqu      [eax], xmm0
        movdqu      [eax+16], xmm1
        movdqu      [eax+32], xmm2
        movdqu      [eax+48], xmm3
                
        movdqu      [eax+64], xmm4
        movdqu      [eax+80], xmm5
        movdqu      [eax+96], xmm6
        movdqu      [eax+112], xmm7
                
        // add 128 bytes to edi aligned earlier
        add         edi, 128
                
        // offset esi by the same value
        sub         eax, edi
        sub         esi, eax
                
        // last bytes if dst at dstEnd
        cmp         ecx, edi
        jnz         more
        jmp         last
more:
        // handle equally aligned arrays
        mov         eax, esi
        and         eax, 0xFFFFFF80
        cmp         eax, esi
        jne         unaligned4k
aligned4k:
        mov         eax, esi
        add         eax, 4096
        cmp         eax, ebx
        jle         aligned4kin
        cmp         ecx, edi
        jne         alignedlast
        jmp         last
aligned4kin:
        prefetchnta [esi]
        prefetchnta [esi+32]
        prefetchnta [esi+64]
        prefetchnta [esi+96]
                
        add         esi, 128
                
        cmp         eax, esi
        jne         aligned4kin

        sub         esi, 4096
alinged4kout:
        movdqa      xmm0, [esi]
        movdqa      xmm1, [esi+16]
        movdqa      xmm2, [esi+32]
        movdqa      xmm3, [esi+48]
                
        movdqa      xmm4, [esi+64]
        movdqa      xmm5, [esi+80]
        movdqa      xmm6, [esi+96]
        movdqa      xmm7, [esi+112]
                
        movntdq     [edi], xmm0
        movntdq     [edi+16], xmm1
        movntdq     [edi+32], xmm2
        movntdq     [edi+48], xmm3
                
        movntdq     [edi+64], xmm4
        movntdq     [edi+80], xmm5
        movntdq     [edi+96], xmm6
        movntdq     [edi+112], xmm7
                
        add         esi, 128
        add         edi, 128
                
        cmp         eax, esi
        jne         alinged4kout
        jmp         aligned4k
alignedlast:
        mov         eax, esi
alignedlastin:
        prefetchnta [esi]
        prefetchnta [esi+32]
        prefetchnta [esi+64]
        prefetchnta [esi+96]
                
        add         esi, 128
                
        cmp         ebx, esi
        jne         alignedlastin
                
        mov         esi, eax
alignedlastout:
        movdqa      xmm0, [esi]
        movdqa      xmm1, [esi+16]
        movdqa      xmm2, [esi+32]
        movdqa      xmm3, [esi+48]
                
        movdqa      xmm4, [esi+64]
        movdqa      xmm5, [esi+80]
        movdqa      xmm6, [esi+96]
        movdqa      xmm7, [esi+112]
                
        movntdq     [edi], xmm0
        movntdq     [edi+16], xmm1
        movntdq     [edi+32], xmm2
        movntdq     [edi+48], xmm3
                
        movntdq     [edi+64], xmm4
        movntdq     [edi+80], xmm5
        movntdq     [edi+96], xmm6
        movntdq     [edi+112], xmm7
                
        add         esi, 128
        add         edi, 128
                
        cmp         ecx, edi
        jne         alignedlastout
        jmp         last
unaligned4k:
        mov         eax, esi
        add         eax, 4096
        cmp         eax, ebx
        jle         unaligned4kin
        cmp         ecx, edi
        jne         unalignedlast
        jmp         last
unaligned4kin:
        prefetchnta [esi]
        prefetchnta [esi+32]
        prefetchnta [esi+64]
        prefetchnta [esi+96]
                
        add         esi, 128
                
        cmp         eax, esi
        jne         unaligned4kin

        sub         esi, 4096
unalinged4kout:
        movdqu      xmm0, [esi]
        movdqu      xmm1, [esi+16]
        movdqu      xmm2, [esi+32]
        movdqu      xmm3, [esi+48]
                
        movdqu      xmm4, [esi+64]
        movdqu      xmm5, [esi+80]
        movdqu      xmm6, [esi+96]
        movdqu      xmm7, [esi+112]
                
        movntdq     [edi], xmm0
        movntdq     [edi+16], xmm1
        movntdq     [edi+32], xmm2
        movntdq     [edi+48], xmm3
                
        movntdq     [edi+64], xmm4
        movntdq     [edi+80], xmm5
        movntdq     [edi+96], xmm6
        movntdq     [edi+112], xmm7
                
        add         esi, 128
        add         edi, 128
                
        cmp         eax, esi
        jne         unalinged4kout
        jmp         unaligned4k
unalignedlast:
        mov         eax, esi
unalignedlastin:
        prefetchnta [esi]
        prefetchnta [esi+32]
        prefetchnta [esi+64]
        prefetchnta [esi+96]
                
        add         esi, 128
                
        cmp         ebx, esi
        jne         unalignedlastin
                
        mov         esi, eax
unalignedlastout:
        movdqu      xmm0, [esi]
        movdqu      xmm1, [esi+16]
        movdqu      xmm2, [esi+32]
        movdqu      xmm3, [esi+48]
                
        movdqu      xmm4, [esi+64]
        movdqu      xmm5, [esi+80]
        movdqu      xmm6, [esi+96]
        movdqu      xmm7, [esi+112]
                
        movntdq     [edi], xmm0
        movntdq     [edi+16], xmm1
        movntdq     [edi+32], xmm2
        movntdq     [edi+48], xmm3
                
        movntdq     [edi+64], xmm4
        movntdq     [edi+80], xmm5
        movntdq     [edi+96], xmm6
        movntdq     [edi+112], xmm7
                
        add         esi, 128
        add         edi, 128
                
        cmp         ecx, edi
        jne         unalignedlastout
        jmp         last
last:
        // get the last 128 bytes
        mov         ecx, nBytes
        mov         edi, dst
        mov         esi, src
        add         edi, ecx
        add         esi, ecx
        sub         edi, 128
        sub         esi, 128

        // copy the last 128 bytes unaligned
        movdqu      xmm0, [esi]
        movdqu      xmm1, [esi+16]
        movdqu      xmm2, [esi+32]
        movdqu      xmm3, [esi+48]
                
        movdqu      xmm4, [esi+64]
        movdqu      xmm5, [esi+80]
        movdqu      xmm6, [esi+96]
        movdqu      xmm7, [esi+112]
                
        movdqu      [edi], xmm0
        movdqu      [edi+16], xmm1
        movdqu      [edi+32], xmm2
        movdqu      [edi+48], xmm3
                
        movdqu      [edi+64], xmm4
        movdqu      [edi+80], xmm5
        movdqu      [edi+96], xmm6
        movdqu      [edi+112], xmm7
endc:
    }
}

// initial non-intrinsic implementation of stereo3D
#if 0
for(; x < xend; x++,offsz++)
{
//const UINT z = (float)(bufferzcopy[offsz]&zmask)*(float)(255.0/zmask);
//bufferfinal[x + y*nPitch] = z|(z<<8)|(z<<16);

const unsigned int minDepthR = min( min( bufferzcopy[offsz - samples[0]]&zmask, bufferzcopy[offsz - samples[1]]&zmask ), bufferzcopy[offsz - samples[2]]&zmask );
const unsigned int minDepthL = min( min( bufferzcopy[offsz + samples[0]]&zmask, bufferzcopy[offsz + samples[1]]&zmask ), bufferzcopy[offsz + samples[2]]&zmask );

const unsigned int parallaxR = min(ZPDU / minDepthR, maxSeparationU<<4); //!! <<5? to allow for pop out, but then modify bilerp filter/pixel offset below, depending on sign of -overall- parallax shift?!
const unsigned int parallaxL = min(ZPDU / minDepthL, maxSeparationU<<4); //!! <<5? to allow for pop out, but then modify bilerp filter/pixel offset below, depending on sign of -overall- parallax shift?!
const unsigned int pR = parallaxR>>4; // /16 = fixed point math, also see above
const unsigned int pL = parallaxL>>4;

const unsigned int separationR = x + (y - maxSeparationU + pR)*nPitch; // + y*nPitch - maxSeparationU + pR  for x
const unsigned int right0 = buffercopy[separationR];
const unsigned int right1 = buffercopy[separationR - nPitch /*+ (((maxSeparationU - pR)>>30)&2)*nPitch*/]; // -1 instead of nPitch for x

const unsigned int r13   = (parallaxR*16) & 0xFFu; // *16 = scale from fixed point math to 256 for linear filtering below
const unsigned int r23   = 0xFFu - r13;
const unsigned int right = ((((right0&0xFF00FFu)*r13+(right1&0xFF00FFu)*r23)&0xFF00FF00)|(((right0&0x00FF00u)*r13+(right1&0x00FF00u)*r23)&0x00FF0000u))>>8; // linear filtering

const unsigned int separationL = x + (y + maxSeparationU - pL)*nPitch; // + y*nPitch + maxSeparationU - pL  for x
const unsigned int left0 = buffercopy[separationL];
const unsigned int left1 = buffercopy[separationL + nPitch /*- (((maxSeparationU - pL)>>30)&2)*nPitch*/]; // +1 instead of nPitch for x

const unsigned int l13  = (parallaxL*16) & 0xFFu; // *16 = scale from fixed point math to 256 for linear filtering below
const unsigned int l23  = 0xFFu - l13;
const unsigned int left = ((((left0&0xFF00FFu)*l13+(left1&0xFF00FFu)*l23)&0xFF00FF00u)|(((left0&0x00FF00u)*l13+(left1&0x00FF00u)*l23)&0x00FF0000u))>>8; // linear filtering

//!! bufferfinal[offs] = (right&0xFF0000u) | (left&0xFFFFFFu);

if(AA & y) // so always triggered if AA and y&1
{
	bufferfinal[offshalf0 + x] = ((bufferfinal[offshalf0 + x]&0xFEFEFEu) + (right&0xFEFEFEu))>>1; // average with previously computed line
	bufferfinal[offshalf1 + x] = ((bufferfinal[offshalf1 + x]&0xFEFEFEu) + (left &0xFEFEFEu))>>1;
} else {
	bufferfinal[offshalf0 + x] = right;
	bufferfinal[offshalf1 + x] = left;
}
#endif

//
// the following/same code is basically copied 4 times to optimize for 16bit yaxis, 32bit yaxis, 16bit xaxis and 32bit xaxis (as the compiler doesn't figure that out automatically and i hate templates for such low-level code ;))
//

__forceinline void stereo_repro_16bit_y(const int ystart, const int yend, const int xstart, const int xend, const unsigned int width, const unsigned int owidth, const unsigned int nwidth, const unsigned int height, const unsigned int maxSeparationU, const unsigned short * const __restrict buffercopy, const unsigned short * const __restrict bufferzcopy, unsigned short * const __restrict bufferfinal, const unsigned int samples[3], const __m128& ZPDU128, const __m128& maxSepShl4128, const bool handle_borders, const bool topdown, const unsigned int AA, unsigned char* const __restrict mask)
{
if(handle_borders) {
	if(topdown) {
		ZeroMemory(bufferfinal,                                (width*(ystart>>1))<<1);                        //!! black out border pixels, replicate borders for one half instead?? //!! opt. with SSE2?
		ZeroMemory(bufferfinal+             (yend>>1) *nwidth, (width*(((height-yend)>>1) + (ystart>>1)))<<1); //!! black out border pixels, replicate borders for one half instead??
		ZeroMemory(bufferfinal+((height>>1)+(yend>>1))*nwidth, (width* ((height-yend)>>1))<<1);                //!! black out border pixels, replicate borders for one half instead??
	} else {
		ZeroMemory(bufferfinal,                                (width*ystart)<<1);							   //!! black out border pixels, replicate borders for one half instead?? //!! opt. with SSE2?
		ZeroMemory(bufferfinal+                   yend*nwidth, (width*(height-yend))<<1);					   //!! black out border pixels, replicate borders for one half instead??
	}
}

const __m128i width128 = _mm_set1_epi32(owidth);

const int incr = (AA^1)+1;

#pragma omp parallel for schedule(dynamic) //!! make configurable for update/non-update version
for(int yi = ystart; yi < yend; yi+=incr)  //!! interleave left/right and calcs instead? (might be faster, too, due to smaller register usage?)
{
const unsigned int y = yi;
const unsigned int offshalf0 = (topdown ? (y>>1) : (y&0xFFFFFFFE)) * nwidth;
const unsigned int offshalf1 = (topdown ? ((height>>1)*nwidth) : nwidth) + offshalf0;
const unsigned int ymms = y - maxSeparationU;
const unsigned int ypms = y + maxSeparationU;

const __m128i ymms128 = _mm_set1_epi32(ymms);
const __m128i ypms128 = _mm_set1_epi32(ypms);

const unsigned short* __restrict z = bufferzcopy + (y*owidth + xstart);
__m128i x128 = _mm_add_epi32(t0123,_mm_set1_epi32(xstart));

unsigned int xm = y*(owidth>>2) + (xstart>>2);
int x = xstart;
for(; x < xend; x+=4,z+=4,++xm,x128=_mm_add_epi32(x128,t4444)) if(mask[xm] == 0)
{
mask[xm] = 1;

const __m128 minDepthR = _mm_min_ps(_mm_min_ps(_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(z-samples[0])),_mm_setzero_si128())),_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(z-samples[1])),_mm_setzero_si128()))),_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(z-samples[2])),_mm_setzero_si128())));
const __m128 minDepthL = _mm_min_ps(_mm_min_ps(_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(z+samples[0])),_mm_setzero_si128())),_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(z+samples[1])),_mm_setzero_si128()))),_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(z+samples[2])),_mm_setzero_si128())));

const __m128i parallaxR = _mm_cvtps_epi32(_mm_min_ps(_mm_mul_ps(ZPDU128,_mm_rcp_ps(minDepthR)),maxSepShl4128)); // doesn't seem to be needing full precision div
const __m128i parallaxL = _mm_cvtps_epi32(_mm_min_ps(_mm_mul_ps(ZPDU128,_mm_rcp_ps(minDepthL)),maxSepShl4128)); // dto.

const __m128i pR = _mm_add_epi32(_mm_mul_int_i(_mm_add_epi32(ymms128,_mm_srli_epi32(parallaxR,4)),width128),x128);
const __m128i pL = _mm_add_epi32(_mm_mul_int_i(_mm_sub_epi32(ypms128,_mm_srli_epi32(parallaxL,4)),width128),x128);

const __m128i pRs4_1 = _mm_and_si128(_mm_slli_epi32(parallaxR,4),FF128);
const __m128i pLs4_1 = _mm_and_si128(_mm_slli_epi32(parallaxL,4),FF128);
const __m128i pRs4_2 = _mm_sub_epi32(FF128,pRs4_1);
const __m128i pLs4_2 = _mm_sub_epi32(FF128,pLs4_1);

const unsigned int separationR0 = ((unsigned int*)&pR)[0];
const unsigned int separationR1 = ((unsigned int*)&pR)[1];
const unsigned int separationR2 = ((unsigned int*)&pR)[2];
const unsigned int separationR3 = ((unsigned int*)&pR)[3];

//!! opt. the set_epi32's here?!
const __m128i right0 = unpack_565(_mm_set_epi32(buffercopy[separationR3]         ,buffercopy[separationR2]         ,buffercopy[separationR1]         ,buffercopy[separationR0]));
const __m128i right1 = unpack_565(_mm_set_epi32(buffercopy[separationR3 - owidth],buffercopy[separationR2 - owidth],buffercopy[separationR1 - owidth],buffercopy[separationR0 - owidth]));

const unsigned int separationL0 = ((unsigned int*)&pL)[0];
const unsigned int separationL1 = ((unsigned int*)&pL)[1];
const unsigned int separationL2 = ((unsigned int*)&pL)[2];
const unsigned int separationL3 = ((unsigned int*)&pL)[3];

const __m128i left0 = unpack_565(_mm_set_epi32(buffercopy[separationL3]         ,buffercopy[separationL2]         ,buffercopy[separationL1]         ,buffercopy[separationL0]));
const __m128i left1 = unpack_565(_mm_set_epi32(buffercopy[separationL3 + owidth],buffercopy[separationL2 + owidth],buffercopy[separationL1 + owidth],buffercopy[separationL0 + owidth]));

const __m128i right00 = _mm_mul_int(_mm_and_si128(right0,FF00FF128),pRs4_1);
const __m128i right01 = _mm_mul_int(_mm_and_si128(right0,FF00128),  pRs4_1);
const __m128i right10 = _mm_mul_int(_mm_and_si128(right1,FF00FF128),pRs4_2);
const __m128i right11 = _mm_mul_int(_mm_and_si128(right1,FF00128),  pRs4_2);

__m128i right = pack_565(_mm_srli_epi32(_mm_or_si128(_mm_and_si128(_mm_add_epi32(right00,right10),FF00FF00128),_mm_and_si128(_mm_add_epi32(right01,right11),FF0000128)),8));

if(AA & y) // so always triggered if AA and y&1
	right = _mm_add_epi32(_mm_srli_epi32(_mm_and_si128((__m128i&)_mm_loadl_pi(_mm_setzero_ps(),(__m64*)(bufferfinal+offshalf0+x)),FEFEFE128_16),1), _mm_srli_epi32(_mm_and_si128(right,FEFEFE128_16),1));

_mm_storel_pi((__m64*)(bufferfinal+offshalf0+x), (__m128&)right);

const __m128i left00 = _mm_mul_int(_mm_and_si128(left0,FF00FF128),pLs4_1);
const __m128i left01 = _mm_mul_int(_mm_and_si128(left0,FF00128),  pLs4_1);
const __m128i left10 = _mm_mul_int(_mm_and_si128(left1,FF00FF128),pLs4_2);
const __m128i left11 = _mm_mul_int(_mm_and_si128(left1,FF00128),  pLs4_2);

__m128i left = pack_565(_mm_srli_epi32(_mm_or_si128(_mm_and_si128(_mm_add_epi32(left00,left10),FF00FF00128),_mm_and_si128(_mm_add_epi32(left01,left11),FF0000128)),8));

if(AA & y) // so always triggered if AA and y&1
	left = _mm_add_epi32(_mm_srli_epi32(_mm_and_si128((__m128i&)_mm_loadl_pi(_mm_setzero_ps(),(__m64*)(bufferfinal+offshalf1+x)),FEFEFE128_16),1), _mm_srli_epi32(_mm_and_si128(left,FEFEFE128_16),1));

_mm_storel_pi((__m64*)(bufferfinal+offshalf1+x), (__m128&)left);
}
}
}

__forceinline void stereo_repro_32bit_y(const int ystart, const int yend, const int xstart, const int xend, const unsigned int width, const unsigned int owidth, const unsigned int nwidth, const unsigned int height, const unsigned int maxSeparationU, const unsigned int* const __restrict buffercopy, const unsigned int* const __restrict bufferzcopy, unsigned int* const __restrict bufferfinal, const unsigned int samples[3], const __m128i& zmask128, const __m128& ZPDU128, const __m128& maxSepShl4128, const bool handle_borders, const bool topdown, const unsigned int AA, unsigned char* const __restrict mask)
{
if(handle_borders) {
	if(topdown) {
		ZeroMemory(bufferfinal,                                (width*(ystart>>1))<<2);                        //!! black out border pixels, replicate borders for one half instead?? //!! opt. with SSE2?
		ZeroMemory(bufferfinal+             (yend>>1) *nwidth, (width*(((height-yend)>>1) + (ystart>>1)))<<2); //!! black out border pixels, replicate borders for one half instead??
		ZeroMemory(bufferfinal+((height>>1)+(yend>>1))*nwidth, (width* ((height-yend)>>1))<<2);                //!! black out border pixels, replicate borders for one half instead??
	} else {
		ZeroMemory(bufferfinal,                                (width*ystart)<<2);							   //!! black out border pixels, replicate borders for one half instead?? //!! opt. with SSE2?
		ZeroMemory(bufferfinal+                   yend*nwidth, (width*(height-yend))<<2);					   //!! black out border pixels, replicate borders for one half instead??
	}
}

const __m128i width128 = _mm_set1_epi32(owidth);

const int incr = (AA^1)+1;

#pragma omp parallel for schedule(dynamic) //!! make configurable for update/non-update version
for(int yi = ystart; yi < yend; yi+=incr)  //!! interleave left/right and calcs instead? (might be faster, too, due to smaller register usage?)
{
const unsigned int y = yi;
const unsigned int offshalf0 = (topdown ? (y>>1) : (y&0xFFFFFFFE)) * nwidth;
const unsigned int offshalf1 = (topdown ? ((height>>1)*nwidth) : nwidth) + offshalf0;
const unsigned int ymms = y - maxSeparationU;
const unsigned int ypms = y + maxSeparationU;

const __m128i ymms128 = _mm_set1_epi32(ymms);
const __m128i ypms128 = _mm_set1_epi32(ypms);

const unsigned int * __restrict z = bufferzcopy + (y*owidth + xstart);
__m128i x128 = _mm_add_epi32(t0123,_mm_set1_epi32(xstart));

unsigned int xm = y*(owidth>>2) + (xstart>>2);
int x = xstart;
for(; x < xend; x+=4,z+=4,++xm,x128=_mm_add_epi32(x128,t4444)) if(mask[xm] == 0)
{
mask[xm] = 1;

const __m128 minDepthR = _mm_min_ps(_mm_min_ps(_mm_cvtepi32_ps(_mm_and_si128(_mm_load_si128((__m128i*)(z-samples[0])),zmask128)),_mm_cvtepi32_ps(_mm_and_si128(_mm_load_si128((__m128i*)(z-samples[1])),zmask128))),_mm_cvtepi32_ps(_mm_and_si128(_mm_load_si128((__m128i*)(z-samples[2])),zmask128)));
const __m128 minDepthL = _mm_min_ps(_mm_min_ps(_mm_cvtepi32_ps(_mm_and_si128(_mm_load_si128((__m128i*)(z+samples[0])),zmask128)),_mm_cvtepi32_ps(_mm_and_si128(_mm_load_si128((__m128i*)(z+samples[1])),zmask128))),_mm_cvtepi32_ps(_mm_and_si128(_mm_load_si128((__m128i*)(z+samples[2])),zmask128)));

const __m128i parallaxR = _mm_cvtps_epi32(_mm_min_ps(_mm_mul_ps(ZPDU128,_mm_rcp_ps(minDepthR)),maxSepShl4128)); // doesn't seem to be needing full prec. div
const __m128i parallaxL = _mm_cvtps_epi32(_mm_min_ps(_mm_mul_ps(ZPDU128,_mm_rcp_ps(minDepthL)),maxSepShl4128)); // dto.

const __m128i pR = _mm_add_epi32(_mm_mul_int_i(_mm_add_epi32(ymms128,_mm_srli_epi32(parallaxR,4)),width128),x128);
const __m128i pL = _mm_add_epi32(_mm_mul_int_i(_mm_sub_epi32(ypms128,_mm_srli_epi32(parallaxL,4)),width128),x128);

const __m128i pRs4_1 = _mm_and_si128(_mm_slli_epi32(parallaxR,4),FF128);
const __m128i pLs4_1 = _mm_and_si128(_mm_slli_epi32(parallaxL,4),FF128);
const __m128i pRs4_2 = _mm_sub_epi32(FF128,pRs4_1);
const __m128i pLs4_2 = _mm_sub_epi32(FF128,pLs4_1);

const unsigned int separationR0 = ((unsigned int*)&pR)[0];
const unsigned int separationR1 = ((unsigned int*)&pR)[1];
const unsigned int separationR2 = ((unsigned int*)&pR)[2];
const unsigned int separationR3 = ((unsigned int*)&pR)[3];

//!! opt. the set_epi32's here?!
const __m128i right0 =   _mm_set_epi32(buffercopy[separationR3]         ,buffercopy[separationR2]         ,buffercopy[separationR1]         ,buffercopy[separationR0]);
const __m128i right1 =   _mm_set_epi32(buffercopy[separationR3 - owidth],buffercopy[separationR2 - owidth],buffercopy[separationR1 - owidth],buffercopy[separationR0 - owidth]);

const unsigned int separationL0 = ((unsigned int*)&pL)[0];
const unsigned int separationL1 = ((unsigned int*)&pL)[1];
const unsigned int separationL2 = ((unsigned int*)&pL)[2];
const unsigned int separationL3 = ((unsigned int*)&pL)[3];

const __m128i left0 =   _mm_set_epi32(buffercopy[separationL3]         ,buffercopy[separationL2]         ,buffercopy[separationL1]         ,buffercopy[separationL0]);
const __m128i left1 =   _mm_set_epi32(buffercopy[separationL3 + owidth],buffercopy[separationL2 + owidth],buffercopy[separationL1 + owidth],buffercopy[separationL0 + owidth]);

const __m128i right00 = _mm_mul_int(_mm_and_si128(right0,FF00FF128),pRs4_1);
const __m128i right01 = _mm_mul_int(_mm_and_si128(right0,FF00128),  pRs4_1);
const __m128i right10 = _mm_mul_int(_mm_and_si128(right1,FF00FF128),pRs4_2);
const __m128i right11 = _mm_mul_int(_mm_and_si128(right1,FF00128),  pRs4_2);

__m128i right = _mm_srli_epi32(_mm_or_si128(_mm_and_si128(_mm_add_epi32(right00,right10),FF00FF00128),_mm_and_si128(_mm_add_epi32(right01,right11),FF0000128)),8);

if(AA & y) // so always triggered if AA and y&1
	right = _mm_srli_epi32(_mm_add_epi32(_mm_and_si128(_mm_load_si128((__m128i*)(bufferfinal+offshalf0+x)),FEFEFE128), _mm_and_si128(right,FEFEFE128)),1);

_mm_stream_si128((__m128i*)(bufferfinal+offshalf0+x), right);

const __m128i left00 = _mm_mul_int(_mm_and_si128(left0,FF00FF128),pLs4_1);
const __m128i left01 = _mm_mul_int(_mm_and_si128(left0,FF00128),  pLs4_1);
const __m128i left10 = _mm_mul_int(_mm_and_si128(left1,FF00FF128),pLs4_2);
const __m128i left11 = _mm_mul_int(_mm_and_si128(left1,FF00128),  pLs4_2);

__m128i left = _mm_srli_epi32(_mm_or_si128(_mm_and_si128(_mm_add_epi32(left00,left10),FF00FF00128),_mm_and_si128(_mm_add_epi32(left01,left11),FF0000128)),8);

if(AA & y) // so always triggered if AA and y&1
	left = _mm_srli_epi32(_mm_add_epi32(_mm_and_si128(_mm_load_si128((__m128i*)(bufferfinal+offshalf1+x)),FEFEFE128), _mm_and_si128(left,FEFEFE128)),1);

_mm_stream_si128((__m128i*)(bufferfinal+offshalf1+x), left);
}
}
}

__forceinline void stereo_repro_16bit_x(const int ystart, const int yend, const int xstart, const int xend, const unsigned int width, const unsigned int owidth, const unsigned int nwidth, const unsigned int height, const unsigned int maxSeparationU, const unsigned short * const __restrict buffercopy, const unsigned short * const __restrict bufferzcopy, unsigned short * const __restrict bufferfinal, const unsigned int samples[3], const __m128& ZPDU128, const __m128& maxSepShl4128, const bool handle_borders, const bool topdown, const unsigned int AA, unsigned char* const __restrict mask)
{
const int incr = (AA^1)+1;

#pragma omp parallel for schedule(dynamic) //!! make configurable for update/non-update version
for(int yi = ystart; yi < yend; yi+=incr)  //!! or interleave left/right and calcs instead? (might be faster, too, due to smaller register usage?)
{
const unsigned int y = yi;
const unsigned int offshalf1 = (topdown ? (y>>1) : (y&0xFFFFFFFE)) * nwidth;
const unsigned int offshalf0 = (topdown ? ((height>>1)*nwidth) : nwidth) + offshalf1;

const __m128i ymms128 = _mm_set1_epi32(y*owidth - maxSeparationU);
const __m128i ypms128 = _mm_set1_epi32(y*owidth + maxSeparationU);

const unsigned short* __restrict z = bufferzcopy + (y*owidth + xstart);
__m128i x128 = _mm_add_epi32(t0123,_mm_set1_epi32(xstart));

int x;
if(handle_borders) {
    for(x = 0; x < xstart; ++x) //!! black out border pixels, replicate borders for one half instead??
	    bufferfinal[offshalf0 + x] = bufferfinal[offshalf1 + x] = 0;
} else
    x = xstart;

unsigned int xm = y*(owidth>>2) + (xstart>>2);
for(; x < xend; x+=4,z+=4,++xm,x128=_mm_add_epi32(x128,t4444)) if(mask[xm] == 0)
{
mask[xm] = 1;

const __m128 minDepthR = _mm_min_ps(_mm_min_ps(_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(z-samples[0])),_mm_setzero_si128())),_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(z-samples[1])),_mm_setzero_si128()))),_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(z-samples[2])),_mm_setzero_si128())));
const __m128 minDepthL = _mm_min_ps(_mm_min_ps(_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(z+samples[0])),_mm_setzero_si128())),_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(z+samples[1])),_mm_setzero_si128()))),_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(z+samples[2])),_mm_setzero_si128())));

const __m128i parallaxR = _mm_cvtps_epi32(_mm_min_ps(_mm_mul_ps(ZPDU128,_mm_rcp_ps(minDepthR)),maxSepShl4128)); // doesn't seem to be needing full prec. div
const __m128i parallaxL = _mm_cvtps_epi32(_mm_min_ps(_mm_mul_ps(ZPDU128,_mm_rcp_ps(minDepthL)),maxSepShl4128)); // dto.

const __m128i pR = _mm_add_epi32(_mm_add_epi32(ymms128,_mm_srli_epi32(parallaxR,4)),x128);
const __m128i pL = _mm_add_epi32(_mm_sub_epi32(ypms128,_mm_srli_epi32(parallaxL,4)),x128);

const __m128i pRs4_1 = _mm_and_si128(_mm_slli_epi32(parallaxR,4),FF128);
const __m128i pLs4_1 = _mm_and_si128(_mm_slli_epi32(parallaxL,4),FF128);
const __m128i pRs4_2 = _mm_sub_epi32(FF128,pRs4_1);
const __m128i pLs4_2 = _mm_sub_epi32(FF128,pLs4_1);

const unsigned int separationR0 = ((unsigned int*)&pR)[0];
const unsigned int separationR1 = ((unsigned int*)&pR)[1];
const unsigned int separationR2 = ((unsigned int*)&pR)[2];
const unsigned int separationR3 = ((unsigned int*)&pR)[3];

//!! opt. the set_epi32's here?!
const __m128i right0 = unpack_565(_mm_set_epi32(buffercopy[separationR3]        ,buffercopy[separationR2]        ,buffercopy[separationR1]        ,buffercopy[separationR0]));
const __m128i right1 = unpack_565(_mm_set_epi32(buffercopy[separationR3 - 1]    ,buffercopy[separationR2 - 1]    ,buffercopy[separationR1 - 1]    ,buffercopy[separationR0 - 1]));

const unsigned int separationL0 = ((unsigned int*)&pL)[0];
const unsigned int separationL1 = ((unsigned int*)&pL)[1];
const unsigned int separationL2 = ((unsigned int*)&pL)[2];
const unsigned int separationL3 = ((unsigned int*)&pL)[3];

const __m128i left0 = unpack_565(_mm_set_epi32(buffercopy[separationL3]        ,buffercopy[separationL2]        ,buffercopy[separationL1]        ,buffercopy[separationL0]));
const __m128i left1 = unpack_565(_mm_set_epi32(buffercopy[separationL3 + 1]    ,buffercopy[separationL2 + 1]    ,buffercopy[separationL1 + 1]    ,buffercopy[separationL0 + 1]));

const __m128i right00 = _mm_mul_int(_mm_and_si128(right0,FF00FF128),pRs4_1);
const __m128i right01 = _mm_mul_int(_mm_and_si128(right0,FF00128),  pRs4_1);
const __m128i right10 = _mm_mul_int(_mm_and_si128(right1,FF00FF128),pRs4_2);
const __m128i right11 = _mm_mul_int(_mm_and_si128(right1,FF00128),  pRs4_2);

__m128i right = pack_565(_mm_srli_epi32(_mm_or_si128(_mm_and_si128(_mm_add_epi32(right00,right10),FF00FF00128),_mm_and_si128(_mm_add_epi32(right01,right11),FF0000128)),8));

if(AA & y) // so always triggered if AA and y&1
	right = _mm_add_epi32(_mm_srli_epi32(_mm_and_si128((__m128i&)_mm_loadl_pi(_mm_setzero_ps(),(__m64*)(bufferfinal+offshalf0+x)),FEFEFE128_16),1), _mm_srli_epi32(_mm_and_si128(right,FEFEFE128_16),1));

_mm_storel_pi((__m64*)(bufferfinal+offshalf0+x), (__m128&)right);

const __m128i left00 = _mm_mul_int(_mm_and_si128(left0,FF00FF128),pLs4_1);
const __m128i left01 = _mm_mul_int(_mm_and_si128(left0,FF00128),  pLs4_1);
const __m128i left10 = _mm_mul_int(_mm_and_si128(left1,FF00FF128),pLs4_2);
const __m128i left11 = _mm_mul_int(_mm_and_si128(left1,FF00128),  pLs4_2);

__m128i left = pack_565(_mm_srli_epi32(_mm_or_si128(_mm_and_si128(_mm_add_epi32(left00,left10),FF00FF00128),_mm_and_si128(_mm_add_epi32(left01,left11),FF0000128)),8));

if(AA & y) // so always triggered if AA and y&1
	left = _mm_add_epi32(_mm_srli_epi32(_mm_and_si128((__m128i&)_mm_loadl_pi(_mm_setzero_ps(),(__m64*)(bufferfinal+offshalf1+x)),FEFEFE128_16),1), _mm_srli_epi32(_mm_and_si128(left,FEFEFE128_16),1));

_mm_storel_pi((__m64*)(bufferfinal+offshalf1+x), (__m128&)left);
}

if(handle_borders)
    for(; x < (int)width; ++x) //!! black out border pixels, replicate borders for one half instead??
	    bufferfinal[offshalf0 + x] = bufferfinal[offshalf1 + x] = 0;
}
}

__forceinline void stereo_repro_32bit_x(const int ystart, const int yend, const int xstart, const int xend, const unsigned int width, const unsigned int owidth, const unsigned int nwidth, const unsigned int height, const unsigned int maxSeparationU, const unsigned int* const __restrict buffercopy, const unsigned int* const __restrict bufferzcopy, unsigned int* const __restrict bufferfinal, const unsigned int samples[3], const __m128i& zmask128, const __m128& ZPDU128, const __m128& maxSepShl4128, const bool handle_borders, const bool topdown, const unsigned int AA, unsigned char* const __restrict mask)
{
const int incr = (AA^1)+1;

#pragma omp parallel for schedule(dynamic) //!! make configurable for update/non-update version
for(int yi = ystart; yi < yend; yi+=incr)  //!! interleave left/right and calcs instead? (might be faster, too, due to smaller register usage?)
{
const unsigned int y = yi;
const unsigned int offshalf1 = (topdown ? (y>>1) : (y&0xFFFFFFFE)) * nwidth;
const unsigned int offshalf0 = (topdown ? ((height>>1)*nwidth) : nwidth) + offshalf1;

const __m128i ymms128 = _mm_set1_epi32(y*owidth - maxSeparationU);
const __m128i ypms128 = _mm_set1_epi32(y*owidth + maxSeparationU);

const unsigned int * __restrict z = bufferzcopy + (y*owidth + xstart);
__m128i x128 = _mm_add_epi32(t0123,_mm_set1_epi32(xstart));

unsigned int xm = y*(owidth>>2) + (xstart>>2);
int x;
if(handle_borders) {
    for(x = 0; x < xstart; ++x) //!! black out border pixels, replicate borders for one half instead??
	    bufferfinal[offshalf0 + x] = bufferfinal[offshalf1 + x] = 0;
} else
    x = xstart;

for(; x < xend; x+=4,z+=4,++xm,x128=_mm_add_epi32(x128,t4444)) if(mask[xm] == 0)
{
mask[xm] = 1;

const __m128 minDepthR = _mm_min_ps(_mm_min_ps(_mm_cvtepi32_ps(_mm_and_si128(_mm_loadu_si128((__m128i*)(z-samples[0])),zmask128)),_mm_cvtepi32_ps(_mm_and_si128(_mm_loadu_si128((__m128i*)(z-samples[1])),zmask128))),_mm_cvtepi32_ps(_mm_and_si128(_mm_loadu_si128((__m128i*)(z-samples[2])),zmask128)));
const __m128 minDepthL = _mm_min_ps(_mm_min_ps(_mm_cvtepi32_ps(_mm_and_si128(_mm_loadu_si128((__m128i*)(z+samples[0])),zmask128)),_mm_cvtepi32_ps(_mm_and_si128(_mm_loadu_si128((__m128i*)(z+samples[1])),zmask128))),_mm_cvtepi32_ps(_mm_and_si128(_mm_loadu_si128((__m128i*)(z+samples[2])),zmask128)));

const __m128i parallaxR = _mm_cvtps_epi32(_mm_min_ps(_mm_mul_ps(ZPDU128,_mm_rcp_ps(minDepthR)),maxSepShl4128)); // doesn't seem to be needing full prec. div
const __m128i parallaxL = _mm_cvtps_epi32(_mm_min_ps(_mm_mul_ps(ZPDU128,_mm_rcp_ps(minDepthL)),maxSepShl4128)); // dto.

const __m128i pR = _mm_add_epi32(_mm_add_epi32(ymms128,_mm_srli_epi32(parallaxR,4)),x128);
const __m128i pL = _mm_add_epi32(_mm_sub_epi32(ypms128,_mm_srli_epi32(parallaxL,4)),x128);

const __m128i pRs4_1 = _mm_and_si128(_mm_slli_epi32(parallaxR,4),FF128);
const __m128i pLs4_1 = _mm_and_si128(_mm_slli_epi32(parallaxL,4),FF128);
const __m128i pRs4_2 = _mm_sub_epi32(FF128,pRs4_1);
const __m128i pLs4_2 = _mm_sub_epi32(FF128,pLs4_1);

const unsigned int separationR0 = ((unsigned int*)&pR)[0];
const unsigned int separationR1 = ((unsigned int*)&pR)[1];
const unsigned int separationR2 = ((unsigned int*)&pR)[2];
const unsigned int separationR3 = ((unsigned int*)&pR)[3];

//!! opt. the set_epi32's here?!
const __m128i right0 = _mm_set_epi32(buffercopy[separationR3]        ,buffercopy[separationR2]        ,buffercopy[separationR1]        ,buffercopy[separationR0]);
const __m128i right1 = _mm_set_epi32(buffercopy[separationR3 - 1]    ,buffercopy[separationR2 - 1]    ,buffercopy[separationR1 - 1]    ,buffercopy[separationR0 - 1]);

const unsigned int separationL0 = ((unsigned int*)&pL)[0];
const unsigned int separationL1 = ((unsigned int*)&pL)[1];
const unsigned int separationL2 = ((unsigned int*)&pL)[2];
const unsigned int separationL3 = ((unsigned int*)&pL)[3];

const __m128i left0 = _mm_set_epi32(buffercopy[separationL3]        ,buffercopy[separationL2]        ,buffercopy[separationL1]        ,buffercopy[separationL0]);
const __m128i left1 = _mm_set_epi32(buffercopy[separationL3 + 1]    ,buffercopy[separationL2 + 1]    ,buffercopy[separationL1 + 1]    ,buffercopy[separationL0 + 1]);

const __m128i right00 = _mm_mul_int(_mm_and_si128(right0,FF00FF128),pRs4_1);
const __m128i right01 = _mm_mul_int(_mm_and_si128(right0,FF00128),  pRs4_1);
const __m128i right10 = _mm_mul_int(_mm_and_si128(right1,FF00FF128),pRs4_2);
const __m128i right11 = _mm_mul_int(_mm_and_si128(right1,FF00128),  pRs4_2);

__m128i right = _mm_srli_epi32(_mm_or_si128(_mm_and_si128(_mm_add_epi32(right00,right10),FF00FF00128),_mm_and_si128(_mm_add_epi32(right01,right11),FF0000128)),8);

if(AA & y) // so always triggered if AA and y&1
	right = _mm_srli_epi32(_mm_add_epi32(_mm_and_si128(_mm_load_si128((__m128i*)(bufferfinal+offshalf0+x)),FEFEFE128), _mm_and_si128(right,FEFEFE128)),1);

_mm_stream_si128((__m128i*)(bufferfinal+offshalf0+x), right);

const __m128i left00 = _mm_mul_int(_mm_and_si128(left0,FF00FF128),pLs4_1);
const __m128i left01 = _mm_mul_int(_mm_and_si128(left0,FF00128),  pLs4_1);
const __m128i left10 = _mm_mul_int(_mm_and_si128(left1,FF00FF128),pLs4_2);
const __m128i left11 = _mm_mul_int(_mm_and_si128(left1,FF00128),  pLs4_2);

__m128i left = _mm_srli_epi32(_mm_or_si128(_mm_and_si128(_mm_add_epi32(left00,left10),FF00FF00128),_mm_and_si128(_mm_add_epi32(left01,left11),FF0000128)),8);

if(AA & y) // so always triggered if AA and y&1
	left = _mm_srli_epi32(_mm_add_epi32(_mm_and_si128(_mm_load_si128((__m128i*)(bufferfinal+offshalf1+x)),FEFEFE128), _mm_and_si128(left,FEFEFE128)),1);

_mm_stream_si128((__m128i*)(bufferfinal+offshalf1+x), left);
}

if(handle_borders)
    for(; x < (int)width; ++x) //!! black out border pixels, replicate borders for one half instead??
	    bufferfinal[offshalf0 + x] = bufferfinal[offshalf1 + x] = 0;
}
}

static const __m128i FF4128 = _mm_set1_epi32(0xFFu*4);
static const __m128i FF004128 = _mm_set1_epi32(0xFF00u*4);
static const __m128i FF00004128 = _mm_set1_epi32(0xFF0000u*4);
static const __m128i F128 = _mm_set1_epi32(0xFu);
static const __m128i FE00FE00128 = _mm_set1_epi32(0xFE00FE00u);
static const __m128i FE0000128 = _mm_set1_epi32(0x00FE0000u);
static const __m128  x7FFFFFFF128 = (__m128&)_mm_set1_epi32(0x7FFFFFFF);
static const __m128i FCFCFC128 = _mm_set1_epi32(0xFCFCFC);
static const __m128i ZEROFIRST = _mm_set_epi32(0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0);

#define FXAA_SPAN_MAX 8
#define FXAA_OFFS (((FXAA_SPAN_MAX*8)>>4) + 1)

static const __m128 fFXAA_SPAN_MAX  = _mm_set1_ps( (float)(FXAA_SPAN_MAX*8));
static const __m128 mfFXAA_SPAN_MAX = _mm_set1_ps(-(float)(FXAA_SPAN_MAX*8));

__forceinline __m128i luma(const __m128i &rgb)
{
	return _mm_add_epi32(
	 _mm_add_epi32(_mm_srli_epi32(_mm_and_si128(rgb,FF128),3),
	 			   _mm_srli_epi32(_mm_and_si128(rgb,FF00128),9)),
	 _mm_srli_epi32(_mm_and_si128(rgb,FF0000128),18)); //!! R,B swapped
}

__forceinline __m128i lumas2(const __m128i &rgb)
{
	return _mm_add_epi32(
	 _mm_add_epi32(_mm_srli_epi32(_mm_and_si128(rgb,FF4128),5),
	 			   _mm_srli_epi32(_mm_and_si128(rgb,FF004128),11)),
	 _mm_srli_epi32(_mm_and_si128(rgb,FF00004128),20)); //!! R,B swapped
}

__forceinline __m128i bilerpFE(const unsigned int* const __restrict pic, const __m128i &offs, __m128i dx, __m128i dy, const __m128i &nPitch128, const unsigned int nPitch)
{
const __m128i dy4n  = _mm_add_epi32(_mm_srai_epi32(dx,4),_mm_mul_int_i(_mm_srai_epi32(dy,4),nPitch128));
const __m128i offsa = _mm_add_epi32(offs,dy4n);
const __m128i offsb = _mm_sub_epi32(offs,dy4n);
dx = _mm_and_si128(dx,F128);
dy = _mm_and_si128(dy,F128);
const __m128i xy  = _mm_mul_int(dx,dy);
const __m128i x16 = _mm_slli_epi32(dx,4);
const __m128i invxy = _mm_sub_epi32(_mm_slli_epi32(dy,4),xy);
const __m128i xinvy = _mm_sub_epi32(x16,xy);
const __m128i invxinvy = _mm_sub_epi32(_mm_sub_epi32(_mm_set1_epi32(256),x16),invxy);

const unsigned int offsa0 = ((int*)&offsa)[0];
const unsigned int offsa1 = ((int*)&offsa)[1];
const unsigned int offsa2 = ((int*)&offsa)[2];
const unsigned int offsa3 = ((int*)&offsa)[3];

const __m128i r00a = _mm_set_epi32(pic[offsa3],         pic[offsa2],         pic[offsa1],         pic[offsa0]); //!! opt.?
const __m128i r10a = _mm_set_epi32(pic[offsa3+1],       pic[offsa2+1],       pic[offsa1+1],       pic[offsa0+1]);
const __m128i r01a = _mm_set_epi32(pic[offsa3+nPitch],  pic[offsa2+nPitch],  pic[offsa1+nPitch],  pic[offsa0+nPitch]);
const __m128i r11a = _mm_set_epi32(pic[offsa3+nPitch+1],pic[offsa2+nPitch+1],pic[offsa1+nPitch+1],pic[offsa0+nPitch+1]);

const __m128i lerp0 = _mm_srli_epi32(_mm_or_si128(
	 _mm_and_si128(_mm_add_epi32(
	  _mm_add_epi32(_mm_mul_int(_mm_and_si128(r00a,FF00FF128),invxinvy),
				    _mm_mul_int(_mm_and_si128(r10a,FF00FF128),xinvy)),
	  _mm_add_epi32(_mm_mul_int(_mm_and_si128(r01a,FF00FF128),invxy),
				    _mm_mul_int(_mm_and_si128(r11a,FF00FF128),xy))),
	 FE00FE00128),
	 _mm_and_si128(_mm_add_epi32(
	  _mm_add_epi32(_mm_mul_int(_mm_and_si128(r00a,FF00128),invxinvy),
				    _mm_mul_int(_mm_and_si128(r10a,FF00128),xinvy)),
	  _mm_add_epi32(_mm_mul_int(_mm_and_si128(r01a,FF00128),invxy),
				    _mm_mul_int(_mm_and_si128(r11a,FF00128),xy))),
	 FE0000128)
    ),9);

const unsigned int offsb0 = ((int*)&offsb)[0];
const unsigned int offsb1 = ((int*)&offsb)[1];
const unsigned int offsb2 = ((int*)&offsb)[2];
const unsigned int offsb3 = ((int*)&offsb)[3];

const __m128i r11b = _mm_set_epi32(pic[offsb3-1-nPitch],pic[offsb2-1-nPitch],pic[offsb1-1-nPitch],pic[offsb0-1-nPitch]);
const __m128i r01b = _mm_set_epi32(pic[offsb3-nPitch],  pic[offsb2-nPitch],  pic[offsb1-nPitch],  pic[offsb0-nPitch]);
const __m128i r10b = _mm_set_epi32(pic[offsb3-1],       pic[offsb2-1],       pic[offsb1-1],       pic[offsb0-1]);
const __m128i r00b = _mm_set_epi32(pic[offsb3],         pic[offsb2],         pic[offsb1],         pic[offsb0]);

return _mm_add_epi32(lerp0,
    _mm_srli_epi32(_mm_or_si128(
	 _mm_and_si128(_mm_add_epi32(
	   _mm_add_epi32(_mm_mul_int(_mm_and_si128(r00b,FF00FF128),invxinvy),
				     _mm_mul_int(_mm_and_si128(r10b,FF00FF128),xinvy)),
	   _mm_add_epi32(_mm_mul_int(_mm_and_si128(r01b,FF00FF128),invxy),
					 _mm_mul_int(_mm_and_si128(r11b,FF00FF128),xy))),
	 FE00FE00128),
	 _mm_and_si128(_mm_add_epi32(
	   _mm_add_epi32(_mm_mul_int(_mm_and_si128(r00b,FF00128),invxinvy),
		 		     _mm_mul_int(_mm_and_si128(r10b,FF00128),xinvy)),
	   _mm_add_epi32(_mm_mul_int(_mm_and_si128(r01b,FF00128),invxy),
				     _mm_mul_int(_mm_and_si128(r11b,FF00128),xy))),
	 FE0000128)
	),9));
}

__forceinline void fxaa_32bit(const int ystart, const int yend, const int xstart, const int xend, const unsigned int width, const unsigned int owidth, const unsigned int nwidth, const unsigned int height, const unsigned int* const __restrict buffercopy, unsigned int* const __restrict bufferfinal, unsigned char* const __restrict mask, const bool handle_borders)
{
	if(handle_borders)
	{
		for(int y = 0; y < ystart; ++y)
			memcpy(bufferfinal+y*nwidth,buffercopy+y*owidth,width*4);
		for(int y = yend; y < (int)height; ++y)
			memcpy(bufferfinal+y*nwidth,buffercopy+y*owidth,width*4);
	}

	const __m128i owidth128 = _mm_set1_epi32(owidth);

#pragma omp parallel for schedule(dynamic)
	for(int y = ystart; y < yend; ++y)
	{
	unsigned int offsm1   = (y-1)*owidth - 1 + xstart;
	unsigned int offsn    = y*nwidth + xstart;
	unsigned int offsmask = (y*width + xstart)>>2;
	__m128i x128ynPitch128 = _mm_add_epi32(t0123,_mm_set1_epi32(y*owidth + xstart));

	if(handle_borders)
	{
		for(int x = 0; x < xstart; ++x)
			bufferfinal[offsn-xstart+x] = buffercopy[offsm1-xstart+owidth+1+x];
		for(int x = xend; x < (int)width; ++x)
			bufferfinal[offsn-xstart+x] = buffercopy[offsm1-xstart+owidth+1+x];
	}

	for(int x = xstart; x < xend; x+=4,offsm1+=4,offsn+=4,++offsmask,x128ynPitch128=_mm_add_epi32(x128ynPitch128,t4444)) if(mask[offsmask] == 0)
	{
		mask[offsmask] = 1;

		//!! sliding window instead? (on y-1,y,y+1), incl. the filtered values already?
		const __m128i NW  = _mm_and_si128((__m128i&)_mm_load_ss((float*)(buffercopy+offsm1)),FCFCFC128);
		const __m128i rN  = _mm_and_si128(_mm_load_si128((__m128i*)(buffercopy+offsm1+1)),FCFCFC128);
		const __m128i NE  = _mm_and_si128((__m128i&)_mm_load_ss((float*)(buffercopy+offsm1+5)),FCFCFC128);
		const __m128i rNW = _mm_or_si128(NW,_mm_and_si128(_mm_shuffle_epi32(rN,_MM_SHUFFLE(2,1,0,0)),ZEROFIRST));
		const __m128i rNE = _mm_shuffle_epi32(_mm_or_si128(NE,_mm_and_si128(rN,ZEROFIRST)),_MM_SHUFFLE(0,3,2,1));

		const unsigned int offs = offsm1+owidth;
		const __m128i W   = _mm_and_si128((__m128i&)_mm_load_ss((float*)(buffercopy+offs)),FCFCFC128);
		const __m128i rM  = _mm_and_si128(_mm_load_si128((__m128i*)(buffercopy+offs+1)),FCFCFC128);
		const __m128i E   = _mm_and_si128((__m128i&)_mm_load_ss((float*)(buffercopy+offs+5)),FCFCFC128);
		const __m128i rW  = _mm_or_si128(W,_mm_and_si128(_mm_shuffle_epi32(rM,_MM_SHUFFLE(2,1,0,0)),ZEROFIRST));
		const __m128i rE  = _mm_shuffle_epi32(_mm_or_si128(E,_mm_and_si128(rM,ZEROFIRST)),_MM_SHUFFLE(0,3,2,1));

		const unsigned int offsp1 = offs+owidth;
		const __m128i SW  = _mm_and_si128((__m128i&)_mm_load_ss((float*)(buffercopy+offsp1)),FCFCFC128);
		const __m128i rS  = _mm_and_si128(_mm_load_si128((__m128i*)(buffercopy+offsp1+1)),FCFCFC128);
		const __m128i SE  = _mm_and_si128((__m128i&)_mm_load_ss((float*)(buffercopy+offsp1+5)),FCFCFC128);
		const __m128i rSW = _mm_or_si128(SW,_mm_and_si128(_mm_shuffle_epi32(rS,_MM_SHUFFLE(2,1,0,0)),ZEROFIRST));
		const __m128i rSE = _mm_shuffle_epi32(_mm_or_si128(SE,_mm_and_si128(rS,ZEROFIRST)),_MM_SHUFFLE(0,3,2,1));

		const __m128i rMrN   = _mm_add_epi32(rM,rN);
		const __m128i lumaNW = lumas2(_mm_add_epi32(_mm_add_epi32(rMrN,rNW),rW));
		const __m128i lumaNE = lumas2(_mm_add_epi32(_mm_add_epi32(rMrN,rNE),rE));
		const __m128i rMrS   = _mm_add_epi32(rM,rS);
		const __m128i lumaSW = lumas2(_mm_add_epi32(_mm_add_epi32(rMrS,rSW),rW));
		const __m128i lumaSE = lumas2(_mm_add_epi32(_mm_add_epi32(rMrS,rSE),rE));
		const __m128i lumaM  = luma(rM);

		const __m128i maskS = _mm_cmpgt_epi32(lumaSW,lumaSE);
		const __m128i maskN = _mm_cmpgt_epi32(lumaNW,lumaNE);

#if defined(__SSE4__) || defined(__SSE4_1__) || defined(__SSE4_2__)
#warning uses SSE4
		const __m128i tempMax  = (__m128i&)_mm_blendv_ps((__m128&)lumaSE,(__m128&)lumaSW,(__m128&)maskS);
		const __m128i tempMin  = (__m128i&)_mm_blendv_ps((__m128&)lumaSW,(__m128&)lumaSE,(__m128&)maskS);

		const __m128i tempMax2 = (__m128i&)_mm_blendv_ps((__m128&)lumaNE,(__m128&)lumaNW,(__m128&)maskN);
		const __m128i tempMin2 = (__m128i&)_mm_blendv_ps((__m128&)lumaNW,(__m128&)lumaNE,(__m128&)maskN);
#else
		const __m128i tempMax  = _mm_or_si128(_mm_and_si128(maskS,lumaSW),_mm_andnot_si128(maskS,lumaSE));
		const __m128i tempMin  = _mm_or_si128(_mm_and_si128(maskS,lumaSE),_mm_andnot_si128(maskS,lumaSW));

		const __m128i tempMax2 = _mm_or_si128(_mm_and_si128(maskN,lumaNW),_mm_andnot_si128(maskN,lumaNE));
		const __m128i tempMin2 = _mm_or_si128(_mm_and_si128(maskN,lumaNE),_mm_andnot_si128(maskN,lumaNW));
#endif
		const __m128i SWSE = _mm_add_epi32(lumaSW,lumaSE);
		const __m128i NWNE = _mm_add_epi32(lumaNW,lumaNE);

		const __m128 fdirx = _mm_cvtepi32_ps(_mm_sub_epi32(SWSE,NWNE));
		const __m128 fdiry = _mm_cvtepi32_ps(_mm_sub_epi32(_mm_add_epi32(lumaNW,lumaSW),_mm_add_epi32(lumaNE,lumaSE)));

		const __m128 temp = _mm_rcp_ps(_mm_add_ps(_mm_mul_ps(_mm_min_ps(_mm_and_ps(fdirx,x7FFFFFFF128),
			                                                            _mm_and_ps(fdiry,x7FFFFFFF128)),
															 _mm_set1_ps((float)(1.0/8.0))),
									              _mm_max_ps(_mm_mul_ps(_mm_cvtepi32_ps(_mm_add_epi32(NWNE,SWSE)),_mm_set1_ps((float)(1.0/256.0))), _mm_set1_ps((float)(1.0/4.0)))));

		const __m128i dirx = _mm_cvtps_epi32(_mm_min_ps(fFXAA_SPAN_MAX, _mm_max_ps(mfFXAA_SPAN_MAX, _mm_mul_ps(fdirx, temp))));
		const __m128i diry = _mm_cvtps_epi32(_mm_min_ps(fFXAA_SPAN_MAX, _mm_max_ps(mfFXAA_SPAN_MAX, _mm_mul_ps(fdiry, temp))));

		      __m128i rgbB = bilerpFE(buffercopy,x128ynPitch128,               dirx,                  diry,   owidth128,owidth);
		const __m128i rgbA = bilerpFE(buffercopy,x128ynPitch128,_mm_srai_epi32(dirx,2),_mm_srai_epi32(diry,2),owidth128,owidth);

		rgbB = _mm_srli_epi32(_mm_add_epi32(_mm_and_si128(rgbA,FEFEFE128) , _mm_and_si128(rgbB,FEFEFE128)) , 1);
		const __m128i lumaB = luma(rgbB);

		const __m128i maskL = _mm_or_si128(
								_mm_and_si128(_mm_and_si128(_mm_cmplt_epi32(lumaB,lumaM),_mm_cmplt_epi32(lumaB,tempMin)),_mm_cmplt_epi32(lumaB,tempMin2)),
								_mm_and_si128(_mm_and_si128(_mm_cmpgt_epi32(lumaB,lumaM),_mm_cmpgt_epi32(lumaB,tempMax)),_mm_cmpgt_epi32(lumaB,tempMax2))
							  );
		_mm_store_si128((__m128i*)(bufferfinal+offsn) ,
#if defined(__SSE4__) || defined(__SSE4_1__) || defined(__SSE4_2__)
#warning uses SSE4
			(__m128i&)_mm_blendv_ps((__m128&)rgbB,(__m128&)rgbA,(__m128&)maskL));
#else
			_mm_or_si128(_mm_and_si128(maskL,rgbA),_mm_andnot_si128(maskL,rgbB)));
#endif
	}
	}
}

__forceinline __m128i bilerpFE16(const unsigned short* const __restrict pic, const __m128i &offs, __m128i dx, __m128i dy, const __m128i &nPitch128, const unsigned int nPitch)
{
const __m128i dy4n  = _mm_add_epi32(_mm_srai_epi32(dx,4),_mm_mul_int_i(_mm_srai_epi32(dy,4),nPitch128));
const __m128i offsa = _mm_add_epi32(offs,dy4n);
const __m128i offsb = _mm_sub_epi32(offs,dy4n);
dx = _mm_and_si128(dx,F128);
dy = _mm_and_si128(dy,F128);
const __m128i xy  = _mm_mul_int(dx,dy);
const __m128i x16 = _mm_slli_epi32(dx,4);
const __m128i invxy = _mm_sub_epi32(_mm_slli_epi32(dy,4),xy);
const __m128i xinvy = _mm_sub_epi32(x16,xy);
const __m128i invxinvy = _mm_sub_epi32(_mm_sub_epi32(_mm_set1_epi32(256),x16),invxy);

const unsigned int offsa0 = ((int*)&offsa)[0];
const unsigned int offsa1 = ((int*)&offsa)[1];
const unsigned int offsa2 = ((int*)&offsa)[2];
const unsigned int offsa3 = ((int*)&offsa)[3];

const __m128i r00a = unpack_565(_mm_set_epi32(pic[offsa3],         pic[offsa2],         pic[offsa1],         pic[offsa0])); //!! opt.?
const __m128i r10a = unpack_565(_mm_set_epi32(pic[offsa3+1],       pic[offsa2+1],       pic[offsa1+1],       pic[offsa0+1]));
const __m128i r01a = unpack_565(_mm_set_epi32(pic[offsa3+nPitch],  pic[offsa2+nPitch],  pic[offsa1+nPitch],  pic[offsa0+nPitch]));
const __m128i r11a = unpack_565(_mm_set_epi32(pic[offsa3+nPitch+1],pic[offsa2+nPitch+1],pic[offsa1+nPitch+1],pic[offsa0+nPitch+1]));

const __m128i lerp0 = _mm_srli_epi32(_mm_or_si128(
	 _mm_and_si128(_mm_add_epi32(
	  _mm_add_epi32(_mm_mul_int(_mm_and_si128(r00a,FF00FF128),invxinvy),
				    _mm_mul_int(_mm_and_si128(r10a,FF00FF128),xinvy)),
	  _mm_add_epi32(_mm_mul_int(_mm_and_si128(r01a,FF00FF128),invxy),
				    _mm_mul_int(_mm_and_si128(r11a,FF00FF128),xy))),
	 FE00FE00128),
	 _mm_and_si128(_mm_add_epi32(
	  _mm_add_epi32(_mm_mul_int(_mm_and_si128(r00a,FF00128),invxinvy),
				    _mm_mul_int(_mm_and_si128(r10a,FF00128),xinvy)),
	  _mm_add_epi32(_mm_mul_int(_mm_and_si128(r01a,FF00128),invxy),
				    _mm_mul_int(_mm_and_si128(r11a,FF00128),xy))),
	 FE0000128)
    ),9);

const unsigned int offsb0 = ((int*)&offsb)[0];
const unsigned int offsb1 = ((int*)&offsb)[1];
const unsigned int offsb2 = ((int*)&offsb)[2];
const unsigned int offsb3 = ((int*)&offsb)[3];

const __m128i r11b = unpack_565(_mm_set_epi32(pic[offsb3-1-nPitch],pic[offsb2-1-nPitch],pic[offsb1-1-nPitch],pic[offsb0-1-nPitch]));
const __m128i r01b = unpack_565(_mm_set_epi32(pic[offsb3-nPitch],  pic[offsb2-nPitch],  pic[offsb1-nPitch],  pic[offsb0-nPitch]));
const __m128i r10b = unpack_565(_mm_set_epi32(pic[offsb3-1],       pic[offsb2-1],       pic[offsb1-1],       pic[offsb0-1]));
const __m128i r00b = unpack_565(_mm_set_epi32(pic[offsb3],         pic[offsb2],         pic[offsb1],         pic[offsb0]));

return _mm_add_epi32(lerp0,
    _mm_srli_epi32(_mm_or_si128(
	 _mm_and_si128(_mm_add_epi32(
	   _mm_add_epi32(_mm_mul_int(_mm_and_si128(r00b,FF00FF128),invxinvy),
				     _mm_mul_int(_mm_and_si128(r10b,FF00FF128),xinvy)),
	   _mm_add_epi32(_mm_mul_int(_mm_and_si128(r01b,FF00FF128),invxy),
					 _mm_mul_int(_mm_and_si128(r11b,FF00FF128),xy))),
	 FE00FE00128),
	 _mm_and_si128(_mm_add_epi32(
	   _mm_add_epi32(_mm_mul_int(_mm_and_si128(r00b,FF00128),invxinvy),
		 		     _mm_mul_int(_mm_and_si128(r10b,FF00128),xinvy)),
	   _mm_add_epi32(_mm_mul_int(_mm_and_si128(r01b,FF00128),invxy),
				     _mm_mul_int(_mm_and_si128(r11b,FF00128),xy))),
	 FE0000128)
	),9));
}

__forceinline void fxaa_16bit(const int ystart, const int yend, const int xstart, const int xend, const unsigned int width, const unsigned int owidth, const unsigned int nwidth, const unsigned int height, const unsigned short* const __restrict buffercopy, unsigned short* const __restrict bufferfinal, unsigned char* const __restrict mask, const bool handle_borders)
{
	if(handle_borders)
	{
		for(int y = 0; y < ystart; ++y)
			memcpy(bufferfinal+y*nwidth,buffercopy+y*owidth,width*2);
		for(int y = yend; y < (int)height; ++y)
			memcpy(bufferfinal+y*nwidth,buffercopy+y*owidth,width*2);
	}

	const __m128i owidth128 = _mm_set1_epi32(owidth);

#pragma omp parallel for schedule(dynamic)
	for(int y = ystart; y < yend; ++y)
	{
	unsigned int offsm1   = (y-1)*owidth - 1 + xstart;
	unsigned int offsn    = y*nwidth + xstart;
	unsigned int offsmask = (y*width + xstart)>>2;
	__m128i x128ynPitch128 = _mm_add_epi32(t0123,_mm_set1_epi32(y*owidth + xstart));

	if(handle_borders)
	{
		for(int x = 0; x < xstart; ++x)
			bufferfinal[offsn-xstart+x] = buffercopy[offsm1-xstart+owidth+1+x];
		for(int x = xend; x < (int)width; ++x)
			bufferfinal[offsn-xstart+x] = buffercopy[offsm1-xstart+owidth+1+x];
	}

	for(int x = xstart; x < xend; x+=4,offsm1+=4,offsn+=4,++offsmask,x128ynPitch128=_mm_add_epi32(x128ynPitch128,t4444)) if(mask[offsmask] == 0)
	{
		mask[offsmask] = 1;

		//!! sliding window instead? (on y-1,y,y+1), incl. the filtered values already?
		const unsigned int NWi = buffercopy[offsm1];
		const __m128i NW  = (__m128i&)_mm_set_ss(int_as_float(((NWi&31)<<3) | ((NWi&(63<<5))<<5) | ((NWi&(31<<11))<<8)));
		const __m128i rN  = unpack_565(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(buffercopy+offsm1+1)),_mm_setzero_si128()));
		const unsigned int NEi = buffercopy[offsm1+5];
		const __m128i NE  = (__m128i&)_mm_set_ss(int_as_float(((NEi&31)<<3) | ((NEi&(63<<5))<<5) | ((NEi&(31<<11))<<8)));
		const __m128i rNW = _mm_or_si128(NW,_mm_and_si128(_mm_shuffle_epi32(rN,_MM_SHUFFLE(2,1,0,0)),ZEROFIRST));
		const __m128i rNE = _mm_shuffle_epi32(_mm_or_si128(NE,_mm_and_si128(rN,ZEROFIRST)),_MM_SHUFFLE(0,3,2,1));

		const unsigned int offs = offsm1+owidth;
		const unsigned int Wi = buffercopy[offs];
		const __m128i W   = (__m128i&)_mm_set_ss(int_as_float(((Wi&31)<<3) | ((Wi&(63<<5))<<5) | ((Wi&(31<<11))<<8)));
		const __m128i rM  = unpack_565(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(buffercopy+offs+1)),_mm_setzero_si128()));
		const unsigned int Ei = buffercopy[offs+5];
		const __m128i E   = (__m128i&)_mm_set_ss(int_as_float(((Ei&31)<<3) | ((Ei&(63<<5))<<5) | ((Ei&(31<<11))<<8)));
		const __m128i rW  = _mm_or_si128(W,_mm_and_si128(_mm_shuffle_epi32(rM,_MM_SHUFFLE(2,1,0,0)),ZEROFIRST));
		const __m128i rE  = _mm_shuffle_epi32(_mm_or_si128(E,_mm_and_si128(rM,ZEROFIRST)),_MM_SHUFFLE(0,3,2,1));

		const unsigned int offsp1 = offs+owidth;
		const unsigned int SWi = buffercopy[offsp1];
		const __m128i SW  = (__m128i&)_mm_set_ss(int_as_float(((SWi&31)<<3) | ((SWi&(63<<5))<<5) | ((SWi&(31<<11))<<8)));
		const __m128i rS  = unpack_565(_mm_unpacklo_epi16(_mm_loadl_epi64((__m128i*)(buffercopy+offsp1+1)),_mm_setzero_si128()));
		const unsigned int SEi = buffercopy[offsp1+5];
		const __m128i SE  = (__m128i&)_mm_set_ss(int_as_float(((SEi&31)<<3) | ((SEi&(63<<5))<<5) | ((SEi&(31<<11))<<8)));
		const __m128i rSW = _mm_or_si128(SW,_mm_and_si128(_mm_shuffle_epi32(rS,_MM_SHUFFLE(2,1,0,0)),ZEROFIRST));
		const __m128i rSE = _mm_shuffle_epi32(_mm_or_si128(SE,_mm_and_si128(rS,ZEROFIRST)),_MM_SHUFFLE(0,3,2,1));

		const __m128i rMrN   = _mm_add_epi32(rM,rN);
		const __m128i lumaNW = lumas2(_mm_add_epi32(_mm_add_epi32(rMrN,rNW),rW));
		const __m128i lumaNE = lumas2(_mm_add_epi32(_mm_add_epi32(rMrN,rNE),rE));
		const __m128i rMrS   = _mm_add_epi32(rM,rS);
		const __m128i lumaSW = lumas2(_mm_add_epi32(_mm_add_epi32(rMrS,rSW),rW));
		const __m128i lumaSE = lumas2(_mm_add_epi32(_mm_add_epi32(rMrS,rSE),rE));
		const __m128i lumaM  = luma(rM);

		const __m128i maskS = _mm_cmpgt_epi32(lumaSW,lumaSE);
		const __m128i maskN = _mm_cmpgt_epi32(lumaNW,lumaNE);

#if defined(__SSE4__) || defined(__SSE4_1__) || defined(__SSE4_2__)
#warning uses SSE4
		const __m128i tempMax  = (__m128i&)_mm_blendv_ps((__m128&)lumaSE,(__m128&)lumaSW,(__m128&)maskS);
		const __m128i tempMin  = (__m128i&)_mm_blendv_ps((__m128&)lumaSW,(__m128&)lumaSE,(__m128&)maskS);

		const __m128i tempMax2 = (__m128i&)_mm_blendv_ps((__m128&)lumaNE,(__m128&)lumaNW,(__m128&)maskN);
		const __m128i tempMin2 = (__m128i&)_mm_blendv_ps((__m128&)lumaNW,(__m128&)lumaNE,(__m128&)maskN);
#else
		const __m128i tempMax  = _mm_or_si128(_mm_and_si128(maskS,lumaSW),_mm_andnot_si128(maskS,lumaSE));
		const __m128i tempMin  = _mm_or_si128(_mm_and_si128(maskS,lumaSE),_mm_andnot_si128(maskS,lumaSW));

		const __m128i tempMax2 = _mm_or_si128(_mm_and_si128(maskN,lumaNW),_mm_andnot_si128(maskN,lumaNE));
		const __m128i tempMin2 = _mm_or_si128(_mm_and_si128(maskN,lumaNE),_mm_andnot_si128(maskN,lumaNW));
#endif
		const __m128i SWSE = _mm_add_epi32(lumaSW,lumaSE);
		const __m128i NWNE = _mm_add_epi32(lumaNW,lumaNE);

		const __m128 fdirx = _mm_cvtepi32_ps(_mm_sub_epi32(SWSE,NWNE));
		const __m128 fdiry = _mm_cvtepi32_ps(_mm_sub_epi32(_mm_add_epi32(lumaNW,lumaSW),_mm_add_epi32(lumaNE,lumaSE)));

		const __m128 temp = _mm_rcp_ps(_mm_add_ps(_mm_mul_ps(_mm_min_ps(_mm_and_ps(fdirx,x7FFFFFFF128),
			                                                            _mm_and_ps(fdiry,x7FFFFFFF128)),
															 _mm_set1_ps((float)(1.0/8.0))),
									              _mm_max_ps(_mm_mul_ps(_mm_cvtepi32_ps(_mm_add_epi32(NWNE,SWSE)),_mm_set1_ps((float)(1.0/256.0))), _mm_set1_ps((float)(1.0/4.0)))));

		const __m128i dirx = _mm_cvtps_epi32(_mm_min_ps(fFXAA_SPAN_MAX, _mm_max_ps(mfFXAA_SPAN_MAX, _mm_mul_ps(fdirx, temp))));
		const __m128i diry = _mm_cvtps_epi32(_mm_min_ps(fFXAA_SPAN_MAX, _mm_max_ps(mfFXAA_SPAN_MAX, _mm_mul_ps(fdiry, temp))));

		      __m128i rgbB = bilerpFE16(buffercopy,x128ynPitch128,               dirx,                  diry,   owidth128,owidth);
		const __m128i rgbA = bilerpFE16(buffercopy,x128ynPitch128,_mm_srai_epi32(dirx,2),_mm_srai_epi32(diry,2),owidth128,owidth);

		rgbB = _mm_srli_epi32(_mm_add_epi32(_mm_and_si128(rgbA,FEFEFE128) , _mm_and_si128(rgbB,FEFEFE128)) , 1);
		const __m128i lumaB = luma(rgbB);

		const __m128i maskL = _mm_or_si128(
								_mm_and_si128(_mm_and_si128(_mm_cmplt_epi32(lumaB,lumaM),_mm_cmplt_epi32(lumaB,tempMin)),_mm_cmplt_epi32(lumaB,tempMin2)),
								_mm_and_si128(_mm_and_si128(_mm_cmpgt_epi32(lumaB,lumaM),_mm_cmpgt_epi32(lumaB,tempMax)),_mm_cmpgt_epi32(lumaB,tempMax2))
							  );
		_mm_storel_pi((__m64*)(bufferfinal+offsn) , (__m128&)pack_565(
#if defined(__SSE4__) || defined(__SSE4_1__) || defined(__SSE4_2__)
#warning uses SSE4
			(__m128i&)_mm_blendv_ps((__m128&)rgbB,(__m128&)rgbA,(__m128&)maskL)));
#else
			_mm_or_si128(_mm_and_si128(maskL,rgbA),_mm_andnot_si128(maskL,rgbB))));
#endif
	}
	}
}
