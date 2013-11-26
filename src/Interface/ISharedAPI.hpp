#pragma once

#include <GoldMeta/Gold/IGoldAPI.hpp>
#include <GoldMeta/Meta/MetaAPI.hpp>
#include <string>

namespace gm {
    class ISharedAPI : public IGoldAPI {
    public:
        /// <summary>
        /// Gets the meta API function table
        /// </summary>
        virtual const Meta::MetaAPI& GetMetaAPI() const = 0;
    };
}
