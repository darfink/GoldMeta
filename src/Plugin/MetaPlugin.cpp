#include <GoldMeta/Meta/MetaDefs.hpp>
#include <cassert>

#include "MetaPlugin.hpp"

namespace gm {
    MetaPlugin::MetaPlugin(PluginId id, const fs::path& path) :
        PluginBase(id, path),
        mGiveFnptrsToDll(nullptr),
        mMetaInit(nullptr),
        mMetaAttach(nullptr),
        mMetaDetach(nullptr),
        mMetaQuery(nullptr)
    {
        try {
            mGiveFnptrsToDll = reinterpret_cast<HL::FNGiveFnptrsToDll>(mLibrary.GetSymbol("GiveFnptrsToDll"));
            mMetaQuery       = reinterpret_cast<Meta::FNMetaQuery>    (mLibrary.GetSymbol("Meta_Query"));
            mMetaAttach      = reinterpret_cast<Meta::FNMetaAttach>   (mLibrary.GetSymbol("Meta_Attach"));
            mMetaDetach      = reinterpret_cast<Meta::FNMetaDetach>   (mLibrary.GetSymbol("Meta_Detach"));
            mMetaInit        = reinterpret_cast<Meta::FNMetaInit>     (mLibrary.GetSymbol("Meta_Init", false));
        } catch(const Library::SymbolNotFound& ex) {
            std::cerr << "[WARNING] Export function not found; " << ex.what() << std::endl;
            throw Exception("couldn't find plugin export functions");
        }
    }

    void MetaPlugin::Load() {
    }

    void MetaPlugin::Unload() {
    }

    PluginBase::Type MetaPlugin::GetType() const {
        return PluginBase::Metamod;
    }
}