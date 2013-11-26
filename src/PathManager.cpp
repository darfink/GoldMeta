#include <cassert>

#include "PathManager.hpp"

namespace gm { 
    PathManager::PathManager(const std::string& gameDirectory) {
        assert(gameDirectory.length() > 0);

        mPaths[ServerRoot]    = fs::initial_path<fs::path>();
        mPaths[GameRoot]      = mPaths[ServerRoot]   / gameDirectory;
        mPaths[GameLibraries] = mPaths[GameRoot]     / "dlls";
        mPaths[AddonsRoot]    = mPaths[GameRoot]     / "addons";
        mPaths[GoldMetaRoot]  = mPaths[AddonsRoot]   / "goldmeta";
        mPaths[GoldMetaLogs]  = mPaths[GoldMetaRoot] / "logs";
        mPaths[GoldMetaData]  = mPaths[GoldMetaRoot] / "data";

        for(auto& pair : mPaths) {
            fs::path& path = pair.second;

            switch(pair.first) {
                // All required paths
                case ServerRoot:
                case GameRoot:
                case GameLibraries:
                case AddonsRoot:
                case GoldMetaRoot:
                {
                    if(fs::exists(path)) {
                        break;
                    } else {
                         throw Exception(format("the system critical path '%s' does not exist") % path.relative_path());
                    }
                }

                // All optional paths
                case GoldMetaLogs:
                case GoldMetaData:
                {
                    // Attempt to create the path if it doesn't exist
                    if(!fs::exists(path) && !fs::create_directory(path)) {
                        throw Exception(format("the required path '%s' could not be created") % path.relative_path());
                    }

                    break;
                }
            }

            // Do some sane checks on the path
            assert(fs::is_directory(path));
            assert(path.is_absolute());
        }
    }

    std::string PathManager::GetPath(Path path, Type type) const {
        auto it = mPaths.find(path);
        assert(it != mPaths.end());

        const fs::path& fsPath = it->second;

        switch(type) {
            case Absolute: return fsPath.string();
            case Relative: return fsPath.relative_path().string();
            case Naked: return fsPath.filename().string();
            default: assert(false);
        }

        // For: "not all control paths return a value"
        return std::string();
    }

    const boost::filesystem::path& PathManager::GetPathObject(Path path) const {
        auto it = mPaths.find(path);
        assert(it != mPaths.end());

        return it->second;
    }
}