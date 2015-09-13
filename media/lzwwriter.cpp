#include "stdafx.h"

int LZWWriter::m_masks[17] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F,
0x001F, 0x003F, 0x007F, 0x00FF,
0x01FF, 0x03FF, 0x07FF, 0x0FFF,
0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

LZWWriter::LZWWriter(IStream * pistream, int *bits, int width, int height, int pitch)
{
   m_pistream = pistream;
   m_bits = bits;
   m_width = width; // 32-bit picture
   m_height = height;
   m_pitch = pitch;
}

LZWWriter::~LZWWriter()
{
}

HRESULT LZWWriter::WriteSz(char *sz, int cbytes)
{
   return m_pistream->Write(sz, cbytes, NULL);
}

HRESULT LZWWriter::WriteByte(char ch)
{
   return m_pistream->Write(&ch, 1, NULL);
}

HRESULT LZWWriter::WriteWord(short word)
{
   return m_pistream->Write(&word, 2, NULL);
}

int LZWWriter::bNextPixel()
{
   if (m_iPixelCur == m_pitch*m_height)
      return GIFEOF;

   const int ch = *(((unsigned char *)m_bits) + m_iPixelCur);
   ++m_iPixelCur;
   ++m_iXCur;
   if (m_iXCur == m_width)
   {
      m_iPixelCur += (m_pitch - m_width);
      m_iXCur = 0;
   }

   return ch;
}

HRESULT LZWWriter::CompressBits(int init_bits)
{
   int c;
   int ent;
   int disp;
   int hsize_reg;
   int hshift;

   // Used to be in write gif
   m_iPixelCur = 0;
   m_iXCur = 0;

   m_clear_flg = false;

   m_cur_accum = 0;
   m_cur_bits = 0;

   m_a_count = 0;
   ////////////////////////////

   // Set up the globals:  g_init_bits - initial number of bits
   // bits per pixel
   m_init_bits = init_bits;

   // Set up the necessary values
   m_nbits = m_init_bits;
   m_maxcode = Maxcode(m_nbits);

   m_ClearCode = 1 << (init_bits - 1);
   m_EOFCode = m_ClearCode + 1;
   m_free_ent = m_ClearCode + 2;

   ent = bNextPixel();

   hshift = 0;
   for (int fcode = HSIZE; fcode < 65536; fcode *= 2)
      ++hshift;
   hshift = 8 - hshift;	// set hash code range bound

   hsize_reg = HSIZE;
   ClearHash(hsize_reg);	// clear hash table

   Output(m_ClearCode);

   while ((c = bNextPixel()) != GIFEOF)
   {
      const int fcode = (c << MAXBITS) + ent;
      int i = (c << hshift) ^ ent;		// xor hashing

      if (m_codetab[i] != 0)
      {	/* is first probed slot empty? */
         if (m_htab[i] == fcode)
         {
            ent = m_codetab[i];
            goto nextbyte;
         }
         if (i == 0)			/* secondary hash (after G. Knott) */
            disp = 1;
         else
            disp = HSIZE - i;
         while (1)
         {
            i -= disp;
            if (i < 0)
               i += HSIZE;
            if (m_codetab[i] == 0)
               goto processbyte;			/* hit empty slot */
            if (m_htab[i] == fcode)
            {
               ent = m_codetab[i];
               goto nextbyte;
            }
         }
      }

   processbyte:

      Output(ent);
      ent = c;
      if (m_free_ent < MAXMAXCODE)
      {
         m_codetab[i] = m_free_ent++;	// code -> hashtable
         m_htab[i] = fcode;
      }
      else
         ClearBlock();

   nextbyte:

      ;

   }
   // Put out the final code.
   Output(ent);
   Output(m_EOFCode);

   return S_OK;
}

HRESULT LZWWriter::Output(int code)
{
   m_cur_accum &= m_masks[m_cur_bits];

   if (m_cur_bits > 0)
      m_cur_accum |= (code << m_cur_bits);
   else
      m_cur_accum = code;

   m_cur_bits += m_nbits;

   while (m_cur_bits >= 8)
   {
      CharOut((unsigned char)(m_cur_accum & 0xff));
      m_cur_accum >>= 8;
      m_cur_bits -= 8;
   }

   // If the next entry is going to be too big for the code size,
   // then increase it, if possible.
   if (m_free_ent > m_maxcode || m_clear_flg)
   {
      if (m_clear_flg)
      {
         m_maxcode = Maxcode(m_nbits = m_init_bits);
         m_clear_flg = false;
      }
      else
      {
         ++m_nbits;
         if (m_nbits == MAXBITS)
            m_maxcode = MAXMAXCODE;
         else
            m_maxcode = Maxcode(m_nbits);
      }
   }

   if (code == m_EOFCode)
   {
      // At EOF, write the rest of the buffer.
      while (m_cur_bits > 0)
      {
         CharOut((unsigned char)(m_cur_accum & 0xff));
         m_cur_accum >>= 8;
         m_cur_bits -= 8;
      }

      FlushChar();
   }
   return S_OK;
}

HRESULT LZWWriter::ClearBlock()
{
   ClearHash(HSIZE);
   m_free_ent = m_ClearCode + 2;
   m_clear_flg = true;

   Output(m_ClearCode);
   return S_OK;
}

void LZWWriter::ClearHash(int hsize)
{
   for (int i = 0; i < HSIZE; ++i)
   {
      m_htab[i] = -1;
      m_codetab[i] = 0;
   }
}

HRESULT LZWWriter::CharOut(char c)
{
   m_accum[m_a_count++] = c;
   if (m_a_count >= 254)
      FlushChar();

   return S_OK;
}

HRESULT LZWWriter::FlushChar()
{
   if (m_a_count > 0)
   {
      WriteByte((char)m_a_count);
      WriteSz((char *)m_accum, m_a_count);
      m_a_count = 0;
   }
   return S_OK;
}
