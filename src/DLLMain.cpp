#ifdef _WIN32
# include <windows.h>
#endif

#include "MetaMain.hpp"
#include "OS/OS.hpp"

namespace gm {
    bool DLLEntry() GM_STATIC_ENTRY {
        // Allocate our global instance
        gMetaMain = new MetaMain();

        if(gMetaMain == nullptr) {
#ifndef _WIN32
            // We cannot return false and exit the library on Linux or Mac OS X, so we use an exception instead
            throw std::exception("failed to allocate GoldMeta instance");
#endif
            return false;
        } else {
            return true;
        }
    }

    bool DLLExit() GM_STATIC_EXIT {
        if(gMetaMain != nullptr) {
            delete gMetaMain;
            gMetaMain = nullptr;
        }

        return true;
    }
}

#ifdef _WIN32
BOOL WINAPI DllMain(HANDLE handle, uint reason, void* reserved) {
    bool result = false;

    switch(reason) {
        case DLL_PROCESS_ATTACH:
            result = gm::DLLEntry();
            break;

        case DLL_PROCESS_DETACH:
            result = gm::DLLExit();
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            result = true;
            break;
    }

    return result;
}
#endif