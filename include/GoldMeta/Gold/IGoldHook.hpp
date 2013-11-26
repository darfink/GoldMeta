#pragma once

#include <GoldMeta/Gold/GoldDefs.hpp>

namespace gm {
    // Forward declarations
    class IModuleFunction;

    class IGoldHook {
    public:
        /// <summary>
        /// Describes the different memory areas a gold hook object resides
        /// </summary>
//         enum Memory {
//             Invalid, /* Should not be used, the memory is non-existent (used for custom types) */
//             Engine,  /* Memory within the HLDS engine (e.g 'swds.dll', 'engine_i686.so') */
//             Server,  /* Memory within the HLDS executable (i.e 'hlds[.exe]') */
//             Mod,     /* Memory within the game library (e.g 'mp.dll', 'cs.so') */
//         };

        /// <summary>
        /// Gets a function handler for a custom function
        /// </summary>
        virtual IModuleFunction* GetFunction(PluginId id, const char* name, void* addr) = 0;

        /// <summary>
        /// Gets a function handler for a engine API function
        /// </summary>
        virtual IModuleFunction* GetEngineFunction(PluginId id, EngineAPI function) = 0;

        /// <summary>
        /// Gets a function handler for a library API function
        /// </summary>
        //virtual IModuleFunction* GetLibraryFunction(PluginId id, LibraryAPI function) = 0;
    };
}
