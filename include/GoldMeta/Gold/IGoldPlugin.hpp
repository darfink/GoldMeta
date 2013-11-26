#pragma once

#include <GoldMeta/Gold/GoldDefs.hpp>

namespace gm {
    // Forward declarations
    class IGoldAPI;

    /// <summary>
    /// The GoldMeta plugin interface that all libraries need to expose
    /// </summary>
    class IGoldPlugin {
    public:
        /// <summary>
        /// The GoldMeta plugin interface that all libraries need to expose
        /// </summary>
        virtual void GetApiVersion(int& major, int& minor) const = 0;

        /// <summary>
        /// The GoldMeta plugin interface that all libraries need to expose
        /// </summary>
        virtual bool Load(PluginId id, IGoldAPI* goldApi) = 0;

        /// <summary>
        /// The GoldMeta plugin interface that all libraries need to expose
        /// </summary>
        virtual bool Unload() = 0;

        /// <summary>
        /// Returns the reason why the most recent operation failed
        /// </summary>
        virtual const char* GetLastError() = 0;
    };
}
