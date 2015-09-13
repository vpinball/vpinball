#pragma once
#ifndef __LZWWRITER_H_
#define __LZWWRITER_H_

class LZWWriter
{
public:
   enum {
      BITS = 12,
      MAXBITS = BITS,
      MAXMAXCODE = 1 << BITS,
      HSIZE = 5003,		// 80% occupancy
      GIFEOF = -1,
      MAXLEAF = 4000,
      PALETTESIZE = 256
   };

   LZWWriter(IStream * pistream, int *bits, int width, int height, int pitch);

   ~LZWWriter();

   CComPtr<IStream> m_pistream;
   int *m_bits;
   int m_width, m_height;
   int m_pitch;	// x-length of each scanline (divisible by 8, normally)

   int m_nbits;
   int m_maxcode;

   int m_htab[HSIZE];
   int m_codetab[HSIZE];

   int m_free_ent;

   int m_init_bits;

   int m_ClearCode;
   int m_EOFCode;

   int m_cur_accum;
   int m_cur_bits;

   int m_a_count;

   unsigned char m_accum[256];

   int m_colortable[256];

   static int m_masks[17];

   HRESULT WriteSz(char *sz, int cbytes);
   HRESULT WriteByte(char ch);
   HRESULT WriteWord(short word);

   HRESULT WriteGif();

   int m_iPixelCur;
   int m_iXCur;

   int bNextPixel();

   bool m_clear_flg;

   inline int Maxcode(int n_bits)
   {
      return (1 << n_bits) - 1;
   }

   HRESULT CompressBits(int init_bits);
   HRESULT Output(int code);
   HRESULT ClearBlock();
   void ClearHash(int hsize);
   HRESULT CharOut(char c);
   HRESULT FlushChar();
};

#endif
