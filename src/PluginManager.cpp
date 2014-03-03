#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/tokenizer.hpp>
#include <boost/range.hpp>
#include <fstream>

#include "PluginManager.hpp"
#include "Plugin/GoldPlugin.hpp"
#include "Plugin/MetaPlugin.hpp"
#include "OS/Library.hpp"

namespace gm {
    PluginManager::PluginManager(std::shared_ptr<PathManager> pathManager, std::string pluginsFile) :
        mPluginCounter(PluginId(1)),
        mPathManager(pathManager)
    {
        // Update the plugin source file
        this->SetPluginSource(pluginsFile);
    }

    PluginManager::~PluginManager() {
        for(auto& pair : mPlugins) {
            // Unload each plugin in memory on destruction
            this->UnloadPlugin(pair.second->GetID());
        }
    }


    HL::FNEntity PluginManager::GetEntity(const std::string& entity) {
        // By default we return a null value
        HL::FNEntity result = nullptr;

        // Iterate over each plugin and check if they export the specific entity
        for(auto& plugin : mPlugins | boost::adaptors::map_values) {
            // Retrieve the plugin entity object
            result = plugin->GetEntity(entity);

            if(result != nullptr) {
                break;
            }
        }

        return result;
    }

    void PluginManager::SetPluginSource(std::string path) {
        // Fetch the directory path to the GoldMeta data folder
        fs::path data = mPathManager->GetPathObject(PathManager::GoldMetaData);

        boost::system::error_code ec;
        fs::path configPath = fs::canonical(data / path, ec);

        if(ec) {
            std::cerr << "[WARNING] Plugin configuration file error; " << ec.message() << std::endl;
            throw Exception("couldn't resolve plugin configuration file path");
        } else {
            mPluginsFile = configPath;
        }
    }

    void PluginManager::LoadConfigPlugins(bool unloadRemoved) {
        // Begin by parsing the plugin configuration file for entries
        std::vector<PluginEntry> entries = this->ParsePluginConfig();

        uint pluginsLoaded = 0;
        uint pluginsUpdated = 0;
        uint pluginsRemoved = 0;

        for(PluginEntry& entry : entries) {
            // Check if any plugin with this alias is already loaded
            std::shared_ptr<PluginBase> plugin = this->FindPlugin(entry.alias);

            boost::system::error_code error;

            // Since the path in the configuration file is relative to the game root directory, we make it absolute
            entry.path = fs::canonical(mPathManager->GetPathObject(PathManager::GameRoot) / entry.path, error);

            if(error) {
                std::cerr << format("[ERROR] Failed to resolve plugin path %s; %s\n") % entry.path % error.message();
                continue;
            }

            if(plugin) {
                // In case a plugin uses the same alias but a different path as a currently
                // loaded plugin, we replace the library already in memory with the new one.
                if(plugin->GetPath() != entry.path) {
                    std::cerr << format("[WARNING] Duplicate plugin entry aliases ('%s') with different libraries; replacing plugin \"%s\"...\n") % entry.alias % plugin->GetPath();

                    this->UnloadPlugin(plugin->GetID());
                    this->LoadPlugin(entry.path, entry.alias);

                    pluginsRemoved++;
                    pluginsLoaded++;
                } else if(fs::exists(entry.path)) {
                    // We want to reload the plugin if the library has been updated/modified
                    if(fs::last_write_time(entry.path) != plugin->GetLastModified()) {
                        try { /* Attempt to reload the plugin library */
                            plugin->Unload();
                            plugin->Load();

                            pluginsUpdated++;
                        } catch(const PluginBase::Exception& ex) {
                            std::cerr << "[ERROR] Failed to reload plugin: " << ex.what() << std::endl;
                            this->UnloadPlugin(plugin->GetID());
                            pluginsRemoved++;
                        }
                    }
                }
            } else try {
                // The plugin is not already in memory, so we simply load the library
                this->LoadPlugin(entry.path, entry.alias);
                pluginsLoaded++;
            } catch(const Exception& ex) {
                std::cerr << format("[ERROR] Failed to load plugin library %s; %s\n") % entry.path % ex.what();
            }
        }

        std::cout << "[INFO] The plugin configuration file has been successfully parsed\n\n"
                  << '\t' << "Plugins Loaded:  " << pluginsLoaded << std::endl
                  << '\t' << "Plugins Updated: " << pluginsUpdated << std::endl
                  << '\t' << "Plugins Removed: " << pluginsRemoved << "\n\n";
    }

    void PluginManager::LoadPlugin(fs::path path, std::string alias) {
        // Ensure that we are not loading any plugin library duplicates
        auto it = std::find_if(mPlugins.cbegin(), mPlugins.cend(), [&](const std::pair<std::string, std::shared_ptr<PluginBase>>& pair) {
            return pair.second->GetPath() == path || pair.first == alias;
        });

        if(it != mPlugins.end()) {
            // In case the user wants to load the same library again, he must first unload the current one
            throw Exception("tried to load a plugin library that was already loaded");
        } else if(fs::exists(path)) {
            // Since we support multiple different plugin types we construct a plugin factory
            std::vector<std::function<PluginBase*(PluginId, const fs::path&)>> factories;

            // NOTE: If the plugin fails to identify the type, the allocated memory will automatically be freed when throwing in the constructor
            factories.push_back([](PluginId id, const fs::path& path) { return new GoldPlugin(id, path); });
            factories.push_back([](PluginId id, const fs::path& path) { return new MetaPlugin(id, path); });

            std::shared_ptr<PluginBase> plugin;

            for(uint i = 0; i < factories.size() && !plugin; i++) {
                try { /* Call the factory function for identification */
                    plugin.reset(factories[i](mPluginCounter, path));
                } catch(const PluginBase::Exception& ex) {
                    std::cout << format("[INFO] Plugin index %d was invalid; '%s'\n") % i % ex.what();
                }
            }

            if(!plugin) {
                throw Exception("the plugin type could not be identified");
            } else {
                //plugin->Load();

                mPlugins[alias] = plugin;
                mPluginCounter++;
            }
        } else {
            throw Exception("the library path does not exist");
        }
    }

    std::shared_ptr<PluginBase> PluginManager::FindPlugin(const std::string& alias, bool exact) {
        assert(alias.length() > 0);

        if(exact) {
            auto it = mPlugins.find(alias);

            if(it == mPlugins.end()) {
                return nullptr;
            } else {
                return it->second;
            }
        } else {
            return nullptr;
        }
    }

    std::shared_ptr<PluginBase> PluginManager::FindPlugin(const PluginId pluginId) {
        assert(!(pluginId == 0));

        for(auto& plugin : mPlugins | boost::adaptors::map_values) {
            if(plugin->GetID() != pluginId) {
                continue;
            }

            return plugin;
        }

        return nullptr;
    }

    std::vector<PluginManager::PluginEntry> PluginManager::ParsePluginConfig() {
        // Open the file stream for reading plugins
        std::ifstream stream(mPluginsFile.native());
        std::vector<PluginEntry> result;

        if(!stream.good()) {
            throw Exception(format("couldn't open plugin configuration file '%s'") % mPluginsFile.string());
        }

        typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;
        uint line = 1;

        for(std::string input; std::getline(stream, input); boost::algorithm::trim(input), line++) {
            // Semicolons are comments and empty lines are discarded
            if(input.empty() || input[0] == ';') {
                continue;
            }

            Tokenizer tokenizer(input, boost::escaped_list_separator<char>('\\', ' ', '\"'));
            std::vector<std::string> plugin;

            for(const std::string& part : tokenizer) {
                if(part.empty()) {
                    continue;
                }

                plugin.push_back(part);
            }

            // The 'alias' and 'path' make up at most two parts
            if(plugin.size() > 2 || plugin.size() == 0) {
                std::cerr << format("[WARNING] Invalid plugin entry at line %d: '%s'\n") % line % input;
                continue;
            }

            PluginEntry entry;

            // No matter what extension the user has specified, we enforce a platform specific one
            entry.path = fs::path(plugin.back()).replace_extension(Library::GetExtension());

            if(plugin.size() > 1) {
                // In case an alias has been specified, use it
                entry.alias = plugin.front();
            } else {
                // ... otherwise use the filename as the alias
                entry.alias = entry.path.stem().string();
            }

            // Ensure that we do not have any duplicate plugin paths or aliases
            auto it = std::find_if(result.cbegin(), result.cend(), [&entry](const PluginEntry& plugin) {
                return entry.alias == plugin.alias || entry.path == plugin.path;
            });

            if(it == result.end()) {
                result.push_back(entry);
            } else {
                std::cerr << format("[WARNING] Duplicate plugin entry at line %d; aliases and plugins paths must be unique\n") % line;
            }
        }

        return result;
    }

    void PluginManager::UnloadPlugin(PluginId pluginId) {
    }
}
