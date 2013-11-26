#include <cassert>
#include <cstring>
#ifdef _WIN32
# include <windows.h>
# include <TlHelp32.h>
# include <shellapi.h>
#else
# if !defined __linux__ && !defined __APPLE__
#  error "Only Linux and Mac OS X is supported on non-windows platforms"
# endif
# ifndef __GNUC__
#  error "Only GCC is supported on POSIX platforms"
# endif
// Give us access to additional functions
# define _POSIX_C_SOURCE = 200809L
//# define _GNU_SOURCE
# include <link.h>
# include <dlfcn.h>
#endif

#include "OS.hpp"

namespace gm {
    std::vector<std::string> GetCommandLineArguments() {
        // The result will be stored in here
        std::vector<std::string> commandLine;
#ifdef _WIN32
        int argc;
        wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

        if(argv == nullptr) {
            throw Exception("couldn't retrieve command line arguments");
        }
        
        for(int i = 0; i < argc; i++) {
            // Convert the argument string to UTF-8 encoding, so we use the encoding native for this library
            size_t sizeRequired = WideCharToMultiByte(CP_UTF8, 0, argv[i], wcslen(argv[i]), nullptr, 0, nullptr, nullptr);
            std::string result(sizeRequired, 0);
            WideCharToMultiByte(CP_UTF8, 0, argv[i], wcslen(argv[i]), const_cast<char*>(result.data()), sizeRequired, nullptr, nullptr);

            // Add each command line argument to our vector
            commandLine.push_back(std::move(result));
        }
#else
        // TODO: Implement for Linux and Mac OS X
#endif
        return commandLine;
    }
    
    std::vector<Module> GetProcessModules() {
        // Store all modules in a vector
        std::vector<Module> modules;
        
#ifdef _WIN32
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
        
        if(snapshot == INVALID_HANDLE_VALUE) {
            throw Exception("couldn't create module snapshot");
        }
        
        MODULEENTRY32 entry = {0};
        entry.dwSize = sizeof(MODULEENTRY32);
        
        for(BOOL result = Module32First(snapshot, &entry); result; result = Module32Next(snapshot, &entry)) {
            modules.push_back({ fs::path(entry.szExePath), static_cast<void*>(entry.modBaseAddr), entry.modBaseSize });
        }
#else
        dl_iterate_phdr([](struct dl_phdr_info* info, size_t size, void* data) {
            assert(data != nullptr);
            assert(info != nullptr);
            assert(size > 0);

            size_t moduleSize = 0;

            for(int i = 0; i < info->dlpi_phnum; i++) {
                // Calculate the total size of the module
                moduleSize += info->dlpi_phdr[i].p_memsz;
            }

            reinterpret_cast<std::vector<Module>*>(data)->push_back({ fs::path(info->dlpi_name), reinterpret_cast<void*>(info->dlpi_addr), moduleSize });
            return 0;
        }, &modules);
#endif
        return modules;
    }

    fs::path GetModulePath(void* memory) {
#ifdef _WIN32
        MEMORY_BASIC_INFORMATION information;
        std::memset(&information, 0, sizeof(MEMORY_BASIC_INFORMATION));

        if(!VirtualQuery(memory, &information, sizeof(MEMORY_BASIC_INFORMATION))) {
            throw Exception("couldn't query memory information");
        }

        if(information.State != MEM_COMMIT || !information.AllocationBase) {
            throw Exception("memory information was invalid");
        }

        std::vector<char> path(MAX_PATH);
        size_t result = 0;

        while(true) {
            // Attempt to retrieve the module file name (in case the buffer is too small, we repeat the process)
            result = GetModuleFileName(reinterpret_cast<HMODULE>(information.AllocationBase), path.data(), path.size());

            if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                path.resize(path.size() * 2);
                continue;
            }

            break;
        }

        if(result == 0) {
            throw Exception("couldn't retrieve module file name");
        }

        return fs::path(path.begin(), path.end());
#else
        Dl_info info;
        std::memset(&info, 0, sizeof(Dl_info));

        if (dladdr(memory, &info)) {
            return fs::path(info.dli_fname);
        } else {
            throw Exception("couldn't query memory information");
        }
#endif
    }

    gm::OS GetCurrentOS() {
#ifdef _WIN32
        return OS::Windows;
#elif __linux__
        return OS::Linux;
#elif __APPLE__
        return OS::Mac;
#endif
    }
}