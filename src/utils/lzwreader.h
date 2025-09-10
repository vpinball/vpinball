#pragma once

class LZWReader final
{
public:
   LZWReader(IStream * const pstm, uint8_t *output, const unsigned int width); // immediately decodes pstm content into output
   ~LZWReader() { }

private:
   static constexpr size_t MAX_CODES = 4095;

   static constexpr int FILE_BUF_SIZE = 4096;

   unsigned int get_next_code();
   uint8_t get_byte();

   IStream * const m_pstm;

#ifdef _DEBUG
   // This value is incremented each time an out of range code is read by the decoder. When this value is non - zero after a decode, your file is probably corrupt in some way
   int bad_code_count;
#endif

   unsigned int curr_size;        // The current code size

   // The following variables are used for separating out codes
   int navail_bytes;              // # bytes left in block
   unsigned int nbits_left;       // # bits left in current byte
   uint8_t b1;                    // Current byte
   uint8_t byte_buff[257];        // Current block
   uint8_t *pbytes;               // Pointer to next byte in block

   uint8_t stack[MAX_CODES + 1];  // Stack for storing pixels
   uint8_t suffix[MAX_CODES + 1]; // Suffix table
   uint16_t prefix[MAX_CODES + 1];// Prefix linked list

   // read file in chunks of FILE_BUF_SIZE
   int m_cfilebuffer;
   uint8_t m_pfilebufferbytes[FILE_BUF_SIZE];

   ULONG m_readahead;             // How many bytes we read into our buffer
};
