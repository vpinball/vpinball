//DongleAPI.h
//Copyright (c) GlobalVr, Inc. 2006
#pragma once
//This header contains the prototype of the functions that DongleAPI.lib has.

//This function MUST be called before any other
//Returns true if a valid dongle has been detected.
bool DongleAPI_Init();

//Call this function when a windows WM_DEVICECHANGE message has been recieved
bool DongleAPI_DeviceChanged();

//The remaing functions should only be called AFTER DongleAPI_Init has been called.
//Returns true if a valid dongle is still inserted
bool DongleAPI_IsValid();
//Returns a pointer to a char string with the ASCII Region. NULL if a valid dongle is not present.
const char * DongleAPI_GetRegion();
//Returns a pointer to a char string with the ASCII Game Name. NULL if a valid dongle is not present.
const char * DongleAPI_GetGameName();
//Returns a pointer to a char string with the ASCII Version. NULL if a valid dongle is not present.
const char * DongleAPI_GetVersion();
//Returns a pointer to a char string with the ASCII Region Label. NULL if a valid dongle is not present.
const char * DongleAPI_GetRegionLabel();
//Returns a pointer to a char string with the ASCII Cab Label. NULL if a valid dongle is not present.
const char * DongleAPI_GetCabLabel();
//Returns a pointer to a char string with the ASCII Cab type . NULL if a valid dongle is not present.
//0 = standard sitdown, 1 = Upright, 2=Deluxe
const char * DongleAPI_GetCab();
//Returns the serial number as a long. 0 if no valid dongle is inserted.
long DongleAPI_GetSerialNumber();
