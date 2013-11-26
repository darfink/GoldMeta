#pragma once

#include <GoldMeta/Gold/IModuleFunction.hpp>
#include <GoldMeta/Shared.hpp>

namespace gm {
    // Forward declarations
    class IFunctionBase;

    class ModuleFunction : public IModuleFunction {
    public:
        /// <summary>
        /// Constructs a module function instance
        /// </summary>
        ModuleFunction(PluginId id, IFunctionBase* functionBase);

        /// <summary>
        /// Gets a callable address to the function
        /// </summary>
        virtual void* GetCallableAddress();

        /// <summary>
        /// Sets the current listener for this function module
        /// </summary>
        virtual void SetListener(void* function, void* context, int tense);

        /// <summary>
        /// Sets the listener tense (can be both pre and post simultaneously)
        /// </summary>
        virtual void SetListenerTense(int tense);

        /// <summary>
        /// Disables (pauses) the current listener
        /// </summary>
        virtual void DisableListener();

        /// <summary>
        /// Enables (resumes) the current listener
        /// </summary>
        virtual void EnableListener();

        /// <summary>
        /// Calls the target function in a generic way
        /// </summary>
        virtual void Call(void* returnValue, const void* arguments[]);

        /// <summary>
        /// Releases the module function
        /// </summary>
        virtual void Release();

        /// <summary>
        /// Gets whether this module can be called or not
        /// </summary>
        virtual bool IsCallable(Tense::Type tense);

        /// <summary>
        /// Gets the module function callback
        /// </summary>
        virtual void* GetCallback();

        /// <summary>
        /// Gets the module function context
        /// </summary>
        virtual void* GetContext();

    private:
        // Private members
        IFunctionBase* mFunctionBase;
        PluginId mPluginId;
        void* mCallback;
        void* mContext;
        bool mDisabled;
        int mTense;
    };
}
