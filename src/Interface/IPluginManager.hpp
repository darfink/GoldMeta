#pragma once

#include <GoldMeta/Shared.hpp>
#include <boost/filesystem.hpp>
#include <string>

#include "IEntityExporter.hpp"

namespace /* Anonymous */ {
    namespace fs = boost::filesystem;
}

namespace gm {
    // Forward declarations
    class PluginBase;

    class IPluginManager : public IEntityExporter {
    public:
        /// <summary>
        /// Sets the plugin source file (relative to GoldMeta data directory)
        /// </summary>
        virtual void SetPluginSource(std::string path) = 0;

        /// <summary>
        /// Loads all plugins in the plugin configuration file
        /// </summary>
        virtual void LoadConfigPlugins(bool unloadRemoved = false) = 0;

        /// <summary>
        /// Loads a plugin from a specified with and alias
        /// </summary>
        virtual void LoadPlugin(fs::path path, std::string alias = "") = 0;

        /// <summary>
        /// Unloads a plugin with a specific ID
        /// </summary>
        virtual void UnloadPlugin(PluginId pluginId) = 0;

        /// <summary>
        /// Attempts to find a plugin by using its alias
        /// </summary>
        virtual std::shared_ptr<PluginBase> FindPlugin(const std::string& alias, bool exact = true) = 0;

        /// <summary>
        /// Attempts to find a plugin by its unique identifier
        /// </summary>
        virtual std::shared_ptr<PluginBase> FindPlugin(const PluginId pluginId) = 0;
    };
}
