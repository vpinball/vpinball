#include "core/stdafx.h"

#include "lzwreader.h"

/* DECODE.C - LZW decoder
 * Copyright (C) 1987, by Steven A. Bennett
 *
 * Permission is given by the author to freely redistribute and include
 * this code in any program as long as this credit is given where due.
 *
 * In accordance with the above, I want to credit Steve Wilhite who wrote
 * the code which this is heavily inspired by...
 *
 * Release Notes: This file contains a LZW decoder routine
 * which is similar, structurally, to the original routine by Steve Wilhite.
 * It is, however, somewhat noticably faster in most cases.
 */

// Returns the next code from the file.
unsigned int LZWReader::get_next_code()
{
   if (nbits_left == 0)
   {
      if (navail_bytes <= 0)
      {
         // Out of bytes in current block, so read next block
         pbytes = byte_buff;
         navail_bytes = get_byte();
         for (int i = 0; i < navail_bytes; ++i)
            byte_buff[i] = get_byte();
      }
      b1 = *pbytes++;
      nbits_left = 8;
      --navail_bytes;
   }

   unsigned int ret = b1 >> (8 - nbits_left);
   while (curr_size > nbits_left)
   {
      if (navail_bytes <= 0)
      {
         // Out of bytes in current block, so read next block
         pbytes = byte_buff;
         navail_bytes = get_byte();
         for (int i = 0; i < navail_bytes; ++i)
            byte_buff[i] = get_byte();
      }
      b1 = *pbytes++;
      ret |= b1 << nbits_left;
      nbits_left += 8;
      --navail_bytes;
   }
   nbits_left -= curr_size;

   static constexpr unsigned int code_mask[13] = { 0, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF, 0x0FFF };

   return (ret & code_mask[curr_size]);
}


/* This function decodes an LZW image, according to the method used
 * in the GIF spec.  Every *width* characters/pixels decoded
 * will move the output one line ahead.  The function gets it's codes from
 * get_next_code() which is responsible for reading blocks of data and
 * separating them into the proper size codes.  Finally, get_byte() is
 * the global routine to read the next byte from the file.
 */
LZWReader::LZWReader(IStream *const pstm, uint8_t *output, const unsigned int width)
   : m_pstm(pstm)
#ifdef _DEBUG
   , bad_code_count(0)
#endif
   , m_cfilebuffer(FILE_BUF_SIZE - 1)
   , m_readahead(FILE_BUF_SIZE)
{
   // Initialize for decoding a new image...
   constexpr unsigned int size = 8;

   curr_size = size + 1;
   unsigned int top_slot = 1u << curr_size; // Highest code for current size
   constexpr unsigned int clear = 1u << size; // Value for a clear code
   constexpr unsigned int ending = clear + 1; // Value for an ending code
   constexpr unsigned int newcodes = ending + 1; // First available code
   unsigned int slot = newcodes; // Last read code
   navail_bytes = 0;
   nbits_left = 0;

   /* Initialize in case they forgot to put in a clear code.
    * (This shouldn't happen, but we'll try and decode it anyway...)
    */
   unsigned int oc = 0;
   unsigned int fc = 0;

   // Set up the decoding buffer from the output pointer
   uint8_t *buf = output;
   output += width;

   unsigned int bufcnt = width;

   // Set up the stack pointer
   uint8_t *sp = stack;

   /* This is the main loop.  For each code we get, we pass through the
    * linked list of prefix codes, pushing the corresponding "character" for
    * each code onto the stack.  When the list reaches a single "character"
    * we push that on the stack too, and then start unstacking each
    * character for output in the correct order.  Special handling is
    * included for the clear code, and the whole thing ends when we get
    * an ending code.
    */
   unsigned int c;
   while ((c = get_next_code()) != ending)
   {
      // If the code is a clear code, reinitialize all necessary items.
      if (c == clear)
      {
         curr_size = size + 1;
         slot = newcodes;
         top_slot = 1u << curr_size;

         /* Continue reading codes until we get a non-clear code
          * (Another unlikely, but possible case...)
          */
         while ((c = get_next_code()) == clear)
            ;

         /* If we get an ending code immediately after a clear code
          * (Yet another unlikely case), then break out of the loop.
          */
         if (c == ending)
            break;

         /* Finally, if the code is beyond the range of already set codes,
          * (This one had better NOT happen...  I have no idea what will
          * result from this, but I doubt it will look good...) then set it
          * to color zero.
          */
         if (c >= slot)
            c = 0;

         oc = fc = c;

         /* And let us not forget to put the char into the buffer... And
          * if, on the off chance, we were exactly one pixel from the end
          * of the line, we have to move the buffer forward
          */
         *buf++ = (uint8_t)c;

         if (--bufcnt == 0)
         {
            buf = output;
            output += width;
            bufcnt = width;
         }
      }
      else
      {
         /* In this case, it's not a clear code or an ending code, so
          * it must be a code code...  So we can now decode the code into
          * a stack of character codes. (Clear as mud, right?)
          */
         unsigned int code = c;

         /* Here we go again with one of those off chances...  If, on the
          * off chance, the code we got is beyond the range of those already
          * set up (Another thing which had better NOT happen...) we trick
          * the decoder into thinking it actually got the last code read.
          * (Hmmn... I'm not sure why this works...  But it does...)
          */
         if (code >= slot)
         {
#ifdef _DEBUG
            if (code > slot)
               ++bad_code_count;
#endif
            code = oc;
            *sp++ = (uint8_t)fc;
         }

         /* Here we scan back along the linked list of prefixes, pushing
          * helpless characters (i.e. suffixes) onto the stack as we do so.
          */
         while (code >= newcodes)
         {
            *sp++ = suffix[code];
            code = prefix[code];
         }

         /* Push the last character on the stack, and set up the new
          * prefix and suffix, and if the required slot number is greater
          * than that allowed by the current bit size, increase the bit
          * size.  (NOTE - If we are all full, we *don't* save the new
          * suffix and prefix...  I'm not certain if this is correct...
          * it might be more proper to overwrite the last code...
          */
         *sp++ = (uint8_t)code;
         if (slot < top_slot)
         {
            fc = code;
            suffix[slot] = (uint8_t)fc;	// = code;
            prefix[slot++] = (uint16_t)oc;
            oc = c;
         }
         if (slot >= top_slot)
            if (curr_size < 12)
            {
               top_slot <<= 1;
               ++curr_size;
            }

         /* Now that we've pushed the decoded string (in reverse order)
          * onto the stack, lets pop it off and put it into our decode
          * buffer...  And when the decode buffer is full, write another
          * line...
          */
         while (sp > stack)
         {
            *buf++ = *(--sp);
            if (--bufcnt == 0)
            {
               buf = output;
               output += width;
               bufcnt = width;
            }
         }
      }
   }

   LONGLONG toofar = (LONGLONG)m_readahead - m_cfilebuffer; // bytes we already read from the stream, BUT that we shouldn't have read
   toofar--;  // m_readahead == the byte we just read, so we actually used up one more than the math shows
   LARGE_INTEGER li;
   li.QuadPart = -toofar;
   m_pstm->Seek(li, STREAM_SEEK_CUR, nullptr); // move back in the stream so next VPT data chunk reads will be correct again
}

// This returns the next byte from the file
uint8_t LZWReader::get_byte()
{
   ++m_cfilebuffer;
   if (m_cfilebuffer == FILE_BUF_SIZE)
   {
      m_cfilebuffer = 0;
      m_readahead = 0;
      m_pstm->Read(m_pfilebufferbytes, FILE_BUF_SIZE, &m_readahead);
   }

   return m_pfilebufferbytes[m_cfilebuffer];
}
