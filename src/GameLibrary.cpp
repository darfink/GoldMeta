#include <GoldMeta/Shared.hpp>
#include <boost/filesystem.hpp>
#include <boost/range.hpp>

#include "GameLibrary.hpp"
#include "PathManager.hpp"
#include "HLSDK.hpp"

namespace gm {
    GameLibrary::GameLibrary(std::shared_ptr<PathManager> pathManager) :
        mPathManager(pathManager)
    {
        // Retrieve the path object to the game library folder (e.g 'hlds/cstrike/dlls')
        const fs::path& libraries = mPathManager->GetPathObject(PathManager::GameLibraries);

        if(!fs::exists(libraries)) {
            throw Exception(format("the game library folder does not exist: %s") % libraries);
        }

        // We want to visit sub-directories as well, so we use a recursive iterator
        fs::recursive_directory_iterator it(libraries);
        fs::recursive_directory_iterator end;

        // Since we cannot know which specific game library this mod uses,
        // we iterate over each file in the 'dlls' folder and check whether
        // it is a dynamic library or not. If it is we attempt to load the
        // library and check for all required API functions. If they are all
        // found in the library, we assume that it is the correct library.
        for(const fs::path& path : boost::make_iterator_range(it, end)) {
            if(!fs::is_regular_file(path)) {
                continue;
            }

            // Check whether this file object is a library or not
            if(path.extension() == Library::GetExtension()) {
                try /* Attempt to load the library */ {
                    mLibrary.Load(path.string());

                    // The only exports required are 'GiveFnptrsToDll' and either one of the entity APIs
                    mGiveFnptrsToDll      = brute_cast<HL::FNGiveFnptrsToDll>     (mLibrary.GetSymbol("GiveFnptrsToDll"));
                    mGetBlendingInterface = brute_cast<HL::FNGetBlendingInterface>(mLibrary.GetSymbol("Server_GetBlendingInterface", false));
                    mGetNewDLLFunctions   = brute_cast<HL::FNGetNewDLLFunctions>  (mLibrary.GetSymbol("GetNewDLLFunctions", false));
                    mGetEntityAPI         = brute_cast<HL::FNGetEntityAPI2>       (mLibrary.GetSymbol("GetEntityAPI2", false));

                    if(!mGetEntityAPI) {
                        // We need to cache this address for the lambda so it can capture it
                        HL::FNGetEntityAPI fnGetEntityApi = reinterpret_cast<HL::FNGetEntityAPI>(mLibrary.GetSymbol("GetEntityAPI"));

                        // Since the obsolete version of 'GetEntityAPI' doesn't take a pointer for the version,
                        // argument we use a lambda for the compatibility conversion (i.e we can't feed it a pointer)
                        mGetEntityAPI = [fnGetEntityApi](HL::DLL_FUNCTIONS* libraryFunctions, int* version) {
                            return fnGetEntityApi(libraryFunctions, *version);
                        };
                    }

                    // If we have reached to this point, we have found a valid game library!
                    std::cout << format("[INFO] Loading game library %s\n") % path;
                    break;
                } catch(const Library::LibraryNotLoaded& ex) {
                    std::cerr << format("[WARNING] Load error for library %s: %s\n") % path % ex.what();
                } catch(const Library::SymbolNotFound& ex) {
                    std::cerr << format("[WARNING] Symbol error for library %s: %s\n") % path % ex.what();
                }

                // In case we failed, unload
                mLibrary.Unload();
            }
        }

        if(!mLibrary.IsLoaded()) {
            throw Exception("couldn't find any game library to load");
        }
    }

    HL::FNEntity GameLibrary::GetEntity(const std::string& symbol) {
        assert(symbol.length() > 0);
        return reinterpret_cast<HL::FNEntity>(mLibrary.GetSymbol(symbol, false));
    }

    const std::string& GameLibrary::GetPath() const {
        return mLibrary.GetPath();
    }

    const std::string& GameLibrary::GetGameDescription() const {
        return mGameDescription;
    }

    bool GameLibrary::IsUsingExtendedEntityAPI() const {
        return !!mGetNewDLLFunctions;
    }

    bool GameLibrary::IsUsingBlendingInterface() const {
        return !!mGetBlendingInterface;
    }

    void GameLibrary::GiveFnptrsToDll(HL::enginefuncs_t* engineFunctions, HL::globalvars_t* engineGlobals) {
        mGiveFnptrsToDll(engineFunctions, engineGlobals);
    }

    int GameLibrary::GetNewDLLFunctions(HL::NEW_DLL_FUNCTIONS* libraryFunctions, int* version) {
        return mGetNewDLLFunctions(libraryFunctions, version);
    }

    int GameLibrary::GetEntityAPI(HL::DLL_FUNCTIONS* libraryFunctions, int* version) {
        int result = mGetEntityAPI(libraryFunctions, version);

        if(result != 0) {
            // Retrieve the game description and store it in our own string
            mGameDescription.assign(libraryFunctions->pfnGetGameDescription());
        }

        return result;
    }

    int GameLibrary::GetBlendingInterface(int version,
        HL::sv_blending_interface_t** blendingInterface,
        HL::engine_studio_api_t* engineStudioApi,
        float (*rotationMatrix)[3][4],
        float (*boneTransform)[128][3][4])
    {
        return mGetBlendingInterface(version, blendingInterface, engineStudioApi, rotationMatrix, boneTransform);
    }
}
