#pragma once

#include <GoldMeta/Meta/MetaDefs.hpp>
#include <boost/filesystem.hpp>
#include <string>

#include "PluginBase.hpp"

namespace /* Anonymous */ {
    namespace fs = boost::filesystem;
}

namespace gm {
    class MetaPlugin : public PluginBase {
    public:
        /// <summary>
        /// The exception class that the meta plugin throws
        /// </summary>
        GM_DEFINE_EXCEPTION_INHERIT(Exception, PluginBase::Exception);

        /// <summary>
        /// Constructs a 'meta' plugin from a path with a specified ID
        /// </summary>
        MetaPlugin(PluginId id, const fs::path& path);

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
        Meta::FNMetaInit mMetaInit;
        Meta::FNMetaAttach mMetaAttach;
        Meta::FNMetaDetach mMetaDetach;
        Meta::FNMetaQuery mMetaQuery;
        HL::FNGiveFnptrsToDll mGiveFnptrsToDll;
    };
}
