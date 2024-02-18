#include "core/stdafx.h"

#include <fcntl.h>
#include <cstdio>
#include <cassert>

#include "serial.h"

void serial::list_ports(vector<string> &availablePorts)
{
   availablePorts.clear();
   DWORD size = 512*512;
   do
   {
      vector<char> allDeviceNames(size);

      // on Windows the serial ports are devices that begin with "COM"
      const DWORD ret = QueryDosDevice(nullptr, allDeviceNames.data(), size);
      if (ret != 0)
      {
         DWORD c = 0;
         do
         {
            DWORD c2 = 0;
            string tmp;
            while (allDeviceNames[c + c2] != '\0')
            {
               tmp += allDeviceNames[c + c2];
               c2++;
            }
            if (!tmp.empty() && tmp.substr(0, 3) == "COM")
               availablePorts.push_back(tmp);
            c += c2 + 1;
         } while (c < ret && allDeviceNames[c] != '\0');

         return;
      }

      size *= 2;
   } while (size < 16777216);
}

serial_baud serial::get_baud(const unsigned int _baud) {
	switch(_baud) {
		case   1200:
		case   2400:
		case   4800:
		case   9600:
		case  19200:
		case  38400:
		case  57600:
		case 115200: return (serial_baud)_baud; break;
		default:     return SERIAL_BAUD_INVALID; break;
	}
}

unsigned int serial::get_baud_int(const serial_baud _baud) {
	switch(_baud) {
		case SERIAL_BAUD_1200  :
		case SERIAL_BAUD_2400  :
		case SERIAL_BAUD_4800  :
		case SERIAL_BAUD_9600  :
		case SERIAL_BAUD_19200 :
		case SERIAL_BAUD_38400 :
		case SERIAL_BAUD_57600 :
		case SERIAL_BAUD_115200: return (unsigned int)_baud; break;
		case SERIAL_BAUD_INVALID:
		default:                 return 0; break;
	}
}

serial_bits serial::get_bits(const unsigned int _bits) {
	switch(_bits) {
		case 5:
		case 6:
		case 7:
		case 8:  return (serial_bits)_bits; break;
		default: return SERIAL_BITS_8; break;
	}
}

unsigned int serial::get_bits_int(const serial_bits _bits) {
	switch(_bits) {
		case SERIAL_BITS_5: 
		case SERIAL_BITS_6: 
		case SERIAL_BITS_7: 
		case SERIAL_BITS_8: return (unsigned int)_bits; break;
		default:            return 0; break;
	}
}

serial_parity serial::get_parity(const char _parity) {
	switch(_parity) {
		case 'N':
		case 'E':
		case 'O': return (serial_parity)_parity; break;
		default:  return SERIAL_PARITY_NONE; break;
	}
}

char serial::get_parity_str(const serial_parity _parity) {
	switch(_parity) {
		case SERIAL_PARITY_NONE:
		case SERIAL_PARITY_EVEN:
		case SERIAL_PARITY_ODD : return (char)_parity; break;
		default:                 return ' '; break;
	}
}

serial_stopbit serial::get_stopbit(const unsigned int _stopbit) {
	switch(_stopbit) {
		case 1:
		case 2:  return (serial_stopbit)_stopbit; break;
		default: return SERIAL_STOPBIT_1; break;
	}
}

unsigned int serial::get_stopbit_int(const serial_stopbit _stopbit) {
	switch(_stopbit) {
		case SERIAL_STOPBIT_1:
		case SERIAL_STOPBIT_2: return (unsigned int)_stopbit; break;
		default:               return 0; break;
	}
}

bool serial::open(const string& device) 
{
	// Fix the device name if required
	string devName;
	if (device.length() > 4 && device[0] != '\\')
		devName = "\\\\.\\";
	devName += device;

	// Create file handle for port
	fd = CreateFile(devName.c_str(), GENERIC_READ | GENERIC_WRITE, 
			0, // Exclusive access
			nullptr, // No security
			OPEN_EXISTING,
			0, // FILE_FLAG_OVERLAPPED,
			NULL);

	if(fd == INVALID_HANDLE_VALUE) 
		return false;

	SetupComm(fd, 4096, 4096); // Set input and output buffer size

	//COMMTIMEOUTS timeouts = {MAXDWORD, MAXDWORD, 3000, 0, 0};
	//shorter timeout:
	COMMTIMEOUTS timeouts = { MAXDWORD, MAXDWORD, 500, 0, 0 };

	SetCommTimeouts(fd, &timeouts);

	SetCommMask(fd, EV_ERR); // Notify us of error events

	GetCommState(fd, &oldtio); // Retrieve port parameters
	GetCommState(fd, &newtio); // Retrieve port parameters

	PurgeComm(fd, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);

	return true;
}

void serial::close() 
{
	assert(fd != INVALID_HANDLE_VALUE);

	flush();
	SetCommState(fd, &oldtio);
	CloseHandle(fd);

	fd = INVALID_HANDLE_VALUE;
	configured = false;
}

void serial::flush() 
{
	assert(fd != INVALID_HANDLE_VALUE);

	// We shouldn't need to flush in non-overlapping (blocking) mode
	PurgeComm(fd, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
}

serial_err serial::setup(
			  const serial_baud _baud,
			  const serial_bits _bits,
			  const serial_parity _parity,
			  const serial_stopbit _stopbit)
{
	assert(fd != INVALID_HANDLE_VALUE);

	switch(_baud) {
		case SERIAL_BAUD_1200  : newtio.BaudRate = CBR_1200; break;
		case SERIAL_BAUD_2400  : newtio.BaudRate = CBR_2400; break;
		case SERIAL_BAUD_4800  : newtio.BaudRate = CBR_4800; break;
		case SERIAL_BAUD_9600  : newtio.BaudRate = CBR_9600; break;
		case SERIAL_BAUD_19200 : newtio.BaudRate = CBR_19200; break;
		case SERIAL_BAUD_38400 : newtio.BaudRate = CBR_38400; break;
		case SERIAL_BAUD_57600 : newtio.BaudRate = CBR_57600; break;
		case SERIAL_BAUD_115200: newtio.BaudRate = CBR_115200; break;
		case SERIAL_BAUD_INVALID:
		default:                 return SERIAL_ERR_INVALID_BAUD; break;
	}

	switch(_bits) {
		case SERIAL_BITS_5:
		case SERIAL_BITS_6:
		case SERIAL_BITS_7:
		case SERIAL_BITS_8: newtio.ByteSize = (BYTE)_bits; break;
		default:            return SERIAL_ERR_INVALID_BITS; break;
	}

	switch(_parity) {
		case SERIAL_PARITY_NONE: newtio.Parity = NOPARITY; break;
		case SERIAL_PARITY_EVEN: newtio.Parity = EVENPARITY; break;
		case SERIAL_PARITY_ODD : newtio.Parity = ODDPARITY; break;
		default:                 return SERIAL_ERR_INVALID_PARITY; break;
	}

	switch(_stopbit) {
		case SERIAL_STOPBIT_1: newtio.StopBits = ONESTOPBIT; break;
		case SERIAL_STOPBIT_2: newtio.StopBits = TWOSTOPBITS; break;
		default:               return SERIAL_ERR_INVALID_STOPBIT; break;
	}

	// if the port is already configured, no need to do anything
	if (
		configured         &&
		baud    == _baud   &&
		bits    == _bits   &&
		parity  == _parity &&
		stopbit == _stopbit
	) return SERIAL_ERR_OK;

	// reset the settings
	newtio.fOutxCtsFlow = FALSE;
	newtio.fOutxDsrFlow = FALSE;
	newtio.fOutX = FALSE;
	newtio.fInX = FALSE;
	newtio.fNull = 0;
	newtio.fAbortOnError = 0;

	// set the settings
	flush();
	if (!SetCommState(fd, &newtio))
		return SERIAL_ERR_SYSTEM;

	configured = true;
	baud       = _baud;
	bits       = _bits;
	parity     = _parity;
	stopbit    = _stopbit;
	return SERIAL_ERR_OK;
}

serial_err serial::write(const vector<char>& buffer) 
{
	assert((fd != INVALID_HANDLE_VALUE) && configured);

	const uint8_t *pos = (const uint8_t*)buffer.data();
	int len = (int)buffer.size();

	while(len > 0) {
		DWORD r;
		if (!WriteFile(fd, pos, len, &r, nullptr))
			return SERIAL_ERR_SYSTEM;
		if (r < 1) return SERIAL_ERR_SYSTEM;

		len -= r;
		pos += r;
	}

	return SERIAL_ERR_OK;
}

serial_err serial::read(vector<char>& buffer) // depends on the size of the incoming vector!
{
	assert((fd != INVALID_HANDLE_VALUE) && configured);

	uint8_t *pos = (uint8_t*)buffer.data();
	int len = (int)buffer.size();

	while(len > 0) {
		DWORD r;
		ReadFile(fd, pos, len, &r, nullptr);
		if (r == 0) return SERIAL_ERR_NODATA;

		len -= r;
		pos += r;
	}

	return SERIAL_ERR_OK;
}

const char* serial::get_setup_str() 
{
	static char str[11];
	if (!configured)
		snprintf(str, sizeof(str), "INVALID");
	else
		snprintf(str, sizeof(str), "%u %d%c%d",
			get_baud_int(baud),
			get_bits_int(bits),
			get_parity_str(parity),
			get_stopbit_int(stopbit)
		);

	return str;
}

serial_err serial::set_rts(bool enable)
{
   assert((fd != INVALID_HANDLE_VALUE) && configured);

   newtio.fRtsControl = enable ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE;

   flush();
   return !SetCommState(fd, &newtio) ? SERIAL_ERR_SYSTEM : SERIAL_ERR_OK;
}

serial_err serial::set_dtr(bool enable)
{
   assert((fd != INVALID_HANDLE_VALUE) && configured);

   newtio.fDtrControl = enable ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE;

   flush();
   return !SetCommState(fd, &newtio) ? SERIAL_ERR_SYSTEM : SERIAL_ERR_OK;
}
