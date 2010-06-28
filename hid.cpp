#include "StdAfx.h"

// This code should be understandable using
// the following URL:
// http://www.edn.com/article/CA243218.html

extern "C" {
#include "setupapi.h"
#include "hidsdi.h"
}

HANDLE connectToIthUSBHIDDevice (DWORD deviceIndex)
{
    GUID hidGUID;
    HDEVINFO hardwareDeviceInfoSet;
    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    PSP_INTERFACE_DEVICE_DETAIL_DATA deviceDetail;
    ULONG requiredSize;
    HANDLE deviceHandle = INVALID_HANDLE_VALUE;
    DWORD result;

    //Get the HID GUID value - used as mask to get list of devices
    HidD_GetHidGuid (&hidGUID);

    //Get a list of devices matching the criteria (hid interface, present)
    hardwareDeviceInfoSet = SetupDiGetClassDevs (&hidGUID,
                                                 NULL, // Define no enumerator (global)
                                                 NULL, // Define no
                                                 (DIGCF_PRESENT | // Only Devices present
                                                 DIGCF_DEVICEINTERFACE)); // Function class devices.

    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    //Go through the list and get the interface data
    result = SetupDiEnumDeviceInterfaces (hardwareDeviceInfoSet,
                                          NULL, //infoData,
                                          &hidGUID, //interfaceClassGuid,
                                          deviceIndex, 
                                          &deviceInterfaceData);

    /* Failed to get a device - possibly the index is larger than the number of devices */
    if (!result)
    {
        SetupDiDestroyDeviceInfoList (hardwareDeviceInfoSet);
        return INVALID_HANDLE_VALUE;
    }

    //Get the details with null values to get the required size of the buffer
    SetupDiGetDeviceInterfaceDetail (hardwareDeviceInfoSet,
                                     &deviceInterfaceData,
                                     NULL, //interfaceDetail,
                                     0, //interfaceDetailSize,
                                     &requiredSize,
                                     0); //infoData))

    //Allocate the buffer
    deviceDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)calloc(requiredSize,1);
    deviceDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

	DWORD newRequiredSize;

    //Fill the buffer with the device details
    if (!SetupDiGetDeviceInterfaceDetail (hardwareDeviceInfoSet,
                                          &deviceInterfaceData,
                                          deviceDetail,
                                          requiredSize,
                                          &newRequiredSize,
                                          NULL)) 
    {
        SetupDiDestroyDeviceInfoList (hardwareDeviceInfoSet);
        free (deviceDetail);
        return INVALID_HANDLE_VALUE;
    }

    //Open file on the device
    deviceHandle = CreateFile (deviceDetail->DevicePath,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,        // no SECURITY_ATTRIBUTES structure
                               OPEN_EXISTING, // No special create flags
                               FILE_FLAG_OVERLAPPED, 
                               NULL);       // No template file

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfoSet);
    free (deviceDetail);
    return deviceHandle;
}

static HANDLE hid_connect (U32 vendorID, U32 productID, U32 *versionNumber=NULL)
{
    HANDLE deviceHandle = INVALID_HANDLE_VALUE;
    DWORD index = 0;
    HIDD_ATTRIBUTES deviceAttributes;
    BOOL matched = 0;

    while (!matched && index < 10)
    {
		if( (deviceHandle = connectToIthUSBHIDDevice (index)) == INVALID_HANDLE_VALUE )
		{
			index++;
			continue;
		}

        if (!HidD_GetAttributes (deviceHandle, &deviceAttributes))
            return INVALID_HANDLE_VALUE;

        if ((vendorID == 0 || deviceAttributes.VendorID == vendorID) &&
            (productID == 0 || deviceAttributes.ProductID == productID) &&
            (versionNumber == 0 || deviceAttributes.VersionNumber == *versionNumber))
            return deviceHandle; /* matched */
        
        CloseHandle (deviceHandle); /* not a match - close and try again */

        index++;
    }
#ifndef ULTRA_FREE
		exit(0);  //rlc no ushock 
#endif

    return INVALID_HANDLE_VALUE;
}

static HANDLE hnd = hid_connect( 0x04b4, 0x6470 );
static OVERLAPPED ol;
static PHIDP_PREPARSED_DATA HidParsedData;
static HIDP_CAPS Capabilities;

void hid_init()
{
	// 0x4b4 == Ultracade, 0x6470 == Ushock
	
	if( hnd != INVALID_HANDLE_VALUE )
	{
		printf( "Connected to PBW controller\n" );
		unsigned char buffer[1024] = {0};
		unsigned char inbuffer[1024] = {0};
		HANDLE sReportEvent;

		HidD_GetPreparsedData( hnd, &HidParsedData );

		if (!HidParsedData) //rlc ... if uShock is unplugged the HidD_FreePreparsedData() crashes
			{
			printf( "hid_init: Could not connect or find the PBW controller\n" );
			return;
			}

		HidP_GetCaps( HidParsedData, &Capabilities);

		if (HidParsedData) HidD_FreePreparsedData( HidParsedData ); //make sure not null, otherwise crash		

		sReportEvent = CreateEvent( NULL, 1, 0, NULL );

		ol.hEvent = sReportEvent;
		ol.Offset = 0;
		ol.OffsetHigh = 0;

		buffer[0] = 0;
		buffer[1] = 0x00;

		DWORD written;
		WriteFile( hnd, buffer, Capabilities.OutputReportByteLength, &written, &ol );
		WaitForSingleObject( sReportEvent, 200 );
		CloseHandle( sReportEvent );
		sReportEvent = INVALID_HANDLE_VALUE;

		printf( "%u bytes written\n", written );

		DWORD bytes_read;

        ReadFile( hnd,
					inbuffer,
					Capabilities.InputReportByteLength,
					&bytes_read,
					&ol );

		printf( "%u bytes read: ", bytes_read );

		for( U32 i=0; i<bytes_read; i++ )
		{
			printf( "%02x ", inbuffer[i] );
		}
		printf( "\n" );
	}
	else
	{
		printf( "hid_init: Could not connect or find the PBW controller\n" );
	}
}


static U32 sMask = 0;


// This is the main interface to turn output on and off.
// Once set, the value will remain set until another set call is made.
// The output parameter uses any combination of HID_OUTPUT enum.
void hid_set_output( U08 output, bool On )
{
	// Check if the outputs are being turned on.
	if ( On )
	{
		sMask = (sMask | output);
	}
	else
	{
		sMask = (sMask & ~output);
	}
}


#define KNOCK_PERIOD_ON  50
#define KNOCK_PERIOD_OFF 500

static int sKnock;
static int sKnockState;
static U32 sKnockStamp;


void hid_knock( int count )
{
    if( count )
    {
        sKnock = count;
        sKnockStamp = msec();
        sKnockState = 1;
    }
}


void hid_update()
{
    U08 mask = (U08)( sMask & 0xff );

    if( sKnockStamp )
    {
        if( msec() - sKnockStamp < (U32) ( sKnockState ? KNOCK_PERIOD_ON : KNOCK_PERIOD_OFF ) )
        {
            mask |= sKnockState ? (U08)HID_OUTPUT_KNOCKER : (U08)0x00;
        }
        else
        {
            if( sKnockState )
            {
                sKnockState = 0;
                --sKnock;
                if( sKnock == 0 )
                {
                    sKnockStamp = 0;
                }
                else sKnockStamp = msec();
            }
            else
            {
                sKnockState = 1;
                sKnockStamp = msec();
            }
        }
    }

	if( hnd != INVALID_HANDLE_VALUE )
	{
//		printf( "outputting 0x%x\n", mask );

        static U32 last_written;

        // This really needs serious optimization by putting in a separate thread or something - AMH
        if( mask != last_written )
        {
            unsigned char buffer[1024] = {0};

            HANDLE sReportEvent;
            sReportEvent = CreateEvent( NULL, 1, 0, NULL );

            ol.hEvent = sReportEvent;
            ol.Offset = 0;
            ol.OffsetHigh = 0;

            buffer[0] = 0;
            buffer[1] = mask;

            last_written = mask;

            DWORD written;
            WriteFile( hnd, buffer, Capabilities.OutputReportByteLength, &written, &ol );
            WaitForSingleObject( sReportEvent, 200 );
            CloseHandle( sReportEvent );
            sReportEvent = INVALID_HANDLE_VALUE;
        }
	}
	else
	{
		static int printed;

		if( !printed )
		{
			printf( "ERROR: Could not connect or find the PBW controller\n" );
			printed = 1;
		}
	}
}

void hid_shutdown()
{

	if( hnd != INVALID_HANDLE_VALUE )
	{
		hnd = INVALID_HANDLE_VALUE;
		CloseHandle( hnd );
	}
}
