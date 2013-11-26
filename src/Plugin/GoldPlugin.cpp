#include <GoldMeta/Gold/IGoldPlugin.hpp>
#include <cassert>

#include "GoldPlugin.hpp"

namespace gm {
    GoldPlugin::GoldPlugin(PluginId id, const fs::path& path) :
        PluginBase(id, path),
        mGetInstance(nullptr),
        mPlugin(nullptr)
    {
        try {
            // If we loaded it successfully, check the export table for the GoldMeta 'GetPluginInstance' function
            mGetInstance = reinterpret_cast<FNGetPluginInstance>(mLibrary.GetSymbol(GetPluginInstanceSymbol));
        } catch(const Library::SymbolNotFound& ex) {
            std::cerr << "[WARNING] Export function not found; " << ex.what() << std::endl;
            throw Exception("couldn't find plugin export function");
        }
    }

    void GoldPlugin::Load() {
        assert(mGetInstance != nullptr);
        assert(mPlugin == nullptr);

        // Retrieve the plugin instance
        mPlugin = mGetInstance();

        if(mPlugin == nullptr) {
            throw Exception("received invalid plugin instance");
        } else if(!mPlugin->Load(this->GetID(), nullptr)) {
            const char* error = mPlugin->GetLastError();
            throw Exception(format("couldn't load plugin: %s") % ((error == nullptr) ? "unknown error" : error));
        }
    }

    void GoldPlugin::Unload() {
    }

    PluginBase::Type GoldPlugin::GetType() const {
        return PluginBase::Goldmeta;
    }
}