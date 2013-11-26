#pragma once

#include <GoldMeta/Meta/MetaDefs.hpp>

namespace HL {
    // Since this is a header file ('MetaDefs.hpp'), we don't want to include the HLSDK and we cannot forward
    // declare this structure either since it is passed by value (WHYYY Metamod!?), so we define it here...
    struct hudtextparms_t {
        float x;
        float y;
        int effect;
        unsigned char r1, g1, b1, a1;
        unsigned char r2, g2, b2, a2;
        float fadeInTime;
        float fadeOutTime;
        float holdTime;
        float effectTime;
        int channel;
    };

    // Forward declarations
    typedef struct enginefuncs_s enginefuncs_t;
    typedef struct entvars_s entvars_t;
    typedef struct edict_s edict_t;
    struct NEW_DLL_FUNCTIONS;
    struct DLL_FUNCTIONS;
}

namespace gm { namespace Meta {
    /// <summary>
    /// The Metamod API utility functions
    /// </summary>
    struct MetaAPI {
        void        (*LogConsole)           (PluginInfo* plugin, const char* format, ...);
        void        (*LogMessage)           (PluginInfo* plugin, const char* format, ...);
        void        (*LogError)             (PluginInfo* plugin, const char* format, ...);
        void        (*LogDeveloper)         (PluginInfo* plugin, const char* format, ...);
        void        (*CenterSay)            (PluginInfo* plugin, const char* format, ...);
        void        (*CenterSayParms)       (PluginInfo* plugin, HL::hudtextparms_t parms, const char* format, ...);
        void        (*CenterSayVarArgs)     (PluginInfo* plugin, HL::hudtextparms_t parms, const char* format, std::va_list va);
        int         (*CallGameEntity)       (PluginInfo* plugin, const char* entity, HL::entvars_t* pev);
        int         (*GetUserMessageId)     (PluginInfo* plugin, const char* message, int* size);
        const char* (*GetUserMessageName)   (PluginInfo* plugin, int index, int* size);
        const char* (*GetPluginPath)        (PluginInfo* plugin);
        const char* (*GetGameInfo)          (PluginInfo* plugin, GameInfo info);
        int         (*LoadPlugin)           (PluginInfo* plugin, const char* commandLine, LoadTime time, void** handle);
        int         (*UnloadPlugin)         (PluginInfo* plugin, const char* commandLine, LoadTime time, UnloadReason reason);
        int         (*UnloadPluginByHandle) (PluginInfo* plugin, void* handle, LoadTime time, UnloadReason reason);
        const char* (*IsQueryingClientCvar) (PluginInfo* plugin, const HL::edict_t* edict);
        int         (*MakeRequestId)        (PluginInfo* plugin);
        void        (*GetHookTables)        (PluginInfo* plugin, HL::enginefuncs_t** engineFunctions, HL::DLL_FUNCTIONS** originalLibraryFunctions, HL::NEW_DLL_FUNCTIONS** extendedLibraryFunctions);
    };
} }
