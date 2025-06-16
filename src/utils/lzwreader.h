#pragma once

#define MAX_CODES   4095

#define FILE_BUF_SIZE 4096

class LZWReader final
{
public:
   LZWReader(IStream *pstm, int *bits, int width, int height, int pitch);
   ~LZWReader() { }

   short Decoder();

private:
   short init_exp(int size);
   int get_next_code();
   int get_byte();
   uint8_t *NextLine();

   IStream *m_pstm;

   /* output */
   uint8_t *m_pbBitsOutCur;
   int m_cbStride;
#ifdef _DEBUG
   int bad_code_count;
#endif

   /* Static variables */
   int curr_size;                 /* The current code size */
   int clear;                     /* Value for a clear code */
   int ending;                    /* Value for a ending code */
   int newcodes;                  /* First available code */
   int top_slot;                  /* Highest code for current size */
   int slot;                      /* Last read code */

   // The following static variables are used
   // for separating out codes
   int navail_bytes;              // # bytes left in block
   int nbits_left;                // # bits left in current byte
   uint8_t b1;                    // Current byte
   uint8_t byte_buff[257];        // Current block
   uint8_t *pbytes;               // Pointer to next byte in block

   uint8_t stack[MAX_CODES + 1];  // Stack for storing pixels
   uint8_t suffix[MAX_CODES + 1]; // Suffix table
   WCHAR prefix[MAX_CODES + 1];   // Prefix linked list

   int m_cfilebuffer;
   uint8_t m_pfilebufferbytes[FILE_BUF_SIZE];

   int m_width;
   int m_height;
   int m_linesleft;

   unsigned int m_readahead;      // How many bytes we read into our buffer
};
