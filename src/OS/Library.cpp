#include <cassert>
#ifdef _WIN32
# include <windows.h>
#else
# include <dlfcn.h>
#endif

#include "Library.hpp"
#include "OS.hpp"

namespace gm {
    Library::Library() { }

    Library::Library(std::string libraryPath) {
        Load(libraryPath);
    }

    void Library::Load(std::string libraryPath) {
        assert(libraryPath.length() > 0);
        Unload();

#ifdef _WIN32
        mLibrary.reset(LoadLibraryA(libraryPath.c_str()), &FreeLibrary);
#else
        mLibrary.reset(dlopen(libraryPath.c_str(), RTLD_NOW), &dlclose);
#endif
        if(!mLibrary) {
            throw LibraryNotLoaded(format("couldn't load library '%s': %s") % libraryPath % GetLibraryError());
        } else {
            mPath = libraryPath;
        }
    }

    void Library::Unload() {
        if(!IsLoaded()) {
            return;
        }

        mLibrary.reset();
        mSymbols.clear();
        mPath.clear();
    }

    const std::string& Library::GetPath() const {
        assert(IsLoaded());
        return mPath;
    }

    void* Library::GetSymbol(const std::string& symbol, bool throwIfNotFound) {
        assert(IsLoaded());
        auto it = mSymbols.find(symbol);

        if(it != mSymbols.end()) {
            return it->second;
        } else {
            void* address = nullptr;
#ifdef _WIN32
            address = GetProcAddress(reinterpret_cast<HMODULE>(mLibrary.get()), symbol.c_str());
#else
            address = dlsym(mLibrary.get(), symbol.c_str());
#endif
            if(address != nullptr) {
                mSymbols[symbol] = address;
                return address;
            } else if(throwIfNotFound) {
                throw SymbolNotFound(format("couldn't find symbol '%s'") % symbol /*% GetLibraryError()*/);
            }
        }

        return nullptr;
    }

    bool Library::IsLoaded() const {
        return !!mLibrary;
    }

    void* Library::GetCodeBase(size_t* size) {
        assert(IsLoaded());

#ifdef _WIN32
        uint baseAddress = reinterpret_cast<uint>(mLibrary.get());

        auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(mLibrary.get());
        auto ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(baseAddress + dosHeader->e_lfanew);

        uint textSectionOffset = ntHeader->OptionalHeader.BaseOfCode;

        MEMORY_BASIC_INFORMATION information;
        if(!VirtualQuery(reinterpret_cast<void*>(baseAddress + textSectionOffset), &information, sizeof(MEMORY_BASIC_INFORMATION))) {
            throw Exception("couldn't query module information");
        }

        if(size != nullptr) {
            *size = information.RegionSize;
        }

        return reinterpret_cast<void*>(baseAddress + textSectionOffset);
#else
#endif
    }

    const std::string& Library::GetExtension() {
        static std::string extension;

        if(extension.empty()) {
#ifdef _WIN32
            extension = ".dll";
#elif __linux__
            extension = ".so";
#else
            extension = ".dylib";
#endif
        }

        return extension;
    }

    std::string Library::GetLibraryError() const {
#ifdef _WIN32
        uint lastError = GetLastError();

        if(lastError != 0)
        {
            void* messageBuffer;

            size_t length = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                lastError,
                MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                reinterpret_cast<char*>(&messageBuffer),
                0,
                nullptr);

            if(length > 0)
            {
                char* message = reinterpret_cast<char*>(messageBuffer);
                std::string result(message, message + length);

                LocalFree(messageBuffer);
                return result;
            }
        }

        return std::string();
#else
        return dlerror();
#endif
    }
}