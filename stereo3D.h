#include <emmintrin.h>

static const unsigned int f0 = 0xFFFFFFFFu;
static const unsigned int t0 = 0;
static const unsigned int FF00FF = 0xFF00FFu;
static const unsigned int FF00 = 0xFF00u;
static const unsigned int FF00FF00 = 0xFF00FF00u;
static const unsigned int FF0000 = 0xFF0000u;
static const unsigned int FEFEFE = 0xFEFEFEu;
static const __m128 f0128 = _mm_set_ps((float&)t0,(float&)f0,(float&)t0,(float&)f0);
static const __m128i t0123 = _mm_set_epi32(3,2,1,0);
static const __m128i t4444 = _mm_set1_epi32(4);
static const __m128i FF128 = _mm_set1_epi32(0xFF);
static const __m128 FF00FF128 = _mm_set1_ps((float&)FF00FF);
static const __m128 FF00128 = _mm_set1_ps((float&)FF00);
static const __m128 FF00FF00128 = _mm_set1_ps((float&)FF00FF00);
static const __m128 FF0000128 = _mm_set1_ps((float&)FF0000);
static const __m128 FEFEFE128 = _mm_set1_ps((float&)FEFEFE);
static const __m128i mask_565_rb = _mm_set1_epi32(0x00F800F8);
static const __m128i mask_565_pack_multiplier = _mm_set1_epi32(0x20000004);
static const __m128i mask_red = _mm_set1_epi32(0x000000F8);
static const __m128i mask_green = _mm_set1_epi32(0x0000FC00);
static const __m128i mask_blue = _mm_set1_epi32(0x00F80000);

__forceinline __m128 pack_565(const __m128i &lo)
{
    __m128i t0 = _mm_or_si128(_mm_madd_epi16(_mm_and_si128(lo, mask_565_rb), mask_565_pack_multiplier), _mm_and_si128(lo, mask_green));

    //!! could use _mm_packus_epi32 on sse4
    t0 = _mm_srai_epi32(_mm_slli_epi32(t0, 16 - 5), 16);
    return (__m128&)_mm_packs_epi32(t0, t0);
}

__forceinline __m128 unpack_565(const __m128i& lo)
{
    return (__m128&)_mm_or_si128(_mm_or_si128(_mm_and_si128(_mm_slli_epi32(lo, 3), mask_red), _mm_and_si128(_mm_slli_epi32(lo, 5), mask_green)), _mm_and_si128(_mm_slli_epi32(lo, 8), mask_blue));
}

__forceinline __m128i _mm_mul_int(const __m128& a, const __m128i& b)
{
    return (__m128i&)_mm_or_ps(
        _mm_and_ps( (__m128&)_mm_mul_epu32( (__m128i&)a,b), f0128),
        (__m128&)_mm_slli_si128(
            (__m128i&)_mm_and_ps( (__m128&)_mm_mul_epu32(_mm_srli_si128((__m128i&)a,4),_mm_srli_si128(b,4)), f0128), 4));
}

__forceinline __m128i _mm_mul_int_i(const __m128i& a, const __m128i& b)
{
    return (__m128i&)_mm_or_ps(
        _mm_and_ps( (__m128&)_mm_mul_epu32(a,b), f0128),
        (__m128&)_mm_slli_si128(
            (__m128i&)_mm_and_ps( (__m128&)_mm_mul_epu32(_mm_srli_si128(a,4),b), f0128), 4));
}

inline void memcpy_sse2(void * const __restrict dst, const void * const __restrict src, const unsigned int nBytes)
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
        jmp         end
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
end:
    }
}

// initial implementation of stereo3D
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

inline void stereo_repro_16bit_y(const int ystart, const int yend, const int xstart, const int xend, const unsigned int width, const unsigned int height, const unsigned int maxSeparationU, const unsigned short * const __restrict buffercopy, const unsigned short * const __restrict bufferzcopy, unsigned short * const __restrict bufferfinal, const unsigned int samples[3], const __m128& zmask128, const __m128& ZPDU128, const __m128& maxSepShl4128, const bool handle_borders, const unsigned int AA, unsigned char* __restrict const mask)
{
if(handle_borders) {
    ZeroMemory(bufferfinal,                               (width*(ystart>>1))<<1);                        //!! black out border pixels, replicate borders for one half instead?? //!! opt. with SSE2?
    ZeroMemory(bufferfinal+             (yend>>1) *width, (width*(((height-yend)>>1) + (ystart>>1)))<<1); //!! black out border pixels, replicate borders for one half instead??
    ZeroMemory(bufferfinal+((height>>1)+(yend>>1))*width, (width* ((height-yend)>>1))<<1);                //!! black out border pixels, replicate borders for one half instead??
}

const __m128i width128 = (__m128i&)_mm_set1_ps((float&)width);

const int incr = (AA^1)+1;

#pragma omp parallel for schedule(dynamic) //!! make configurable for update/non-update version
for(int yi = ystart; yi < yend; yi+=incr)  //!! interleave left/right and calcs instead? (might be faster, too, due to smaller register usage?)
{
const unsigned int y = yi;
const unsigned int offshalf0 = (y>>1)*width;
const unsigned int offshalf1 = (height>>1)*width + offshalf0;
const unsigned int ymms = y - maxSeparationU;
const unsigned int ypms = y + maxSeparationU;

const __m128i ymms128 = (__m128i&)_mm_set1_ps((float&)ymms);
const __m128i ypms128 = (__m128i&)_mm_set1_ps((float&)ypms);

const unsigned short* __restrict z = bufferzcopy + (y*width + xstart);
__m128i x128 = _mm_add_epi32(t0123,_mm_set1_epi32(xstart));

unsigned int xm = y*(width>>2) + (xstart>>2);
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
const __m128 right0 = unpack_565(_mm_set_epi32(buffercopy[separationR3]        ,buffercopy[separationR2]        ,buffercopy[separationR1]        ,buffercopy[separationR0]));
const __m128 right1 = unpack_565(_mm_set_epi32(buffercopy[separationR3 - width],buffercopy[separationR2 - width],buffercopy[separationR1 - width],buffercopy[separationR0 - width]));

const unsigned int separationL0 = ((unsigned int*)&pL)[0];
const unsigned int separationL1 = ((unsigned int*)&pL)[1];
const unsigned int separationL2 = ((unsigned int*)&pL)[2];
const unsigned int separationL3 = ((unsigned int*)&pL)[3];

const __m128 left0 = unpack_565(_mm_set_epi32(buffercopy[separationL3]        ,buffercopy[separationL2]        ,buffercopy[separationL1]        ,buffercopy[separationL0]));
const __m128 left1 = unpack_565(_mm_set_epi32(buffercopy[separationL3 + width],buffercopy[separationL2 + width],buffercopy[separationL1 + width],buffercopy[separationL0 + width]));

const __m128i right00 = _mm_mul_int(_mm_and_ps(right0,FF00FF128),pRs4_1);
const __m128i right01 = _mm_mul_int(_mm_and_ps(right0,FF00128),  pRs4_1);
const __m128i right10 = _mm_mul_int(_mm_and_ps(right1,FF00FF128),pRs4_2);
const __m128i right11 = _mm_mul_int(_mm_and_ps(right1,FF00128),  pRs4_2);

const __m128i right = _mm_srli_epi32((__m128i&)_mm_or_ps(_mm_and_ps((__m128&)_mm_add_epi32(right00,right10),FF00FF00128),_mm_and_ps((__m128&)_mm_add_epi32(right01,right11),FF0000128)),8);

if(AA & y) // so always triggered if AA and y&1
	_mm_storel_pi((__m64*)(bufferfinal+offshalf0+x), pack_565(_mm_srli_epi32(_mm_add_epi32((__m128i&)_mm_and_ps(unpack_565(_mm_set_epi32(bufferfinal[offshalf0+x+3],bufferfinal[offshalf0+x+2],bufferfinal[offshalf0+x+1],bufferfinal[offshalf0+x])),FEFEFE128), (__m128i&)_mm_and_ps((__m128&)right,FEFEFE128)),1))); //!! opt.: this is very very crude code
else
	_mm_storel_pi((__m64*)(bufferfinal+offshalf0+x), pack_565(right));

const __m128i left00 = _mm_mul_int(_mm_and_ps(left0,FF00FF128),pLs4_1);
const __m128i left01 = _mm_mul_int(_mm_and_ps(left0,FF00128),  pLs4_1);
const __m128i left10 = _mm_mul_int(_mm_and_ps(left1,FF00FF128),pLs4_2);
const __m128i left11 = _mm_mul_int(_mm_and_ps(left1,FF00128),  pLs4_2);

const __m128i left = _mm_srli_epi32((__m128i&)_mm_or_ps(_mm_and_ps((__m128&)_mm_add_epi32(left00,left10),FF00FF00128),_mm_and_ps((__m128&)_mm_add_epi32(left01,left11),FF0000128)),8);

if(AA & y) // so always triggered if AA and y&1
	_mm_storel_pi((__m64*)(bufferfinal+offshalf1+x), pack_565(_mm_srli_epi32(_mm_add_epi32((__m128i&)_mm_and_ps(unpack_565(_mm_set_epi32(bufferfinal[offshalf1+x+3],bufferfinal[offshalf1+x+2],bufferfinal[offshalf1+x+1],bufferfinal[offshalf1+x])),FEFEFE128), (__m128i&)_mm_and_ps((__m128&)left,FEFEFE128)),1))); //!! opt.: this is very very crude code
else
	_mm_storel_pi((__m64*)(bufferfinal+offshalf1+x), pack_565(left));
}
}
}

inline void stereo_repro_32bit_y(const int ystart, const int yend, const int xstart, const int xend, const unsigned int width, const unsigned int height, const unsigned int maxSeparationU, const unsigned int   * const __restrict buffercopy, const unsigned int   * const __restrict bufferzcopy, unsigned int   * const __restrict bufferfinal, const unsigned int samples[3], const __m128& zmask128, const __m128& ZPDU128, const __m128& maxSepShl4128, const bool handle_borders, const unsigned int AA, unsigned char* __restrict const mask)
{
if(handle_borders) {
    ZeroMemory(bufferfinal,                               (width*(ystart>>1))<<2);                        //!! black out border pixels, replicate borders for one half instead?? //!! opt. with SSE2?
    ZeroMemory(bufferfinal+             (yend>>1) *width, (width*(((height-yend)>>1) + (ystart>>1)))<<2); //!! black out border pixels, replicate borders for one half instead??
    ZeroMemory(bufferfinal+((height>>1)+(yend>>1))*width, (width* ((height-yend)>>1))<<2);                //!! black out border pixels, replicate borders for one half instead??
}

const __m128i width128 = (__m128i&)_mm_set1_ps((float&)width);

const int incr = (AA^1)+1;

#pragma omp parallel for schedule(dynamic) //!! make configurable for update/non-update version
for(int yi = ystart; yi < yend; yi+=incr)  //!! interleave left/right and calcs instead? (might be faster, too, due to smaller register usage?)
{
const unsigned int y = yi;
const unsigned int offshalf0 = (y>>1)*width;
const unsigned int offshalf1 = (height>>1)*width + offshalf0;
const unsigned int ymms = y - maxSeparationU;
const unsigned int ypms = y + maxSeparationU;

const __m128i ymms128 = (__m128i&)_mm_set1_ps((float&)ymms);
const __m128i ypms128 = (__m128i&)_mm_set1_ps((float&)ypms);

const float * __restrict z = (float*)bufferzcopy + (y*width + xstart);
__m128i x128 = _mm_add_epi32(t0123,_mm_set1_epi32(xstart));

unsigned int xm = y*(width>>2) + (xstart>>2);
int x = xstart;
for(; x < xend; x+=4,z+=4,++xm,x128=_mm_add_epi32(x128,t4444)) if(mask[xm] == 0)
{
mask[xm] = 1;

const __m128 minDepthR = _mm_min_ps(_mm_min_ps(_mm_cvtepi32_ps((__m128i&)_mm_and_ps(_mm_load_ps(z-samples[0]),zmask128)),_mm_cvtepi32_ps((__m128i&)_mm_and_ps(_mm_load_ps(z-samples[1]),zmask128))),_mm_cvtepi32_ps((__m128i&)_mm_and_ps(_mm_load_ps(z-samples[2]),zmask128)));
const __m128 minDepthL = _mm_min_ps(_mm_min_ps(_mm_cvtepi32_ps((__m128i&)_mm_and_ps(_mm_load_ps(z+samples[0]),zmask128)),_mm_cvtepi32_ps((__m128i&)_mm_and_ps(_mm_load_ps(z+samples[1]),zmask128))),_mm_cvtepi32_ps((__m128i&)_mm_and_ps(_mm_load_ps(z+samples[2]),zmask128)));

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
const __m128 right0 =   (__m128&)_mm_set_epi32(buffercopy[separationR3]        ,buffercopy[separationR2]        ,buffercopy[separationR1]        ,buffercopy[separationR0]);
const __m128 right1 =   (__m128&)_mm_set_epi32(buffercopy[separationR3 - width],buffercopy[separationR2 - width],buffercopy[separationR1 - width],buffercopy[separationR0 - width]);

const unsigned int separationL0 = ((unsigned int*)&pL)[0];
const unsigned int separationL1 = ((unsigned int*)&pL)[1];
const unsigned int separationL2 = ((unsigned int*)&pL)[2];
const unsigned int separationL3 = ((unsigned int*)&pL)[3];

const __m128 left0 =   (__m128&)_mm_set_epi32(buffercopy[separationL3]        ,buffercopy[separationL2]        ,buffercopy[separationL1]        ,buffercopy[separationL0]);
const __m128 left1 =   (__m128&)_mm_set_epi32(buffercopy[separationL3 + width],buffercopy[separationL2 + width],buffercopy[separationL1 + width],buffercopy[separationL0 + width]);

const __m128i right00 = _mm_mul_int(_mm_and_ps(right0,FF00FF128),pRs4_1);
const __m128i right01 = _mm_mul_int(_mm_and_ps(right0,FF00128),  pRs4_1);
const __m128i right10 = _mm_mul_int(_mm_and_ps(right1,FF00FF128),pRs4_2);
const __m128i right11 = _mm_mul_int(_mm_and_ps(right1,FF00128),  pRs4_2);

const __m128i right = _mm_srli_epi32((__m128i&)_mm_or_ps(_mm_and_ps((__m128&)_mm_add_epi32(right00,right10),FF00FF00128),_mm_and_ps((__m128&)_mm_add_epi32(right01,right11),FF0000128)),8);

if(AA & y) // so always triggered if AA and y&1
	_mm_stream_si128((__m128i*)(bufferfinal+offshalf0+x),_mm_srli_epi32(_mm_add_epi32((__m128i&)_mm_and_ps(_mm_load_ps((float*)bufferfinal+offshalf0+x),FEFEFE128), (__m128i&)_mm_and_ps((__m128&)right,FEFEFE128)),1));
else
	_mm_stream_si128((__m128i*)(bufferfinal+offshalf0+x), right);

const __m128i left00 = _mm_mul_int(_mm_and_ps(left0,FF00FF128),pLs4_1);
const __m128i left01 = _mm_mul_int(_mm_and_ps(left0,FF00128),  pLs4_1);
const __m128i left10 = _mm_mul_int(_mm_and_ps(left1,FF00FF128),pLs4_2);
const __m128i left11 = _mm_mul_int(_mm_and_ps(left1,FF00128),  pLs4_2);

const __m128i left = _mm_srli_epi32((__m128i&)_mm_or_ps(_mm_and_ps((__m128&)_mm_add_epi32(left00,left10),FF00FF00128),_mm_and_ps((__m128&)_mm_add_epi32(left01,left11),FF0000128)),8);

if(AA & y) // so always triggered if AA and y&1
	_mm_stream_si128((__m128i*)(bufferfinal+offshalf1+x),_mm_srli_epi32(_mm_add_epi32((__m128i&)_mm_and_ps(_mm_load_ps((float*)bufferfinal+offshalf1+x),FEFEFE128), (__m128i&)_mm_and_ps((__m128&)left,FEFEFE128)),1));
else
	_mm_stream_si128((__m128i*)(bufferfinal+offshalf1+x), left);
}
}
}

inline void stereo_repro_16bit_x(const int ystart, const int yend, const int xstart, const int xend, const unsigned int width, const unsigned int height, const unsigned int maxSeparationU, const unsigned short * const __restrict buffercopy, const unsigned short * const __restrict bufferzcopy, unsigned short * const __restrict bufferfinal, const unsigned int samples[3], const __m128& zmask128, const __m128& ZPDU128, const __m128& maxSepShl4128, const bool handle_borders, const unsigned int AA, unsigned char* __restrict const mask)
{
const __m128i width128 = (__m128i&)_mm_set1_ps((float&)width);

const int incr = (AA^1)+1;

#pragma omp parallel for schedule(dynamic) //!! make configurable for update/non-update version
for(int yi = ystart; yi < yend; yi+=incr)  //!! or interleave left/right and calcs instead? (might be faster, too, due to smaller register usage?)
{
const unsigned int y = yi;
const unsigned int offshalf1 = (y>>1)*width;
const unsigned int offshalf0 = (height>>1)*width + offshalf1;
const unsigned int ymms = y*width - maxSeparationU;
const unsigned int ypms = y*width + maxSeparationU;

const __m128i ymms128 = (__m128i&)_mm_set1_ps((float&)ymms);
const __m128i ypms128 = (__m128i&)_mm_set1_ps((float&)ypms);

const unsigned short* __restrict z = bufferzcopy + (y*width + xstart);
__m128i x128 = _mm_add_epi32(t0123,_mm_set1_epi32(xstart));

int x;
if(handle_borders) {
    for(x = 0; x < xstart; ++x) //!! black out border pixels, replicate borders for one half instead??
	    bufferfinal[offshalf0 + x] = bufferfinal[offshalf1 + x] = 0;
} else
    x = xstart;

unsigned int xm = y*(width>>2) + (xstart>>2);
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
const __m128 right0 = unpack_565(_mm_set_epi32(buffercopy[separationR3]        ,buffercopy[separationR2]        ,buffercopy[separationR1]        ,buffercopy[separationR0]));
const __m128 right1 = unpack_565(_mm_set_epi32(buffercopy[separationR3 - 1]    ,buffercopy[separationR2 - 1]    ,buffercopy[separationR1 - 1]    ,buffercopy[separationR0 - 1]));

const unsigned int separationL0 = ((unsigned int*)&pL)[0];
const unsigned int separationL1 = ((unsigned int*)&pL)[1];
const unsigned int separationL2 = ((unsigned int*)&pL)[2];
const unsigned int separationL3 = ((unsigned int*)&pL)[3];

const __m128 left0 = unpack_565(_mm_set_epi32(buffercopy[separationL3]        ,buffercopy[separationL2]        ,buffercopy[separationL1]        ,buffercopy[separationL0]));
const __m128 left1 = unpack_565(_mm_set_epi32(buffercopy[separationL3 + 1]    ,buffercopy[separationL2 + 1]    ,buffercopy[separationL1 + 1]    ,buffercopy[separationL0 + 1]));

const __m128i right00 = _mm_mul_int(_mm_and_ps(right0,FF00FF128),pRs4_1);
const __m128i right01 = _mm_mul_int(_mm_and_ps(right0,FF00128),  pRs4_1);
const __m128i right10 = _mm_mul_int(_mm_and_ps(right1,FF00FF128),pRs4_2);
const __m128i right11 = _mm_mul_int(_mm_and_ps(right1,FF00128),  pRs4_2);

const __m128i right = _mm_srli_epi32((__m128i&)_mm_or_ps(_mm_and_ps((__m128&)_mm_add_epi32(right00,right10),FF00FF00128),_mm_and_ps((__m128&)_mm_add_epi32(right01,right11),FF0000128)),8);

if(AA & y) // so always triggered if AA and y&1
	_mm_storel_pi((__m64*)(bufferfinal+offshalf0+x), pack_565(_mm_srli_epi32(_mm_add_epi32((__m128i&)_mm_and_ps(unpack_565(_mm_set_epi32(bufferfinal[offshalf0+x+3],bufferfinal[offshalf0+x+2],bufferfinal[offshalf0+x+1],bufferfinal[offshalf0+x])),FEFEFE128), (__m128i&)_mm_and_ps((__m128&)right,FEFEFE128)),1))); //!! opt.: this is very very crude code
else
	_mm_storel_pi((__m64*)(bufferfinal+offshalf0+x), pack_565(right));

const __m128i left00 = _mm_mul_int(_mm_and_ps(left0,FF00FF128),pLs4_1);
const __m128i left01 = _mm_mul_int(_mm_and_ps(left0,FF00128),  pLs4_1);
const __m128i left10 = _mm_mul_int(_mm_and_ps(left1,FF00FF128),pLs4_2);
const __m128i left11 = _mm_mul_int(_mm_and_ps(left1,FF00128),  pLs4_2);

const __m128i left = _mm_srli_epi32((__m128i&)_mm_or_ps(_mm_and_ps((__m128&)_mm_add_epi32(left00,left10),FF00FF00128),_mm_and_ps((__m128&)_mm_add_epi32(left01,left11),FF0000128)),8);

if(AA & y) // so always triggered if AA and y&1
	_mm_storel_pi((__m64*)(bufferfinal+offshalf1+x), pack_565(_mm_srli_epi32(_mm_add_epi32((__m128i&)_mm_and_ps(unpack_565(_mm_set_epi32(bufferfinal[offshalf1+x+3],bufferfinal[offshalf1+x+2],bufferfinal[offshalf1+x+1],bufferfinal[offshalf1+x])),FEFEFE128), (__m128i&)_mm_and_ps((__m128&)left,FEFEFE128)),1))); //!! opt.: this is very very crude code
else
	_mm_storel_pi((__m64*)(bufferfinal+offshalf1+x), pack_565(left));
}

if(handle_borders)
    for(; x < (int)width; ++x) //!! black out border pixels, replicate borders for one half instead??
	    bufferfinal[offshalf0 + x] = bufferfinal[offshalf1 + x] = 0;
}
}

inline void stereo_repro_32bit_x(const int ystart, const int yend, const int xstart, const int xend, const unsigned int width, const unsigned int height, const unsigned int maxSeparationU, const unsigned int   * const __restrict buffercopy, const unsigned int   * const __restrict bufferzcopy, unsigned int   * const __restrict bufferfinal, const unsigned int samples[3], const __m128& zmask128, const __m128& ZPDU128, const __m128& maxSepShl4128, const bool handle_borders, const unsigned int AA, unsigned char* __restrict const mask)
{
const __m128i width128 = (__m128i&)_mm_set1_ps((float&)width);

const int incr = (AA^1)+1;

#pragma omp parallel for schedule(dynamic) //!! make configurable for update/non-update version
for(int yi = ystart; yi < yend; yi+=incr)  //!! interleave left/right and calcs instead? (might be faster, too, due to smaller register usage?)
{
const unsigned int y = yi;
const unsigned int offshalf1 = (y>>1)*width;
const unsigned int offshalf0 = (height>>1)*width + offshalf1;
const unsigned int ymms = y*width - maxSeparationU;
const unsigned int ypms = y*width + maxSeparationU;

const __m128i ymms128 = (__m128i&)_mm_set1_ps((float&)ymms);
const __m128i ypms128 = (__m128i&)_mm_set1_ps((float&)ypms);

const float * __restrict z = (float*)bufferzcopy + (y*width + xstart);
__m128i x128 = _mm_add_epi32(t0123,_mm_set1_epi32(xstart));

unsigned int xm = y*(width>>2) + (xstart>>2);
int x;
if(handle_borders) {
    for(x = 0; x < xstart; ++x) //!! black out border pixels, replicate borders for one half instead??
	    bufferfinal[offshalf0 + x] = bufferfinal[offshalf1 + x] = 0;
} else
    x = xstart;

for(; x < xend; x+=4,z+=4,++xm,x128=_mm_add_epi32(x128,t4444)) if(mask[xm] == 0)
{
mask[xm] = 1;

const __m128 minDepthR = _mm_min_ps(_mm_min_ps(_mm_cvtepi32_ps((__m128i&)_mm_and_ps(_mm_loadu_ps(z-samples[0]),zmask128)),_mm_cvtepi32_ps((__m128i&)_mm_and_ps(_mm_loadu_ps(z-samples[1]),zmask128))),_mm_cvtepi32_ps((__m128i&)_mm_and_ps(_mm_loadu_ps(z-samples[2]),zmask128)));
const __m128 minDepthL = _mm_min_ps(_mm_min_ps(_mm_cvtepi32_ps((__m128i&)_mm_and_ps(_mm_loadu_ps(z+samples[0]),zmask128)),_mm_cvtepi32_ps((__m128i&)_mm_and_ps(_mm_loadu_ps(z+samples[1]),zmask128))),_mm_cvtepi32_ps((__m128i&)_mm_and_ps(_mm_loadu_ps(z+samples[2]),zmask128)));

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
const __m128 right0 =   (__m128&)_mm_set_epi32(buffercopy[separationR3]        ,buffercopy[separationR2]        ,buffercopy[separationR1]        ,buffercopy[separationR0]);
const __m128 right1 =   (__m128&)_mm_set_epi32(buffercopy[separationR3 - 1]    ,buffercopy[separationR2 - 1]    ,buffercopy[separationR1 - 1]    ,buffercopy[separationR0 - 1]);

const unsigned int separationL0 = ((unsigned int*)&pL)[0];
const unsigned int separationL1 = ((unsigned int*)&pL)[1];
const unsigned int separationL2 = ((unsigned int*)&pL)[2];
const unsigned int separationL3 = ((unsigned int*)&pL)[3];

const __m128 left0 =   (__m128&)_mm_set_epi32(buffercopy[separationL3]        ,buffercopy[separationL2]        ,buffercopy[separationL1]        ,buffercopy[separationL0]);
const __m128 left1 =   (__m128&)_mm_set_epi32(buffercopy[separationL3 + 1]    ,buffercopy[separationL2 + 1]    ,buffercopy[separationL1 + 1]    ,buffercopy[separationL0 + 1]);

const __m128i right00 = _mm_mul_int(_mm_and_ps(right0,FF00FF128),pRs4_1);
const __m128i right01 = _mm_mul_int(_mm_and_ps(right0,FF00128),  pRs4_1);
const __m128i right10 = _mm_mul_int(_mm_and_ps(right1,FF00FF128),pRs4_2);
const __m128i right11 = _mm_mul_int(_mm_and_ps(right1,FF00128),  pRs4_2);

const __m128i right = _mm_srli_epi32((__m128i&)_mm_or_ps(_mm_and_ps((__m128&)_mm_add_epi32(right00,right10),FF00FF00128),_mm_and_ps((__m128&)_mm_add_epi32(right01,right11),FF0000128)),8);

if(AA & y) // so always triggered if AA and y&1
	_mm_stream_si128((__m128i*)(bufferfinal+offshalf0+x),_mm_srli_epi32(_mm_add_epi32((__m128i&)_mm_and_ps(_mm_load_ps((float*)bufferfinal+offshalf0+x),FEFEFE128), (__m128i&)_mm_and_ps((__m128&)right,FEFEFE128)),1));
else
	_mm_stream_si128((__m128i*)(bufferfinal+offshalf0+x), right);

const __m128i left00 = _mm_mul_int(_mm_and_ps(left0,FF00FF128),pLs4_1);
const __m128i left01 = _mm_mul_int(_mm_and_ps(left0,FF00128),  pLs4_1);
const __m128i left10 = _mm_mul_int(_mm_and_ps(left1,FF00FF128),pLs4_2);
const __m128i left11 = _mm_mul_int(_mm_and_ps(left1,FF00128),  pLs4_2);

const __m128i left = _mm_srli_epi32((__m128i&)_mm_or_ps(_mm_and_ps((__m128&)_mm_add_epi32(left00,left10),FF00FF00128),_mm_and_ps((__m128&)_mm_add_epi32(left01,left11),FF0000128)),8);

if(AA & y) // so always triggered if AA and y&1
	_mm_stream_si128((__m128i*)(bufferfinal+offshalf1+x),_mm_srli_epi32(_mm_add_epi32((__m128i&)_mm_and_ps(_mm_load_ps((float*)bufferfinal+offshalf1+x),FEFEFE128), (__m128i&)_mm_and_ps((__m128&)left,FEFEFE128)),1));
else
	_mm_stream_si128((__m128i*)(bufferfinal+offshalf1+x), left);
}

if(handle_borders)
    for(; x < (int)width; ++x) //!! black out border pixels, replicate borders for one half instead??
	    bufferfinal[offshalf0 + x] = bufferfinal[offshalf1 + x] = 0;
}
}

#ifdef FXAA
__forceinline unsigned int luma(const unsigned int rgb)
{
return ((rgb&0xFFu)>>3)+((rgb&0xFF00u)>>9)+((rgb&0xFF0000u)>>18); //!! R,B swapped
}

__forceinline unsigned int lumas2(const unsigned int rgb)
{
return ((rgb&(0xFFu*4))>>5)+((rgb&(0xFF00u*4))>>11)+((rgb&(0xFF0000u*4))>>20); //!! R,B swapped
}

inline unsigned int bilerpFE(const unsigned int* const __restrict pic, const int xorg, const int yorg, int dx, int dy, const unsigned int nPitch, const unsigned int xres, const unsigned int yres)
{
const int dx4 = dx>>4;
const int dy4 = dy>>4;
dx &= 0xFu;
dy &= 0xFu;
const unsigned int xy  = dx*dy;
const unsigned int x16 = dx<<4;
const unsigned int y16 = dy<<4;
const unsigned int invxy = y16-xy;
const unsigned int xinvy = x16-xy;
const unsigned int invxinvy = 256-x16-invxy;

const int xa = xorg+dx4;
const int ya = yorg+dy4;
const int xb = xorg-dx4;
const int yb = yorg-dy4;

/*const unsigned int clampxa   = min((unsigned int) xa   ,xres-1);
const unsigned int clampxp1a = min((unsigned int)(xa+1),xres-1);
const unsigned int clampya   = min((unsigned int) ya   ,yres-1)*nPitch;
const unsigned int clampyp1a = min((unsigned int)(ya+1),yres-1)*nPitch;

const unsigned int clampxb   = min((unsigned int) xb   ,xres-1);
const unsigned int clampxp1b = min((unsigned int)(xb-1),xres-1);
const unsigned int clampyb   = min((unsigned int) yb   ,yres-1)*nPitch;
const unsigned int clampyp1b = min((unsigned int)(yb-1),yres-1)*nPitch;

const unsigned int r00a = pic[clampxa   + clampya];
const unsigned int r10a = pic[clampxp1a + clampya];
const unsigned int r01a = pic[clampxa   + clampyp1a];
const unsigned int r11a = pic[clampxp1a + clampyp1a];

const unsigned int r10b = pic[clampxp1b + clampyb];
const unsigned int r00b = pic[clampxb   + clampyb];
const unsigned int r11b = pic[clampxp1b + clampyp1b];
const unsigned int r01b = pic[clampxb   + clampyp1b];*/

const unsigned int offsa = xa + ya*nPitch;
const unsigned int r00a = pic[offsa];
const unsigned int r10a = pic[offsa+1];
const unsigned int r01a = pic[offsa+nPitch];
const unsigned int r11a = pic[offsa+1+nPitch];

const unsigned int offsb = xb + yb*nPitch;
const unsigned int r11b = pic[offsb-1-nPitch];
const unsigned int r01b = pic[offsb-nPitch];
const unsigned int r10b = pic[offsb-1];
const unsigned int r00b = pic[offsb];

return (((((r00a&0xFF00FFu)*invxinvy + (r10a&0xFF00FFu)*xinvy + (r01a&0xFF00FFu)*invxy + (r11a&0xFF00FFu)*xy) &0xFE00FE00u)
	|
	 (((r00a&0x00FF00u)*invxinvy + (r10a&0x00FF00u)*xinvy + (r01a&0x00FF00u)*invxy + (r11a&0x00FF00u)*xy) &0x00FE0000u))>>9) +
       (((((r00b&0xFF00FFu)*invxinvy + (r10b&0xFF00FFu)*xinvy + (r01b&0xFF00FFu)*invxy + (r11b&0xFF00FFu)*xy) &0xFE00FE00u)
	|
	 (((r00b&0x00FF00u)*invxinvy + (r10b&0x00FF00u)*xinvy + (r01b&0x00FF00u)*invxy + (r11b&0x00FF00u)*xy) &0x00FE0000u))>>9);
}
#endif
