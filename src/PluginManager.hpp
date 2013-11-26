#pragma once

#include <boost/filesystem.hpp>
#include <unordered_map>
#include <string>
#include <memory>

#include "Exception.hpp"
#include "PathManager.hpp"
#include "Interface/IEntityExporter.hpp"
#include "Plugin/PluginBase.hpp"

namespace /* Anonymous */ {
    namespace fs = boost::filesystem;
}

namespace gm {
    class PluginManager : public IEntityExporter {
    public:
        /// <summary>
        /// The exception class that the plugin manager throws
        /// </summary>
        GM_DEFINE_EXCEPTION(Exception);

        /// <summary>
        /// Constructs a plugin manager
        /// </summary>
        PluginManager(std::shared_ptr<PathManager> pathManager, std::string pluginsFile = "plugins.ini");

        /// <summary>
        /// Destructor for the plugin manager
        /// </summary>
        ~PluginManager();

        /// <summary>
        /// Gets a entity object residing in one of the plugin libraries
        /// </summary>
        virtual HL::FNEntity GetEntity(const std::string& entity);

        /// <summary>
        /// Sets the plugin source file (relative to GoldMeta data directory)
        /// </summary>
        void SetPluginSource(std::string path);

        /// <summary>
        /// Loads all plugins in the plugin configuration file
        /// </summary>
        void LoadConfigPlugins(bool unloadRemoved = false);

        /// <summary>
        /// Loads a plugin from a specified with and alias
        /// </summary>
        void LoadPlugin(fs::path path, std::string alias = "");

        /// <summary>
        /// Unloads a plugin with a specific ID
        /// </summary>
        void UnloadPlugin(PluginId pluginId);

        /// <summary>
        /// Attempts to find a plugin by using its alias
        /// </summary>
        std::shared_ptr<PluginBase> FindPlugin(const std::string& alias, bool exact = true);

        /// <summary>
        /// Attempts to find a plugin by its unique identifier
        /// </summary>
        std::shared_ptr<PluginBase> FindPlugin(const PluginId pluginId);

    private:
        /// <summary>
        /// Defines the properties for a plugin entry
        /// </summary>
        struct PluginEntry {
            std::string alias;
            fs::path path;
        };

        /// <summary>
        /// Parses all plugin entries in the current configuration file
        /// </summary>
        std::vector<PluginEntry> ParsePluginConfig();

        // We want to avoid any abnormally long variable type names
        typedef std::unordered_map<std::string, std::shared_ptr<PluginBase>> PluginCollection;

        // Private members
        std::shared_ptr<PathManager> mPathManager;
        PluginCollection mPlugins;
        PluginId mPluginCounter;
        fs::path mPluginsFile;
    };
}
