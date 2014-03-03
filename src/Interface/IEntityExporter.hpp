#pragma once

#include <string>

namespace HL {
    // Forward declarations
    typedef void*(*FNEntity)(struct entvars_s*);
}

namespace gm {
    class IEntityExporter {
    public:
        /// <summary>
        /// Gets an entity object export function by name
        /// </summary>
        virtual HL::FNEntity GetEntity(const std::string& entity) = 0;
    };
}
