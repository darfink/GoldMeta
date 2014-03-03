#include <boost/range.hpp>
//#include <unordered_map> TODO: Fixed enums and unordered maps hash
#include <map>
#include <functional>
#include <fstream>
#include <cstring>

#include "GoldHook.hpp"
#include "OS/OS.hpp"

namespace gm {
    GoldHook::GoldHook(std::shared_ptr<PathManager> pathManager, std::string /*dbFile*/) :
        mPathManager(pathManager)
    {
        /*fs::path dbPath = mPathManager->GetPathObject(PathManager::GoldMetaData) / dbFile;

        if(!fs::exists(dbPath)) {
            std::cerr << "[ERROR] The database does not exist: " << dbPath << std::endl;
            throw Exception("the database does not exist");
        }

        try {
            mDatabase->Open(dbPath.native().c_str());
        } catch(const SQLite::Exception& ex) {
            std::cerr << format("[ERROR] The database could not be opened: %s\n") % ex.what();
            throw Exception("couldn't open the database");
        }

        // We do this automatically at startup
        this->LoadCustomTypes();*/
     }

    IModuleFunction* GoldHook::GetFunction(PluginId id, const char* name, void* add) {
        if(name == nullptr || std::strlen(name) == 0) {
            std::cerr << "[WARNING] A plugin called 'GetFunction' with an empty name\n";
            return nullptr;
        }

        if(strcmp(name, "FunctionFromName") == 0) {
            mStaticFunctions[name] = std::make_shared<StaticFunction>(name, ConventionInfo(CallingConvention::CDecl, DataType::FromType<void*>(), { DataType::FromType<const char*>() }), add);
        } else {
            DataType retType;
            retType.SetType(DataType::Structure);
            retType.SetSize(12);

            mStaticFunctions[name] = std::make_shared<StaticFunction>(name, ConventionInfo(CallingConvention::Thiscall, /*retType*/DataType::FromType<double>(), { DataType::FromType<double>(), retType, retType, DataType::FromType<float>(), DataType::FromType<float>(), DataType::FromType<int>(), DataType::FromType<int>(), DataType::FromType<int>(), DataType::FromType<float>(), DataType::FromType<void*>(), DataType::FromType<bool>(), DataType::FromType<int>() }), add);
        }

        return mStaticFunctions.find(name)->second->GetModule(id);
        /*
        IModuleFunction* result = nullptr;

        if(result == nullptr) {
            // It is not cached, so attempt to load it
            result = this->LoadFunction(name);
        }

        return result;*/
    }

    IModuleFunction* GoldHook::GetEngineFunction(PluginId id, EngineAPI function) {
        static const std::map<EngineAPI, std::string> mapping = {
            { EngineAPI::PrecacheModel, "PrecacheModel" },
            { EngineAPI::PrecacheSound, "PrecacheSound" },
            { EngineAPI::SetModel,      "SetModel"      },
            { EngineAPI::ModelIndex,    "ModelIndex"    },
            { EngineAPI::ModelFrames,   "ModelFrames"   },
            { EngineAPI::SetSize,       "SetSize"       },
            { EngineAPI::ChangeLevel,   "ChangeLevel"   },
        };

        return nullptr;
    }

    void GoldHook::LoadCustomTypes() {
        /*SQLite::Statement statement(mDatabase.get());
        uint count = 0;

        try {
            statement.Sql("SELECT name, size FROM types");

            while(statement.FetchRow()) {
                DataType dataType;

                dataType.SetType(DataType::Structure);
                dataType.SetSize(statement.GetColumnInt("size"));

                // Add the data type to our collection (with overwrite)
                mTypes[statement.GetColumnString("name")] = dataType;
                count++;
            }
        } catch(const SQLite::Exception& ex) {
            std::cerr << format("[ERROR] Could not load custom types: %s\n") << ex.what();
            throw Exception("couldn't execute SQL statement");
        }

        std::cout << format("[INFO] Loaded %d custom data types\n") % count;*/
    }

    IModuleFunction* GoldHook::LoadFunction(const std::string& name) {
        /*SQLite::Statement statement(mDatabase.get());

        try {
            statement.Sql(str(format(
                "SELECT fns.ROWID, fns.type, fns.returns, fns.arguments, fns.memory, ids.value, ids.offset "
                "FROM functions fns "
                "JOIN identifiers ids ON fns.ROWID = ids.function "
                "WHERE fns.name = '%s' AND fns.class IS NULL AND ids.os & %d LIMIT 1") % name % static_cast<int>(GetCurrentOS())));

            if(statement.FetchRow()) {
                return nullptr;
            } else {
                throw Exception(format("the function '%s' could not be found") % name);
            }
        } catch(const SQLite::Exception& ex) {
            std::cerr << format("[ERROR] Could not load function: %s\n") % ex.what();
            throw Exception("couldn't execute SQL statement");
        }*/
        return nullptr;
    }
}
