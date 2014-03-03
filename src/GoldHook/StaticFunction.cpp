#include <asmjit/asmjit.h>
#include <udis86.h>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

#include "StaticFuntion.hpp"
#include "../OS/MemoryRegion.hpp"

namespace gm {
    // 'jmp <relative>' - This is probably the best detour type to use
    const byte StaticFunction::PatchRelative[] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };

    StaticFunction::StaticFunction(std::string name, ConventionInfo cInfo, void* address) :
        Function(name, cInfo),
        mBytesDisassembled(0)
    {
        assert(address != nullptr);
        mOriginal = address;

        ud_t ud;
        ud_init(&ud);

        // Assume the address consists of at least 20 bytes
        // TODO: Try to find a better solution than this (e.g search for 'ret'?)
        ud_set_input_buffer(&ud, static_cast<byte*>(mOriginal), 20);
        ud_set_mode(&ud, 32);

        while(mBytesDisassembled < GM_ARRAY_SIZE(PatchRelative)) {
            size_t bytes = ud_disassemble(&ud);

            if(bytes == 0) {
                throw Exception("couldn't disassemble enough bytes from target address");
            } else {
                mBytesDisassembled += bytes;
            }
        }
    }

    void* StaticFunction::GetCallableAddress() {
        if(mDetoured == true) {
            return mTrampoline.get();
        } else /* We can just return the original */ {
            return mOriginal;
        }
    }

    void StaticFunction::SetDetour(bool enabled) {
        // Ensure that we are working with a valid target
        assert(mBytesDisassembled > 0);

        if(mDetoured == enabled) {
            return;
        }

        MemoryRegion region(reinterpret_cast<uintptr_t>(mOriginal), mBytesDisassembled);
        region.SetFlags(MemoryRegion::Execute | MemoryRegion::Read | MemoryRegion::Write);

        if(enabled == true) {
            this->ApplyHook();
        } else /* Remove hook */ {
            this->RemoveHook();
        }
    }

    void StaticFunction::ApplyHook() {
        // This is only generated once, so it's safe to call it twice
        void* callback = mCodeGenerator->GenerateHookHandler();
        assert(callback != nullptr);

        // Allocate executable memory to backup the original function (i.e the trampoline)
        mTrampoline.reset(reinterpret_cast<byte*>(asmjit::MemoryManager::getGlobal()->alloc(mBytesDisassembled + GM_ARRAY_SIZE(PatchRelative))), [](byte* memory) {
            asmjit::MemoryManager::getGlobal()->release(memory);
        });

        // Copy the original function bytes to our trampoline
        std::memcpy(mTrampoline.get(), mOriginal, mBytesDisassembled);

        // To avoid any execution of the function whilst it is being modified, we
        // first create the patch in this vector, so we can copy it in one sweep
        std::vector<byte> patch(GM_ARRAY_SIZE(PatchRelative));
        std::memcpy(patch.data(), PatchRelative, patch.size());

        // Calculate the relative address to the callback function (the user provided one) from the current EIP
        *reinterpret_cast<uint*>(patch.data() + 0x01) = (reinterpret_cast<byte*>(callback) - reinterpret_cast<byte*>(mOriginal)) - patch.size();

        // Copy the patch to the original function to detour it!
        std::memcpy(mOriginal, patch.data(), patch.size());

        uint delta = mBytesDisassembled - GM_ARRAY_SIZE(PatchRelative);

        if(delta > 0) {
            std::vector<byte> nops(delta);
            std::fill(nops.begin(), nops.end(), 0x90);

            // In case we disassembled more instructions than we've replaced, there might be
            // corrupt ones left in the function. These will never be executed but just because
            // they _might_ do, we replace those invalid instructions with normal 'nops'
            std::memcpy(reinterpret_cast<byte*>(mOriginal) + GM_ARRAY_SIZE(PatchRelative), nops.data(), nops.size());
        }

        // If the user wants to execute the original function, we must first execute the bytes
        // that we replaced with the detour, and then jump to the rest of the function. So we
        // do this by adding a relative jump at the end of our trampoline.
        std::memcpy(patch.data(), PatchRelative, patch.size());

        // Calculate the relative address to the callback function (the user provided one) from the current EIP
        *reinterpret_cast<uint*>(patch.data() + 0x01) = (reinterpret_cast<byte*>(mOriginal) - reinterpret_cast<byte*>(&mTrampoline.get()[mBytesDisassembled])) - patch.size() + mBytesDisassembled;
        std::memcpy(&mTrampoline.get()[mBytesDisassembled], patch.data(), patch.size());

        // We have successfully hooked the function!
        mDetoured = true;
    }

    void StaticFunction::RemoveHook() {
        // Could it be more simple, or is it just me?
        std::memcpy(mOriginal, mTrampoline.get(), mBytesDisassembled);

        // Free our trampoline
        mTrampoline.reset();
        mDetoured = false;
    }
}
