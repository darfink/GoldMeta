#include <cassert>

#include "PluginBase.hpp"

namespace gm {
    PluginBase::PluginBase(PluginId id, const fs::path& path) :
        mLastModified(fs::last_write_time(path)),
        mPluginId(id)
    {
        assert(fs::exists(path));
        assert(!(id == 0));

        try {
            // Attempt to load the dynamic library
            mLibrary.Load(path.string());
        } catch(const Library::LibraryNotLoaded& ex) {
            std::cerr << "[ERROR] Library load failure: " << ex.what() << std::endl;
            throw Exception("couldn't load the plugin library");
        }
    }

    const std::string& PluginBase::GetPath() const {
        return mLibrary.GetPath();
    }

    std::time_t PluginBase::GetLastModified() const {
        return mLastModified;
    }

    PluginId PluginBase::GetID() const {
        return mPluginId;
    }

    HL::FNEntity PluginBase::GetEntity(const std::string& entity) {
        // Simply just check if the library exports this specific entity
        return static_cast<HL::FNEntity>(mLibrary.GetSymbol(entity, false));
    }
}