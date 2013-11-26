#pragma once

#include <functional>
#include <string>
#include <memory>

#include "Exception.hpp"
#include "PathManager.hpp"
#include "OS/Library.hpp"
#include "Interface/IEntityExporter.hpp"

namespace HL {
    // Forward declarations
    typedef struct enginefuncs_s enginefuncs_t;
    typedef struct globalvars_s globalvars_t;
    typedef struct new_dll_functions_s NEW_DLL_FUNCTIONS;
    typedef struct dll_functions_s DLL_FUNCTIONS;
    typedef struct sv_blending_interface_s sv_blending_interface_t;
    typedef struct engine_studio_api_s engine_studio_api_t;
}

namespace /* Anonymous */ {
    namespace fs = boost::filesystem;
}

namespace gm {
    class GameLibrary : public IEntityExporter {
    public:
        /// <summary>
        /// The exception class that the game library instance throws
        /// </summary>
        GM_DEFINE_EXCEPTION(Exception);

        /// <summary>
        /// Constructs a game library instance
        /// </summary>
        GameLibrary(std::shared_ptr<PathManager> pathManager);

        /// <summary>
        /// Gets an entity object residing in the game library
        /// </summary>
        virtual HL::FNEntity GetEntity(const std::string& entity);

        /// <summary>
        /// Gets the path to the game library file
        /// </summary>
        const std::string& GetPath() const;

        /// <summary>
        /// Gets the game description
        /// </summary>
        const std::string& GetGameDescription() const;

        /// <summary>
        /// Gets whether the game is using the extended DLL functions
        /// </summary>
        bool IsUsingExtendedEntityAPI() const;

        /// <summary>
        /// Gets whether the game is using a custom blending interface
        /// </summary>
        bool IsUsingBlendingInterface() const;

        /// <summary>
        /// Calls the game library's 'GiveFnptrsToDll' function
        /// </summary>
        void GiveFnptrsToDll(HL::enginefuncs_t* engineFunctions, HL::globalvars_t* engineGlobals);

        /// <summary>
        /// Calls the game library's 'GetEntityAPI' function
        /// </summary>
        int GetEntityAPI(HL::DLL_FUNCTIONS* libraryFunctions, int* version);

        /// <summary>
        /// Calls the game library's 'GetNewDLLFunctions' function
        /// </summary>
        int GetNewDLLFunctions(HL::NEW_DLL_FUNCTIONS* libraryFunctions, int* version);

        /// <summary>
        /// Calls the game library's 'Server_GetBlendingInterface' function
        /// </summary>
        int GetBlendingInterface(int version, HL::sv_blending_interface_t** blendingInterface, HL::engine_studio_api_t* engineStudioApi, float (*rotationMatrix)[3][4], float (*boneTransform)[128][3][4]);

    private:
        // Private members
        std::function<int(int, HL::sv_blending_interface_t**, HL::engine_studio_api_t*, float(*)[3][4], float(*)[128][3][4])> mGetBlendingInterface;
        std::function<void(HL::enginefuncs_t*, HL::globalvars_t*)> mGiveFnptrsToDll;
        std::function<int(HL::NEW_DLL_FUNCTIONS*, int*)> mGetNewDLLFunctions;
        std::function<int(HL::DLL_FUNCTIONS*, int*)> mGetEntityAPI;
        std::shared_ptr<PathManager> mPathManager;
        std::string mGameDescription;
        Library mLibrary;
    };
}
