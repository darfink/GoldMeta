#pragma once

#include <GoldMeta/Shared.hpp>
#include <cstdarg>

#ifndef WINAPI
# ifdef _WIN32
#  define WINAPI __stdcall
# else /* POSIX */
#  define WINAPI
# endif
#endif

namespace HL {
    // Forward declarations
    typedef struct globalvars_s globalvars_t;
    typedef struct enginefuncs_s enginefuncs_t;
    typedef struct new_dll_functions_s NEW_DLL_FUNCTIONS;
    typedef struct dll_functions_s DLL_FUNCTIONS;

    typedef void(WINAPI *FNGiveFnptrsToDll)(enginefuncs_t*, globalvars_t*);
    typedef int(__cdecl *FNGetNewDLLFunctions)(NEW_DLL_FUNCTIONS*, int*);
    typedef int(__cdecl *FNGetEntityAPI2)(DLL_FUNCTIONS*, int*);
    typedef int(__cdecl *FNGetEntityAPI)(DLL_FUNCTIONS*, int);
}

namespace gm { namespace Meta {
    // Forward declarations
    struct GameLibraryFunctions;
    struct PluginInfo;
    struct Functions;
    struct Globals;
    struct MetaAPI;

    // The current (and probably last) Metamod interface version
    namespace Version {
        const int Major = 5;
        const int Minor = 13;
    }

    /// <summary>
    /// Describes events for when a plugin can be loaded/unloaded
    /// </summary>
    enum class LoadTime {
        Never,
        Startup,     /* Should only be loaded/unloaded at initial HLDS execution */
        ChangeLevel, /* Can be loaded/unloaded between maps */
        AnyTime,     /* Can be loaded/unloaded at any time */
        AnyPause,    /* Can be loaded/unloaded at any time, and can be "paused" during a map */
    };

    enum class UnloadReason {
        Null,
        ConfigDeleted,      /* Was deleted from plugins.ini */
        FileModified,       /* File on disk is newer than last load */
        ServerCommand,      /* Requested by server/console command */
        ServerForceCommand, /* Forced by server/console command */
        Delayed,            /* Delayed from previous request; can't tell origin */
        PluginCall,         /* Requested by plugin function call */
        PluginForcedCall,   /* Forced by plugin function call */
        Reload,             /* Forced unload by reload */
    };

    /// <summary>
    /// The Metamod API function called during initialization
    /// </summary>
    typedef void(__cdecl *FNMetaInit)();

    /// <summary>
    /// The Metamod API function called when the plugin needs to be queried
    /// </summary>
    typedef int(__cdecl *FNMetaQuery)(const char*, PluginInfo**, MetaAPI*);

    /// <summary>
    /// The Metamod API function called when the plugin is attached
    /// </summary>
    typedef int(__cdecl *FNMetaAttach)(LoadTime, Functions*, Globals*, GameLibraryFunctions*);

    /// <summary>
    /// The Metamod API function called when the plugin is detached
    /// </summary>
    typedef int(__cdecl *FNMetaDetach)(LoadTime, UnloadReason);

    /// <summary>
    /// The Metamod API function for supplying the engine functions
    /// </summary>
    typedef int(__cdecl *FNGetEngineFunctions)(HL::enginefuncs_t*, int*);

    /// <summary>
    /// The Metamod API globals (this is mimicked in 'HookContext')
    /// </summary>
    struct Globals {
        Result result;
        Result previousResult;
        Result highestResult;
        void* originalReturn;
        void* overrideReturn;
    };

    /// <summary>
    /// The Metamod callback functions available
    /// </summary>
    struct Functions {
        HL::FNGetEntityAPI getEntityApi;
        HL::FNGetEntityAPI getEntityApiPost;
        HL::FNGetEntityAPI2 getEntityApi2;
        HL::FNGetEntityAPI2 getEntityApi2Post;
        HL::FNGetNewDLLFunctions getNewDLLFunctions;
        HL::FNGetNewDLLFunctions getNewDLLFunctionsPost;
        FNGetEngineFunctions getEngineFunctions;
        FNGetEngineFunctions getEngineFunctionsPost;
    };

    /// <summary>
    /// The information that the plugin provides
    /// </summary>
    struct PluginInfo {
        const char* interfaceVersion;
        const char* name;
        const char* version;
        const char* date;
        const char* author;
        const char* url;
        const char* logTag;
        LoadTime loadable;
        LoadTime unloadable;
    };

    /// <summary>
    /// The game library functions pair
    /// </summary>
    struct GameLibraryFunctions {
        HL::DLL_FUNCTIONS* originalFunctions;
        HL::NEW_DLL_FUNCTIONS* extendedFunctions;
    };
} }
