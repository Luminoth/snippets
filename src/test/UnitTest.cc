#include "src/pch.h"
#if defined WITH_UNIT_TESTS
#if defined WIN32
#include <conio.h>
#else
#include <curses.h>
#endif
#include "src/core/common.h"
#include "src/core/util/Random.h"
#include "UnitTest.h"
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TextTestProgressListener.h>

int main(int argc, char* argv[])
{
    std::cout << "Initializing RNG..." << std::endl;
    energonsoftware::Random::seed();

    std::cout << "Initializing logging..." << std::endl;
    energonsoftware::Logger::configure(energonsoftware::Logger::Level::Debug);

    energonsoftware::Logger& logger(energonsoftware::Logger::instance("energonsoftware.test"));
    LOG_INFO("Initializing tests...\n");

#if defined _MSC_VER
    LOG_INFO("Visual C++ version: " << _MSC_VER << "\n");
#elif defined __GNUC__
    LOG_INFO("gcc version: " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__<< "\n");
#endif

    std::string testPath = (argc > 1) ? std::string(argv[1]) : "";
    CppUnit::TestResult controller;

    CppUnit::TestResultCollector result;
    controller.addListener(&result);

    CppUnit::TextTestProgressListener progress;
    controller.addListener(&progress);

    CppUnit::TestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
    try {
#if defined USE_SSE
        LOG_INFO("Using SSE\n");
#endif
#if defined USE_OPENSSL
        LOG_INFO("Using OpenSSL\n");
#endif

        LOG_INFO("Running tests "  <<  testPath << "\n");
#if defined WITH_CRYPTO
        LOG_INFO("Including crypto tests...\n");
#endif
#if defined WITH_TLS
        LOG_INFO("Including TLS tests...\n");
#endif
        runner.run(controller, testPath);

        CppUnit::CompilerOutputter outputter(&result, std::cerr);
        outputter.write();
    } catch(const std::invalid_argument &e) {
        LOG_ERROR("ERROR: " << e.what() << "\n");
        return 0;
    }

    std::cout << "Press any key to continue..." << std::endl;
    _getch();

    return result.wasSuccessful() ? 0 : 1;
}
#endif
