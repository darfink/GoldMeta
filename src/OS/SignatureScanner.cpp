#include <GoldMeta/Shared.hpp>
#include <cassert>
#include <cstring>
#include <vector>
#ifdef _WIN32
# include <windows.h>
#else
# include <dlfcn.h>
# include <sys/stat.h>
#endif

#include "SignatureScanner.hpp"
#include "OS.hpp"

namespace gm {
    SignatureScanner::SignatureScanner(uintptr_t base, size_t length) :
        mBaseAddress(base),
        mBaseLength(length)
    {
    }

    SignatureScanner::SignatureScanner(uintptr_t address) :
        mBaseAddress(0),
        mBaseLength(0)
    {
        assert(address);
#ifdef _WIN32
        MEMORY_BASIC_INFORMATION memory;

        if(!VirtualQuery(reinterpret_cast<void*>(address), &memory, sizeof(MEMORY_BASIC_INFORMATION))) {
            throw Exception("couldn't query memory information from address");
        }

        mBaseAddress = reinterpret_cast<uintptr_t>(memory.AllocationBase);

        IMAGE_DOS_HEADER* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(mBaseAddress);
        IMAGE_NT_HEADERS* ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<uint>(dosHeader) + dosHeader->e_lfanew);

        if(ntHeader->Signature != IMAGE_NT_SIGNATURE) {
            throw Exception("the address provided is not a PE executable");
        }

        mBaseLength = static_cast<size_t>(ntHeader->OptionalHeader.SizeOfImage);
#else /* POSIX */
        Dl_info info;
        struct stat buf;

        if(!dladdr(reinterpret_cast<void*>(address), &info)) {
            throw Exception("couldn't retrieve memory information from address");
        }

        if(!info.dli_fbase || !info.dli_fname) {
            throw Exception("the retrieved memory information was invalid");
        }

        if(stat(info.dli_fname, &buf) == -1) {
            throw Exception("couldn't query owning process executable");
        }

        mBaseAddress = brute_cast<uint>(info.dli_fbase);
        mBaseLength = buf.st_size;
#endif
    }

    uintptr_t SignatureScanner::FindSignature(const std::vector<byte>& signature, const char* mask, int offset /*= 0*/) {
        assert(signature.size() == strlen(mask));

        uintptr_t start = mBaseAddress;
        uintptr_t end = mBaseAddress + mBaseLength;

        MEMORY_BASIC_INFORMATION memInfo;
        size_t x = 0;

        while(start < end) {
            // We don't want to access protected memory (will cause an access violation)
            VirtualQuery(reinterpret_cast<void*>(start), &memInfo, sizeof(MEMORY_BASIC_INFORMATION));

            // Calculate the bounds for the current memory region
            const size_t Region = reinterpret_cast<uintptr_t>(memInfo.BaseAddress) + memInfo.RegionSize;

            if(!(memInfo.Protect & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_WRITECOPY | PAGE_READONLY | PAGE_READWRITE)) ||
               !(memInfo.State & MEM_COMMIT) || (memInfo.Protect & PAGE_GUARD)) {
                    start = Region;
                    x = 0;

                    continue;
            }

            bool quit = false;

            for(; start < Region && !quit; start++) {
                for(; x < signature.size(); x++) {
                    // Ensure that we aren't intruding in a new memory region
                    if((start + x) >= Region) {
                        // We must decrement 'x', otherwise it might equal 'signature.size()' and we 'think' we have a match
                        quit = true;
                        x--;

                        break;
                    }

                    // Exit the loop if the signature did not match with the source
                    if(mask[x] != '?' && signature[x] != reinterpret_cast<byte*>(start)[x]) {
                        break;
                    }
                }

                // If the increment count equals the signature length, we know we have a match!
                if(x == signature.size()) {
                    return start + offset;
                } else {
                    x = 0;
                }
            }
        }

        return 0;
    }
}
