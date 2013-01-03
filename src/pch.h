#if !defined __PCH_H__
#define __PCH_H__

#include "targetver.h"

#if defined WIN32
    #define _CRTDBG_MAP_ALLOC
    #include <cstdlib>
    #include <crtdbg.h>

    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include <Io.h>
    #include <WinSock2.h>
    #include <UserEnv.h>
#else
    #if defined __APPLE__
        #include <AvailabilityMacros.h>
    #endif

    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <strings.h>
#endif

#if defined _MSC_VER
    #pragma warning(disable:4290)   // exception specification ignored except to indicate nothrow
    #pragma warning(disable:4396)   // inline specifier can't be used when a friend refers to a specialization
    #pragma warning(disable:4996)   // warnings about calling unsafe standard C++ methods
    #pragma warning(disable:4099)   // type name first seen using 'class' now seen using 'struct'
    #pragma warning(disable:4251)   // class X needs to have dll-interface to be used by clients of class Y
    //#pragma warning(disable:4800)   // type conversion, performance warning
    #pragma warning(disable:4244)   // type conversion, possible loss of data

    typedef signed __int8 int8_t;
    typedef unsigned __int8 uint8_t;
    typedef signed __int16 int16_t;
    typedef unsigned __int16 uint16_t;
    typedef signed __int32 int32_t;
    typedef unsigned __int32 uint32_t;
    typedef signed __int64 int64_t;
    typedef unsigned __int64 uint64_t;
#else
    #include <stdint.h>
#endif

#include <typeinfo>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <new>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
//#include <malloc.h>

#define BOOST_ALL_NO_LIB
#include <boost/version.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/any.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread.hpp>

#if defined HAS_CALLGRIND_H
    #include <valgrind/callgrind.h>
#endif

#if defined USE_SSE
    // SSE3
    #include <pmmintrin.h>
#endif

static const int MAX_BUFFER = 1024;

#if defined WIN32
 // can remove this once VC++ gets its shit together
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName&); \
TypeName& operator=(const TypeName&)
#else
// NOTE: also disables move operations
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName&) = delete; \
TypeName(TypeName&) = delete; \
TypeName& operator=(const TypeName&) = delete; \
TypeName& operator=(TypeName&) = delete
#endif

#include "src/core/errors/NotImplementedError.h"
#include "src/core/logging/Logger.h"
#include "src/core/util/MemoryAllocator.h"

#define BUFFER_OFFSET(i) (reinterpret_cast<char*>(0) + (i))

#if defined WIN32
    #define stdext std

    #define ALIGN(s) __declspec(align(s))
#else
    #define stdext __gnu_cxx
    #define __declspec(t)
    #define _CRTIMP

    #define ALIGN(s) __attribute__((aligned(s)))
#endif

#if defined WIN32
    #define snprintf _snprintf
    #define vsnprintf _vsnprintf
    #define strcasecmp _stricmp
    //#define getcwd _getcwd
    #define chmod _chmod
    //#define sleep(s) Sleep((s) * 1000)
    #define usleep(u) do { int m = (u) / 1000; Sleep(m <= 0 ? 1 : m); } while(0)

    // guess we can't be thread-safe...
    #define asctime_r(t, b) asctime((t))
    #define ctime_r(t, b) ctime((t))
    #define gmtime_r(t, r) gmtime((t))
    #define localtime_r(t, r) localtime((t))
    //#define rand_r(s) rand()
    #define strtok_r(s, d, b) strtok((s), (d))
#else
    // non-thread-safe functions are bad
    #define asctime asctime_r
    #define ctime ctime_r
    #define gmtime gmtime_r
    #define localtime localtime_r
    //#define rand rand_r
    #define strtok strtok_r

    // TODO: memalign is obsoleted by posix_memalign
    #define _aligned_malloc(s, b) memalign(b, s)
    #define _aligned_free free
#endif

#if defined WIN32
    #if !defined HAVE_SSIZE_T
        typedef long ssize_t;
    #endif
    typedef long pid_t;
#else
    typedef int HANDLE;
    typedef void* HINSTANCE;
    typedef HINSTANCE HMODULE;
#endif

#if defined WIN32
    #define SAFE_RELEASE(p) \
        if((p)) { \
            (p)->Release(); \
            (p) = nullptr; \
        } else (void)0
#else
    // from winsock2.h
    #define MAKEWORD(low, high) ((unsigned short)(((unsigned char)(low)) | ((unsigned short)((unsigned char)(high))) << 8))

    // from winnt.h
    #define ZeroMemory(d, l) std::memset((d), 0, (l))
#endif

#if defined WIN32
    #define PATH_SEPARATOR ";"
    #define DIR_SEPARATOR "\\"
    #define EOL "\r\n"

    // how stupid is this shit?
    #define POINT_IN 72     // 1 point = 1/72 inch
    #define TWIP_PT 20      // 1 twip = 1/20 point
    #define TWIP_IN 1440    // 1 twip = 1/1440 inch
    #define TWIP_CM 567     // 1 twip = 1/567 cm
#else
    #define PATH_SEPARATOR ":"
    #define DIR_SEPARATOR "/"
    #define EOL "\n"

    // this is from windef.h
    // though it seems kinda small
    #define MAX_PATH 260
#endif

#endif
