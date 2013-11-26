#include <iostream>

#include "HLExport.hpp"
#include "MetaMain.hpp"

void WINAPI GiveFnptrsToDll(HL::enginefuncs_t* engineFunctions, HL::globalvars_t* engineGlobals) {
    // We cannot return the result from this function, so instead we must use a standard call to 'exit'
    bool result = gm::gMetaMain->ExportAPI(ExportType::GetFuntionPointersFromEngine, engineFunctions, engineGlobals);

    if(!result) {
        std::cerr << "[FATAL] Failed to initialize GoldMeta" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

int GetEntityAPI2(HL::DLL_FUNCTIONS* libraryFunctions, int* interfaceVersion) {
    return gm::gMetaMain->ExportAPI(ExportType::GiveEntityAPI, libraryFunctions, interfaceVersion);
}

int GetEntityAPI(HL::DLL_FUNCTIONS* libraryFunctions, int interfaceVersion) {
    return gm::gMetaMain->ExportAPI(ExportType::GiveEntityAPI, libraryFunctions, &interfaceVersion);
}

int GetNewDLLFunctions(HL::NEW_DLL_FUNCTIONS* libraryFunctions, int* interfaceVersion) {
    return gm::gMetaMain->ExportAPI(ExportType::GiveExtendedEntityAPI, libraryFunctions, interfaceVersion);
}
int Server_GetBlendingInterface(int version, HL::sv_blending_interface_t** blendingInterface, HL::engine_studio_api_t* engineStudioApi, float (*rotationMatrix)[3][4], float (*boneTransform)[128][3][4]) {
    return gm::gMetaMain->ExportAPI(ExportType::GiveBlendingInterface, version, blendingInterface, engineStudioApi, rotationMatrix, boneTransform);
}