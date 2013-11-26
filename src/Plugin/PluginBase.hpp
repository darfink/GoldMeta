#pragma once

#include <GoldMeta/Shared.hpp>
#include <boost/filesystem.hpp>

#include "../Exception.hpp"
#include "../OS/Library.hpp"
#include "../Interface/IEntityExporter.hpp"

namespace /* Anonymous */ {
    namespace fs = boost::filesystem;
}

namespace gm {
    class PluginBase : public IEntityExporter {
    public:
        /// <summary>
        /// The base exception class that the plugins throw
        /// </summary>
        GM_DEFINE_EXCEPTION(Exception);

        /// <summary>
        /// Describes all plugin types
        /// </summary>
        enum Type {
            Metamod,
            Goldmeta,
        };

        /// <summary>
        /// Constructs a base plugin from a path with a specified ID
        /// </summary>
        PluginBase(PluginId id, const fs::path& path);

        /// <summary>
        /// Loads the plugin library
        /// </summary>
        virtual void Load() = 0;

        /// <summary>
        /// Unloads the plugin library
        /// </summary>
        virtual void Unload() = 0;

        /// <summary>
        /// Gets the plugin type
        /// </summary>
        virtual Type GetType() const = 0;

        /// <summary>
        /// Gets the plugin's path to the library file
        /// </summary>
        const std::string& GetPath() const;

        /// <summary>
        /// Gets the time when the plugin library was most recently modified
        /// </summary>
        std::time_t GetLastModified() const;

        /// <summary>
        /// Gets the plugin's unique identifier
        /// </summary>
        PluginId GetID() const;

        /// <summary>
        /// Gets a entity object export function by name
        /// </summary>
        virtual HL::FNEntity GetEntity(const std::string& entity);

    protected:
        // Protected members
        Library mLibrary;

    private:
        // Private members
        std::time_t mLastModified;
        PluginId mPluginId;
    };
}
