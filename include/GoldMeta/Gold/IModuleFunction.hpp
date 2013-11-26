#pragma once

#include <GoldMeta/Shared.hpp>

namespace gm {
    // We use a namespace since strong enums cannot be used as bit flags
    namespace Tense {
        /// <summary>
        /// Describes during which tense the listener is called (can be both pre and post)
        /// </summary>
        enum Type {
            Pre = (1 << 0),
            Post = (1 << 1),
        };
    }

    class IModuleFunction {
    public:
        /// <summary>
        /// Gets a callable address to the function
        /// </summary>
        virtual void* GetCallableAddress() = 0;

        /// <summary>
        /// Sets the current listener for this function module
        /// </summary>
        virtual void SetListener(void* callback, void* context, int tense) = 0;

        /// <summary>
        /// Sets the listener tense (can be both pre and post simultaneously)
        /// </summary>
        virtual void SetListenerTense(int tense) = 0;

        /// <summary>
        /// Disables (pauses) the current listener
        /// </summary>
        virtual void DisableListener() = 0;

        /// <summary>
        /// Enables (resumes) the current listener
        /// </summary>
        virtual void EnableListener() = 0;

        /// <summary>
        /// Calls the target function in a generic way
        /// </summary>
        virtual void Call(void* returnValue, const void* arguments[]) = 0;

        /// <summary>
        /// Releases the module function
        /// </summary>
        virtual void Release() = 0;

        /// <summary>
        /// Gets whether this module can be called or not
        /// </summary>
        virtual bool IsCallable(Tense::Type tense) = 0;

        /// <summary>
        /// Gets the module function callback
        /// </summary>
        virtual void* GetCallback() = 0;

        /// <summary>
        /// Gets the module function context
        /// </summary>
        virtual void* GetContext() = 0;
    };
}
