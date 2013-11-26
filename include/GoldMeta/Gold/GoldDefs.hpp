#pragma once

#include <GoldMeta/Shared.hpp>

namespace gm {
    // Forward declarations
    class IGoldPlugin;

    // The current GoldMeta interface version
    namespace Version {
        const int Major = 0;
        const int Minor = 1;
    }

    /// <summary>
    /// The API function that needs to expose the global 'IGoldPlugin' instance
    /// </summary>
    typedef IGoldPlugin*(__cdecl *FNGetPluginInstance)();

    /// <summary>
    /// The symbol used to identify the plugin instance API function
    /// </summary>
    static const char* GetPluginInstanceSymbol = "GetPluginInstance";

    /// <summary>
    /// Describes the engine API functions available
    /// </summary>
    enum class EngineAPI {
        PrecacheModel,
        PrecacheSound,
        SetModel,
        ModelIndex,
        ModelFrames,
        SetSize,
        ChangeLevel,
    };

    /// <summary>
    /// Describes the library API functions available
    /// </summary>
    enum class LibraryAPI {
        GameDLLInit,
        DispatchSpawn,
        DispatchThink,
        DispatchUse,
        DispatchTouch,
        DispatchBlocked,
    };
}
