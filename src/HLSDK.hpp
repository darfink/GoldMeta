#pragma once

namespace HL {
    // Enclose everything in the Half-Life namespace
    #include <extdll.h>
    #include <edict.h>
    #include <eiface.h>
    #include <r_studioint.h>

    struct UserMessage {
        int index;         // 0x0000
        int size;          // 0x0004
        char name[16];     // 0x0008
        UserMessage* prev; // 0x0018
    };

    struct Command {
        Command* next;     // 0x0000
        const char* name;  // 0x0004
        void(*callback)(); // 0x0008
        int unknown;       // 0x000C    (0 = normal command, 1 = HUD command, 2 = game command, 4 = wrapper command)
    };

    /// <summary>
    /// The entity exporter function type
    /// </summary>
    typedef void*(__cdecl *FNEntity)(HL::entvars_t*);

    typedef void(WINAPI *FNGiveFnptrsToDll)(HL::enginefuncs_t*, HL::globalvars_t*);
    typedef int(__cdecl *FNGetBlendingInterface)(int, HL::sv_blending_interface_t**, HL::engine_studio_api_t*, float(*)[3][4], float(*)[128][3][4]);
    typedef int(__cdecl *FNGetNewDLLFunctions)(HL::NEW_DLL_FUNCTIONS*, int*);
    typedef int(__cdecl *FNGetEntityAPI2)(HL::DLL_FUNCTIONS*, int*);
    typedef int(__cdecl *FNGetEntityAPI)(HL::DLL_FUNCTIONS*, int);
}
