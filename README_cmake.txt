Instructions for building VPinball from source with CMake
---------------------------------------------------------

This has been tested with Visual Studio (VS) 2008 + 2010, Professional and Express editions.

Prerequisites:
--------------
- VPinball source code
  Seems like you already got that.

- Microsoft Visual Studio
  The Express edition is free: http://www.microsoft.com/express/Downloads/#2010-Visual-CPP

- CMake, latest version
  Download: http://www.cmake.org/cmake/resources/software.html
  Note:
  - For VS2010, minimum required version is CMake v2.8.4.
  - If you want to call CMake from the command prompt, make sure it is added to the system path during setup.

- Microsoft DirectX SDK (August 2007)
  Download: http://www.microsoft.com/downloads/en/details.aspx?FamilyID=529f03be-1339-48c4-bd5a-8506e5acf571
  Notes:
  - You may save some drive space by only installing the "DirectX Headers and Libs" part of the package.
  - With VS2010, there is a conflict in one SDK header: You must rename the file
    "c:\Program Files\Microsoft DirectX SDK (August 2007)\Include\rpcsal.h" to something different
    (e.g. "rpcsal.h_OLD") so the compiler will not find it anymore.
  
To build with Visual Studio Express, you also need:
- Microsoft Windows Server 2003 R2 Platform SDK
  Download: http://www.microsoft.com/downloads/en/details.aspx?FamilyId=E15438AC-60BE-41BD-AA14-7F1E0F19CA0D&displaylang=en
  (Note: ISO image. Burn or mount with a virtual CDRom software - for instance VirtualCloneDrive.)
  Notes:
  - You only need the Core Toolset with Headers and Libs. Select custom install and deselect everything except
    "Microsoft Windows Core SDK" -> "Build Environment" -> "Build Environment (x86 32-bit)" 
  - There is an error(!) in one header file: You have to add a missing int keyword in line 1753 of file
    "C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Include\atl\atlwin.h":
    -> "for(int i = 0; i < m_aChainEntry.GetSize(); i++)"
            ---

Building VPinball (via CMake GUI):
----------------------------------

- Checkout VPinball trunk
- Create a build directory for VPinball
  (Not required, but helps prevent cluttering the source directory with build files)

- Start the graphical CMake GUI
- Select the source directory of VPinball and the destination build directory
- Click <Configure>, Select your version of Visual Studio
- Click <Generate>
  (This generates project files. There should be a "vpinball.sln" created in your build directory.)

- Open "vpinball.sln" in Visual Studio
- Build the project 


Building VPinball (via Command Prompt):
---------------------------------------

- Checkout VPinball trunk
- Create a build directory for VPinball
  (Not required, but helps prevent cluttering the source directory with build files)

- Open a Visual Studio Command Prompt (for x86)
- cd to the build directory

For VS2008 (assuming the sources are at 'd:\Data\vpinball'):
- cmake -G "Visual Studio 9 2008" d:\Data\vpinball
or for VS2010:
- cmake -G "Visual Studio 10" d:\Data\vpinball
  (This generates project files. There should be a "vpinball.sln" created in your build directory.)
  
- Open "vpinball.sln" in Visual Studio
- Build the project 


Random Notes / Troubleshooting:
-------------------------------
- If CMake stops with an error, stating that DirectX or a required header could not be found, this
  indicates a non-standard (or localized) installation path for the DevKits.
  Please check the "find_path" commands in CMakeLists.txt file and adapt their search paths
  to match your local environment. 

- In theory, CMake offers x64 builds of the software.
  In practice this will fail for several reasons. Even if you get to the linking stage, x64 builds
  of all dependent libs are required - some of which are probably not available anywhere
  (hid, xaudio, ...).

- If you decide to modify the CMakeLists.txt build script, you may want to delete the cache
  file called "CMakeCache.txt" in your build directory before re-running CMake.

- The VPinball project contains a very simple INSTALL target which copies the binary
  output to a destination directory (no packaging or installer being created for now).
  You may manually specify the target directory via "-DCMAKE_INSTALL_PREFIX" command line
  option. Example:  
  cmake -G "Visual Studio 9 2008" -DCMAKE_INSTALL_PREFIX=c:\data c:\data\vpinball  

- There are various build configurations to choose from:
  - Debug = Adds debugging information in the build.
  - Release = Optimizations enabled. Corresponds to the "Release MinDependency Fast" config in the old project files.
  - RelWithSSE = Corresponds to the "Release MinDependency Fast+SSE" config in the old project files.
  - RelWithDebInfo = Adds debug info while keeping optimizations.
  (You may add more configs or adapt the existing ones to your needs. Just search the CMakeLists.txt file for those
  strings.)
