#pragma once

#include <vector>

#include "../Exception.hpp"

namespace gm {
    class SignatureScanner {
    public:
        /// <summary>
        /// The exception class that the signature scanner throws
        /// </summary>
        GM_DEFINE_EXCEPTION(Exception);

        /// <summary>
        /// Constructs a signature scanner within a specific area
        /// </summary>
        SignatureScanner(uintptr_t base, size_t length);

        /// <summary>
        /// Constructs a signature scanner for the specific address
        /// </summary>
        SignatureScanner(uintptr_t address);

        /// <summary>
        /// Searches for a signature in the specified memory
        /// </summary>
        uintptr_t FindSignature(const std::vector<byte>& signature, const char* mask, int offset = 0);

    private:
        // Private members
        uintptr_t mBaseAddress;
        size_t mBaseLength;
    };
}
