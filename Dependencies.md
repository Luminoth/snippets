* C++11 compiler
** g++ 4.7.1 and Scons 2.3.0
** Visual Studio 2013 Update 4
* boost 1.55.0 - http://www.boost.org/
** For Win32, follow the Getting Started instructions
* CppUnit 1.12.1 - http://sourceforge.net/apps/mediawiki/cppunit/index.php?title=Main_Page
** For Win32, follow the INSTALL-WIN32.txt directions, build Debug/Release Unicode libraries (or DLL if desired)
*** Building the debug library may fail to copy, just do it by hand (into the lib dir)
* Xerces-C++ 3.1.1 - http://xerces.apache.org/xerces-c/
** For Win32, get the pre-built binaries for x86 (or x64 if building for that target)
*** Lib is xerces-c_3.lib
* zlib 1.2.8 - http://www.zlib.net/
** For Win32, this is required to build libpng on Windows, needs to match the version libpng is expecting
*** Just unzip the source, no need to build
* libpng 1.6.17 - http://www.libpng.org/
** For Win32, solution is in the projects/vstudio directory
*** Modify the zlib.props as per the instructions in the solution
*** Build the Release configuration (Release Library doens't build the DLL)
* OpenSSL 1.0.1j - http://www.openssl.org/
** For Win32, follow the INSTALL.W32 instructions
*** Use the VS2013 dev command prompt to get compiler tools in path
*** Need to make sure and install to get headers and libs in include/ and lib/
*** Requires ​Perl and ​NASM to build
* GNU TLS 3.3.13 - http://www.gnutls.org/
** Use the pre-built Win32 libs
** Open a Visual Studio command window in the gnutls bin directory and run lib /def:libgnutls-28.def. Move the generated .lib and .exp into the gnutls lib directory

EventLog viewer requires Python 2.7, wxWidgets 2.8.12 and wxPython 3.0.0.0
