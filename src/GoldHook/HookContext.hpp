#pragma once

#include <GoldMeta/Gold/IHookContext.hpp>
#include <GoldMeta/Gold/IModuleFunction.hpp>

#include "../Interface/IFunctionBase.hpp"

namespace gm {
    class HookContext : public IHookContext {
    public:
        /// <summary>
        /// Constructs a hook context instance
        /// </summary>
        HookContext(IFunctionBase* function);

        /// <summary>
        /// Destructor for the hook context
        /// </summary>
        ~HookContext();

        /// <summary>
        /// Gets the current context (only valid for methods)
        /// </summary>
        virtual void* GetContext();

        /// <summary>
        /// Sets the value of a specific parameter (must return at least Result::Handled)
        /// </summary>
        virtual void SetParameter(unsigned int index, const void* value);

        /// <summary>
        /// Gets the associated module function
        /// </summary>
        virtual IModuleFunction* GetFunction();

        /// <summary>
        /// Gets the currently highest result
        /// </summary>
        virtual Result GetHighestResult();

        /// <summary>
        /// Gets the (if any) previous result
        /// </summary>
        virtual Result GetPreviousResult();

        /// <summary>
        /// Sets the current hook result
        /// </summary>
        virtual void SetResult(Result result);

        /// <summary>
        /// Gets the original return value (not valid for void functions)
        /// </summary>
        virtual void GetOriginalReturn(void* value);

        /// <summary>
        /// Gets the overridden return value (not valid for void functions)
        /// </summary>
        virtual void GetOverrideReturn(void* value);

        /// <summary>
        /// Checks whether the module is called in post or not
        /// </summary>
        virtual bool IsPost();

        /// <summary>
        /// Checks whether the module is called in pre or not
        /// </summary>
        virtual bool IsPre();

        /// <summary>
        /// Resets all member variables (used before assembly execution)
        /// </summary>
        void Reset();

        // Public members (these are public so they can be accessed from the assembly code)
        Result currentResult;
        Result previousResult;
        Result highestResult;
        byte* originalReturn;
        byte* overrideReturn;
        byte* currentReturn;
        void* callerAddress;
        void* calleeContext;
        IModuleFunction* module;
        Tense::Type tense;

    private:
        // Private members
        IFunctionBase* mFunctionBase;
        size_t mReturnSize;
        bool mHiddenReturn;
    };
}
