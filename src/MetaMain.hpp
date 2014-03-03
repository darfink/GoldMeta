#pragma once

#include <unordered_set>
#include <memory>

#include "HLExport.hpp"
#include "Exception.hpp"

namespace HL {
    typedef void*(*FNEntity)(struct entvars_s*);
}

namespace gm {
    // Forward declarations
    class GoldHook;
    class PathManager;
    class GameLibrary;
    class PluginManager;
    class IHookContext;

    /// <summary>
    /// The mother load instance for this library
    /// </summary>
    class MetaMain {
    public:
        /// <summary>
        /// The exception class that the MetaMain instance throws
        /// </summary>
        GM_DEFINE_EXCEPTION(Exception);

        /// <summary>
        /// Constructs a default MetaMain instance
        /// </summary>
        MetaMain();

        /// <summary>
        /// Called whenever an exported API has been triggered
        /// </summary>
        bool ExportAPI(ExportType type, ...);

    private:
        /// <summary>
        /// Setups and initializes all critical components
        /// </summary>
        void Initialize();

        /// <summary>
        /// The callback for the engine API hook
        /// </summary>
        HL::FNEntity EntityHook(IHookContext* hookContext, const char* symbol);

        // Private members
        std::shared_ptr<GoldHook> mGoldHook;
        std::shared_ptr<PathManager> mPathManager;
        std::shared_ptr<GameLibrary> mGameLibrary;
        std::shared_ptr<PluginManager> mPluginManager;
        std::unordered_set<std::string> mEntitySymbols;
        HL::enginefuncs_t* mEngineFunctions;
        HL::globalvars_t* mEngineGlobals;
    };

    /// <summary>
    /// Our global MetaMain instance
    /// </summary>
    extern MetaMain* gMetaMain;
}
