// license:GPLv3+

#pragma once

namespace VPXPluginAPIImpl
{
	extern VPXPluginAPI g_vpxAPI;
	extern void RegisterVPXPluginAPI();

	// Helpers method for transitionning from COM object to plugins
	extern void PinMameOnStart();
};
