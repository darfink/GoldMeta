#pragma once

#include "OS/OS.hpp"

namespace HL {
    // Forward declarations
    typedef struct enginefuncs_s enginefuncs_t;
    typedef struct globalvars_s globalvars_t;
    typedef struct new_dll_functions_s NEW_DLL_FUNCTIONS;
    typedef struct dll_functions_s DLL_FUNCTIONS;
    typedef struct sv_blending_interface_s sv_blending_interface_t;
    typedef struct engine_studio_api_s engine_studio_api_t;
}

/// <summary>
///
/// </summary>
enum class ExportType {
    GetFuntionPointersFromEngine,
    GiveEntityAPI,
    GiveExtendedEntityAPI,
    GiveBlendingInterface,
};

/// <summary>
/// 
/// </summary>
GM_EXPORT void WINAPI GiveFnptrsToDll(HL::enginefuncs_t* engineFunctions, HL::globalvars_t* engineGlobals);

/// <summary>
/// 
/// </summary>
GM_EXPORT int GetEntityAPI2(HL::DLL_FUNCTIONS* libraryFunctions, int* interfaceVersion);

/// <summary>
/// 
/// </summary>
GM_EXPORT int GetEntityAPI(HL::DLL_FUNCTIONS* libraryFunctions, int interfaceVersion);

/// <summary>
///
/// </summary>
GM_EXPORT int GetNewDLLFunctions(HL::NEW_DLL_FUNCTIONS* libraryFunctions, int* interfaceVersion);

/// <summary>
///
/// </summary>
GM_EXPORT int Server_GetBlendingInterface(int version,
    HL::sv_blending_interface_t** blendingInterface,
    HL::engine_studio_api_t* engineStudioApi,
    float (*rotationmatrix)[3][4],
    float (*bonetransform)[128][3][4]);
