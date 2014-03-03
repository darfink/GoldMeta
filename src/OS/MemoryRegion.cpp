#include <map>
#include <regex>
#include <string>
#include <fstream>
#ifdef _WIN32
# include <windows.h>
#else
# include <sys/mman.h>
#endif

#include "MemoryRegion.hpp"
#include "OS.hpp"

namespace gm {
    MemoryRegion::MemoryRegion(uintptr_t address, size_t size) :
        mAddress(address),
        mSize(size),
        mPageSize(0),
        mReset(true)
    {
        assert(address > 0);
        assert(size > 0);

#ifdef _WIN32
#else
        if((mPageSize = sysconf(_SC_PAGE_SIZE)) == -1) {
            throw Exception("couldn't retrieve system page size");
        }

        uintptr_t startPage = (address & ~(mPageSize - 1));
        uintptr_t lastPage = ((address + size) & ~(mPageSize - 1));

        uint pageCount = (lastPage - startPage / mPageSize) + 1;
        mPages.reserve(pageCount);

        std::ifstream fmaps("/proc/self/maps");
        std::regex regex("^([0-9a-fA-F]+)-[0-9a-fA-F]+ (\\w|-){4}");
        std::string input;

        for(uint i = 0; i < pageCount; i++) {
            Page page = {};

            page.size = mPageSize;
            page.base = startPage + (mPageSize * i);

            while(std::getline(fmaps, input)) {
                // Create our regex iterator so we can check each of our group
                std::sregex_token_iterator it(input.begin(), input.end(), regex, 1);

                // The addresses are in hex, and therefore we must add '0x'
                // otherwise 'stoul' won't recognize the string as hex
                if(std::stoul(str(format("0x%s") % *it), 0, 16) != page.base) {
                    continue;
                }

                std::string permissions = *(++it);

                static const std::map<char, ulong> Keys = {
                    { 'r', Read },
                    { 'w', Write },
                    { 'x', Execute },
                };

                int index = 0;

                for(auto& pair : Keys) {
                    if(permissions[index++] == pair.first) {
                        page.initialFlags |= pair.second;
                    }
                }
            }

            // We haven't changed any flags yet
            page.currentFlags = page.initialFlags;
            mPages.push_back(page);
        }
#endif
    }

    MemoryRegion::~MemoryRegion() {
        if(!mReset) {
            return;
        }

        for(Page& page : mPages) {
#ifdef _WIN32
            static ulong dummy;
            VirtualProtect(page.base, page.size, page.initialFlags, dummy);
#else
            mprotect(reinterpret_cast<void*>(page.base), page.size, page.initialFlags);
#endif
        }
    }

    void MemoryRegion::SetFlags(ulong flags) {
        for(Page& page : mPages) {
#ifdef _WIN32
            static ulong dummy;
            if(!VirtualProtect(page.base, page.size, flags, dummy)) {
                throw Exception("couldn't update memory region flags");
            }
#else
            if(!mprotect(reinterpret_cast<void*>(page.base), page.size, page.initialFlags)) {
                throw Exception("couldn't update memory region flags");
            }
#endif
            page.currentFlags = flags;
        }
    }

    uint MemoryRegion::GetPageCount() const {
        return mPages.size();
    }
}
