#include "src/pch.h"
#include <iostream>

#if !defined WITH_UNIT_TESTS
int main(int argc, char* argv[])
{
    std::cerr << "ERROR! Should have been built with unit tests!" << std::endl;
    return 0;
}
#endif
