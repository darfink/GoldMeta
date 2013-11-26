#pragma once

#include <GoldMeta/Gold/IGoldHook.hpp>
#include <GoldMeta/Gold/IModuleFunction.hpp>
#include <memory>
#include <string>
#include <map>

#include "PathManager.hpp"
#include "GoldHook/DataType.hpp"
#include "GoldHook/StaticFuntion.hpp"

namespace /* Anonymous */ {
    namespace fs = boost::filesystem;
}

namespace gm {
    class GoldHook : public IGoldHook {
    public:
        /// <summary>
        /// The exception class that the 'GoldHook' instance throws
        /// </summary>
        GM_DEFINE_EXCEPTION(Exception);

        /// <summary>
        /// Constructs a 'GoldHook' instance object
        /// </summary>
        GoldHook(std::shared_ptr<PathManager> pathManager, std::string dbFile = "database.db");

        /// <summary>
        /// Gets a function handler for a custom function
        /// </summary>
        virtual IModuleFunction* GetFunction(PluginId id, const char* name, void* addr);

        /// <summary>
        /// Gets a function handler for a engine API function
        /// </summary>
        virtual IModuleFunction* GetEngineFunction(PluginId id, EngineAPI function);

        /// <summary>
        ///
        /// </summary>
        void LoadCustomTypes();

    private:
        /// <summary>
        /// Loads a function from the database
        /// </summary>
        IModuleFunction* LoadFunction(const std::string& name);

        // Private members
        std::shared_ptr<PathManager> mPathManager;
        std::map<std::string, std::shared_ptr<StaticFunction>> mStaticFunctions;
        std::map<std::string, DataType> mTypes;
    };
}
