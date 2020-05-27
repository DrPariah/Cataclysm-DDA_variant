#include "get_version.h" // IWYU pragma: associated

#if (defined(_WIN32) || defined(MINGW)) && !defined(GIT_VERSION) && !defined(CROSS_LINUX) && !defined(_MSC_VER)

#ifndef VERSION
#define VERSION "0.E"
#endif

#else

#include "version.h"

#endif

const char *getVersionString()
{
    return VERSION;
}


const char *getVariantVersionString()
{
    return "Variant_JP_0.E_r14 (based #10478)";
}
