#pragma once

#ifdef _WIN32
# define GM_STATIC_ENTRY
# define GM_STATIC_EXIT
# define GM_EXPORT extern "C" __declspec(dllexport)
# ifndef WINAPI
#  define WINAPI __stdcall
#  define STDCALL __stdcall
# endif
#else
# ifndef __GNUC__
#  error "You must use GCC on non-windows platforms"
# endif
# define GM_STATIC_ENTRY __attribute__((constructor))
# define GM_STATIC_EXIT __attribute__((destructor))
# define GM_EXPORT extern "C"
# define STDCALL __attribute__((ms_abi))
# define WINAPI
# if __APPLE__
# elif __linux__
# else
#  error "The target operating system is not supported"
# endif
#endif

#include <boost/filesystem.hpp>
#include <vector>
#include <string>

#include "../Exception.hpp"

namespace /* Anonymous */ {
    namespace fs = boost::filesystem;
}

namespace gm {
    /// <summary>
    /// The exception class that generic OS functions throw
    /// </summary>
    GM_DEFINE_EXCEPTION(Exception);

    /// <summary>
    /// Gets the executables command line arguments
    /// </summary>
    std::vector<std::string> GetCommandLineArguments();

    /// <summary>
    /// Describes the module information available
    /// </summary>
    struct Module {
        fs::path path;
        void* baseAddress;
        size_t size;
    };

    /// <summary>
    /// Gets all modules contained in the current process
    /// </summary>
    std::vector<Module> GetProcessModules();

    /// <summary>
    /// Gets the library path of a module that is identified by a memory address
    /// </summary>
    fs::path GetModulePath(void* memory);

    /// <summary>
    /// Describes the possible operative systems
    /// </summary>
    enum class OS {
        Windows = 1,
        Linux   = 2,
        Mac     = 4,
    };

    /// <summary>
    /// Gets the current OS
    /// </summary>
    OS GetCurrentOS();
}
