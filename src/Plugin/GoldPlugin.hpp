#pragma once

#include <GoldMeta/Gold/GoldDefs.hpp>
#include <boost/filesystem.hpp>
#include <string>

#include "PluginBase.hpp"

namespace /* Anonymous */ {
    namespace fs = boost::filesystem;
}

namespace gm {
    class GoldPlugin : public PluginBase {
    public:
        /// <summary>
        /// The exception class that the gold plugin throws
        /// </summary>
        GM_DEFINE_EXCEPTION_INHERIT(Exception, PluginBase::Exception);

        /// <summary>
        /// Constructs a 'gold' plugin from a path with a specified ID
        /// </summary>
        GoldPlugin(PluginId id, const fs::path& path);

        /// <summary>
        /// Loads the plugin library
        /// </summary>
        virtual void Load();

        /// <summary>
        /// Unloads the plugin library
        /// </summary>
        virtual void Unload();

        /// <summary>
        /// Gets the plugin type
        /// </summary>
        virtual Type GetType() const;

    private:
        // Private members
        FNGetPluginInstance mGetInstance;
        IGoldPlugin* mPlugin;
    };
}
