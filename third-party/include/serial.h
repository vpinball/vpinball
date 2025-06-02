#pragma once

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

enum serial_parity
{
	SERIAL_PARITY_NONE = 'N',
	SERIAL_PARITY_EVEN = 'E',
	SERIAL_PARITY_ODD  = 'O'
};

enum serial_bits
{
	SERIAL_BITS_5 = 5,
	SERIAL_BITS_6 = 6,
	SERIAL_BITS_7 = 7,
	SERIAL_BITS_8 = 8
};

enum serial_baud
{
	SERIAL_BAUD_1200 = 1200,
	SERIAL_BAUD_2400 = 2400,
	SERIAL_BAUD_4800 = 4800,
	SERIAL_BAUD_9600 = 9600,
	SERIAL_BAUD_19200 = 19200,
	SERIAL_BAUD_38400 = 38400,
	SERIAL_BAUD_57600 = 57600,
	SERIAL_BAUD_115200 = 115200,

	SERIAL_BAUD_INVALID
};

enum serial_stopbit
{
	SERIAL_STOPBIT_1 = 1,
	SERIAL_STOPBIT_2 = 2,
};

enum serial_err
{
	SERIAL_ERR_OK = 0,

	SERIAL_ERR_SYSTEM,
	SERIAL_ERR_UNKNOWN,
	SERIAL_ERR_INVALID_BAUD,
	SERIAL_ERR_INVALID_BITS,
	SERIAL_ERR_INVALID_PARITY,
	SERIAL_ERR_INVALID_STOPBIT,
	SERIAL_ERR_NODATA
};

class serial
{
public:
   serial() : fd(INVALID_HANDLE_VALUE), configured(false) {}

   bool open(const string &device);
   void close();
   void flush();
   serial_err setup(const serial_baud baud, const serial_bits bits, const serial_parity parity, const serial_stopbit stopbit);
   serial_err write(const vector<char>& buffer);
   serial_err read(vector<char>& buffer); // depends on the size of the incoming vector!
   const char *get_setup_str();
   serial_err set_rts(bool enable);
   serial_err set_dtr(bool enable);

   // helper functions
   static void list_ports(vector<string> &availablePorts);

   static serial_baud get_baud(const unsigned int baud);
   static unsigned int get_baud_int(const serial_baud baud);
   static serial_bits get_bits(const unsigned int bits);
   static unsigned int get_bits_int(const serial_bits bits);
   static serial_parity get_parity(const char parity);
   static char get_parity_str(const serial_parity parity);
   static serial_stopbit get_stopbit(const unsigned int stopbit);
   static unsigned int get_stopbit_int(const serial_stopbit stopbit);

   serial_baud baud;
   serial_bits bits;
   serial_parity parity;
   serial_stopbit stopbit;

private:
   HANDLE fd;
   DCB oldtio;
   DCB newtio;

   bool configured;
};
