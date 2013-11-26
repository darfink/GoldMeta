#include <GoldMeta/Gold/IHookContext.hpp>
#include <boost/program_options.hpp>
#include <cassert>
#include <cstdarg>

#include "MetaMain.hpp"
#include "GoldHook.hpp"
#include "PathManager.hpp"
#include "GameLibrary.hpp"
#include "PluginManager.hpp"
#include "OS/SignatureScanner.hpp"

// We use a short hand namespace for this
namespace po = boost::program_options;

namespace gm {
    MetaMain::MetaMain() :
        mEngineFunctions(nullptr),
        mEngineGlobals(nullptr)
    {
        // NOTE: Do as little as possible here since it is called from 'DLLMain' on Windows
    }

    // NOTE: The order of 'va_arg' is critical!
    bool MetaMain::ExportAPI(ExportType type, ...) {
        std::va_list arguments;
        va_start(arguments, type);

        bool result = false;

        switch(type) {
            case ExportType::GetFuntionPointersFromEngine: {
                mEngineFunctions = va_arg(arguments, HL::enginefuncs_t*);
                mEngineGlobals = va_arg(arguments, HL::globalvars_t*);

                assert(mEngineFunctions != nullptr);
                assert(mEngineGlobals != nullptr);

                // Since this API function is the first one called, we do all of our initializing when
                // it is called to setup our critical components (e.g game library, plugin manager)
                this->Initialize();
                result = true;

                break;
            }

            case ExportType::GiveEntityAPI: {
                // Retrieve the library functions and interface version
                auto libraryFunctions = va_arg(arguments, HL::DLL_FUNCTIONS*);
                int* interfaceVersion = va_arg(arguments, int*);

                assert(libraryFunctions != nullptr);
                assert(interfaceVersion != nullptr);

                result = !!mGameLibrary->GetEntityAPI(libraryFunctions, interfaceVersion);
                
                if(result != 0) {
                    std::cout << format("[INFO] Successfully initialized game '%s'\n") % mGameLibrary->GetGameDescription();
                }

                break;
            }

            case ExportType::GiveExtendedEntityAPI: {
                auto libraryFunctions = va_arg(arguments, HL::NEW_DLL_FUNCTIONS*);
                int* interfaceVersion = va_arg(arguments, int*);

                assert(libraryFunctions != nullptr);
                assert(interfaceVersion != nullptr);

                if(mGameLibrary->IsUsingExtendedEntityAPI()) {
                    // If the game library is utilizing the extended API, forward the call
                    result = !!mGameLibrary->GetNewDLLFunctions(libraryFunctions, interfaceVersion);
                    std::cout << "[INFO] The game is utilizing the extended entity API\n";
                } else {
                    result = true;
                }

                break;
            }

            case ExportType::GiveBlendingInterface: {
                // The 'va_arg' define cannot have parentheses in the second argument
                typedef float(*BoneTransform)[128][3][4];
                typedef float(*RotationMatrix)[3][4];

                int version            = va_arg(arguments, int);
                auto blendingInterface = va_arg(arguments, HL::sv_blending_interface_t**);
                auto engineStudioApi   = va_arg(arguments, HL::engine_studio_api_t*);
                auto rotationMatrix    = va_arg(arguments, RotationMatrix);
                auto boneTransform     = va_arg(arguments, BoneTransform);

                assert(blendingInterface != nullptr);
                assert(engineStudioApi != nullptr);
                assert(rotationMatrix != nullptr);
                assert(boneTransform != nullptr);

                if(mGameLibrary->IsUsingBlendingInterface()) {
                    // In case the game library is using a custom blending interface, forward this call to to the library
                    result = !!mGameLibrary->GetBlendingInterface(version, blendingInterface, engineStudioApi, rotationMatrix, boneTransform);
                    std::cout << "[INFO] The game is using custom hitbox morphology\n";
                } else {
                    // To be explicit; we return false so the engine knows that we aren't using a custom blending interface
                    result = false;
                }

                // This export call is the last one called by the engine, so after this we apply our entity
                // hook so we avoid hooking symbol calls searching for "GetEntityAPI" or "GetNewDLLFunctions".
                //this->SetupEntityHook();

                SignatureScanner ss(*reinterpret_cast<uintptr_t*>(mEngineFunctions));
                uintptr_t lol = ss.FindSignature({ 0x33, 0xFF, 0x85, 0xC0, 0x7E, 0x00, 0x8B, 0x5D, 0x00, 0xBE }, "xxxxx?xx?x", -11);

                IModuleFunction* func = mGoldHook->GetFunction(PluginId(1), "FunctionFromName", reinterpret_cast<void*>(lol));
                func->SetListener(brute_cast<void*>(&MetaMain::EntityHook), this, Tense::Pre);

                SignatureScanner sa(reinterpret_cast<uintptr_t>(*blendingInterface));
                //uintptr_t loa = sa.FindSignature({ 0x81, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x55, 0x56 }, "xx????xx?????xxx");
                //uintptr_t loa = sa.FindSignature({ 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x8b, 0x44, 0x24, 0x70, 0x83 }, "xx????xxxxx", 2);
                uintptr_t loa = sa.FindSignature({ 0x53, 0x55, 0x89, 0x44, 0x00, 0x00, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x56 }, "xxxx??x????x", -13);

                struct Vector { float x, y, z; };
                Vector(__stdcall*asda)(IHookContext*, Vector, Vector, float, float, int, int, int, float, void*, bool, int) = [](
                    IHookContext* context,
                    Vector source,
                    Vector direction,
                    float spread,
                    float distance,
                    int penetration,
                    int bulletType,
                    int damage,
                    float rangeModifier,
                    void* attacker,
                    bool isPistol,
                    int sharedRandom) -> Vector {
                    context->SetResult(Result::Ignored);
                    return source;
                };

                // TODO: Check return value when override and supersede
                IModuleFunction* funa = mGoldHook->GetFunction(PluginId(1), "FireBullets3", reinterpret_cast<void*>(loa));
                funa->SetListener(brute_cast<void*>(asda), this, Tense::Pre);

                break;
            }
        }

        va_end(arguments);
        return result;
    }

    void MetaMain::Initialize() {
        // We need to parse some information from the command line
        po::options_description description("GoldMeta and HLDS options");
        po::variables_map vm;

        description.add_options()
            ("game", po::value<std::string>()->default_value("valve"), "specify the current Half-Life mod")
            ("dev", "set whether developer is enabled or not");

        po::store(po::command_line_parser(GetCommandLineArguments())
            .options(description)
            .style(po::command_line_style::default_style |
                   po::command_line_style::case_insensitive |
                   po::command_line_style::allow_long_disguise)
            .allow_unregistered()
            .run(), vm);

        try { /* Any one of our initialization constructors may throw */
            mPathManager.reset(new PathManager(vm["game"].as<std::string>()));
            mGameLibrary.reset(new GameLibrary(mPathManager));
            mPluginManager.reset(new PluginManager(mPathManager));
            mGoldHook.reset(new GoldHook(mPathManager));
        } catch(const PathManager::Exception& ex) {
            std::cerr << "[FATAL] Path manager initialization failed; " << ex.what() << std::endl;
            throw Exception("couldn't initialize path manager");
        } catch(const GameLibrary::Exception& ex) {
            std::cerr << "[FATAL] Game library initialization failed; " << ex.what() << std::endl;
            throw Exception("couldn't initialize game library");
        } catch(const PluginManager::Exception& ex) {
            std::cerr << "[FATAL] Plugin manager initialization failed; " << ex.what() << std::endl;
            throw Exception("couldn't initialize plugin manager");
        } catch(const GoldHook::Exception& ex) {
            std::cerr << "[FATAL] GoldHook initialization failed; " << ex.what() << std::endl;
            throw Exception("couldn't initialize GoldHook");
        }

        // We just give the function pointers (nothing more) to the game library, before loading
        // all plugins, since we don't want to load them too late so they cannot intercept all calls
        mGameLibrary->GiveFnptrsToDll(mEngineFunctions, mEngineGlobals);

        // Read and load all plugins from the config
        mPluginManager->LoadConfigPlugins();
    }

    // TODO: Add support for plugins to allocate data for each entity by using the following approach;
    // whenever the game library is exporting an entity (e.g 'env_message'), the library will call
    // 'PvAllocEntPrivateData'. If we place a hook on this function, and save the current entity
    // that is being exported, we can allow plugins to add data to each entity by increasing the
    // memory allocated, and append a GoldMeta memory manager. So each player entity can have
    // custom user data, such as the number of points if the plugin uses a "point system".
    HL::FNEntity MetaMain::EntityHook(IHookContext* hookContext, const char* szSymbol) {
        // Save all requested entities
        std::string symbol(szSymbol);
        mEntitySymbols.insert(symbol);

        // Before we check the game engine for the exported entity, we query the plugins first so they have the possibility to
        // override the game entity. Beyond this some plugins also define custom entities, that we must enable support for.
        HL::FNEntity result = mPluginManager->GetEntity(symbol);

        if(result == nullptr) {
            // If it is not found within any plugin, query the game engine for the entity
            if(!(result = mGameLibrary->GetEntity(symbol))) {
                std::cerr << format("[WARNING] Could not find entity API for '%s'\n") % symbol;
            }
        }

        hookContext->SetResult(Result::Supersede);
        return result;
    }

    // Define the global meta instance
    MetaMain* gMetaMain = nullptr;
}