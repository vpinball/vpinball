#ifndef __GIFREADER_H_
#define __GIFREADER_H_

#define MAX_CODES   4095

#define FILE_BUF_SIZE 4096

class LZWReader	
	{
public:
	LZWReader(IStream *pstm, int *bits, int width, int height, int pitch);
	~LZWReader();

	//AniBitmap *LoadURL(WCHAR *szURL);
	//AniBitmap *Read(BYTE *pbGifBits, int cbGifBits);
	//AniBitmap *LoadResource(HINSTANCE hinst, int idb);

	//BYTE *m_pbGifBitsMac;
	//BYTE *m_pbGifBitsCur;

	IStream *m_pstm;

	//AniBitmap *m_panibitmap;
	//DIB256ColorTable m_clrtGlobal;
	int m_msDelayCur;
//	COlOR m_clrTransparent;
//	RGBQUAD m_rgbqTransparent;
	//int m_icvTransparent;

	/* output */
	BYTE *m_pbBitsOutCur;
	int m_cbStride;
	int bad_code_count;

	/* Static variables */
	int curr_size;                     /* The current code size */
	int clear;                         /* Value for a clear code */
	int ending;                        /* Value for a ending code */
	int newcodes;                      /* First available code */
	int top_slot;                      /* Highest code for current size */
	int slot;                          /* Last read code */
	
	/* The following static variables are used
	 * for seperating out codes
	 */
	int navail_bytes;              /* # bytes left in block */
	int nbits_left;                /* # bits left in current byte */
	BYTE b1;                           /* Current byte */
	BYTE byte_buff[257];               /* Current block */
	BYTE *pbytes;                      /* Pointer to next byte in block */
	
	BYTE stack[MAX_CODES + 1];            /* Stack for storing pixels */
	BYTE suffix[MAX_CODES + 1];           /* Suffix table */
	unsigned short prefix[MAX_CODES + 1];           /* Prefix linked list */


	int get_byte();
	//int fread(void *pv, int cb);
	//int skip(int cb);
	BYTE *NextLine();

	short init_exp(short size);
	int get_next_code();
	short Decoder();

	int m_cfilebuffer;
	BYTE m_pfilebufferbytes[FILE_BUF_SIZE];

	int m_width;
	int m_height;
	int m_linesleft;

	ULONG m_readahead; // How many bytes we read into our buffer

	//BOOL FReadClrt(int size, DIB256ColorTable *pclrt);

	//BOOL GIFimage();
	};
#endif

