#pragma once

#include <vector>

#include "../Default.hpp"
#include "../Exception.hpp"

namespace gm {
    class MemoryRegion {
    public:
        /// <summary>
        /// The exception class that the memory region class throws
        /// </summary>
        GM_DEFINE_EXCEPTION(Exception);

        /// <summary>
        /// Describes the different memory flags
        /// </summary>
        enum Flags : ulong {
            Execute = (1 >> 0),
            Write   = (1 >> 1),
            Read    = (1 >> 2)
        };

        /// <summary>
        /// Constructs a memory region within a specific area
        /// </summary>
        MemoryRegion(uintptr_t address, size_t size);

        /// <summary>
        /// Destructor for the memory region
        /// </summary>
        virtual ~MemoryRegion();

        /// <summary>
        /// Sets the protection flags of all pages within the region
        /// </summary>
        void SetFlags(ulong flags);

        /// <summary>
        /// Gets the number of pages within the region
        /// </summary>
        uint GetPageCount() const;

    private:
        /// <summary>
        /// Describes a page within the region
        /// </summary>
        struct Page {
            size_t size;
            uintptr_t base;
            ulong currentFlags;
            ulong initialFlags;
        };

        // Private members
        std::vector<Page> mPages;
        size_t mPageSize;
        uintptr_t mAddress;
        size_t mSize;
        bool mReset;
    };
}
