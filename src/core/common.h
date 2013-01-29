#if !defined __COMMON_H__
#define __COMMON_H__

namespace energonsoftware {

// TODO: move this out of core, it doens't belong here

// sleep time in microseconds
// (eg. use like boost::this_thread::sleep(boost::posix_time::microseconds(thread_sleep_time()))
inline int thread_sleep_time()
{
#if defined _DEBUG
    return 1000;
#else
    return 100;
#endif
}

#if defined WIN32
    /* TODO: move these to the project or something */
    #define INSTALLDIR ""
    #define BINDIR "bin"
    #define CONFDIR "etc\\snippets"
    #define DATADIR "share\\snippets"
#endif

inline boost::filesystem::path install_dir()
{
    return INSTALLDIR;
}

inline boost::filesystem::path bin_dir()
{
    return install_dir() / BINDIR;
}

inline boost::filesystem::path conf_dir()
{
    return install_dir() / CONFDIR;
}

inline boost::filesystem::path data_dir()
{
    return install_dir() / DATADIR;
}

inline boost::filesystem::path test_conf()
{
    return "test.conf";
}

}

#endif
