#pragma once

#include <GoldMeta/Shared.hpp>

namespace gm {
    // Forward declarations
    class IModuleFunction;

    class IHookContext {
    public:
        /// <summary>
        /// Gets the current context (only valid for methods)
        /// </summary>
        virtual void* GetContext() = 0;

        /// <summary>
        /// Sets the value of a specific parameter (must return at least Result::Handled)
        /// </summary>
        virtual void SetParameter(unsigned int index, const void* value) = 0;

        /// <summary>
        /// Gets the associated module function
        /// </summary>
        virtual IModuleFunction* GetFunction() = 0;

        /// <summary>
        /// Gets the currently highest result
        /// </summary>
        virtual Result GetHighestResult() = 0;

        /// <summary>
        /// Gets the (if any) previous result
        /// </summary>
        virtual Result GetPreviousResult() = 0;

        /// <summary>
        /// Sets the current hook result
        /// </summary>
        virtual void SetResult(Result result) = 0;

        /// <summary>
        /// Gets the original return value (not valid for void functions)
        /// </summary>
        virtual void GetOriginalReturn(void* value) = 0;

        /// <summary>
        /// Gets the overridden return value (not valid for void functions)
        /// </summary>
        virtual void GetOverrideReturn(void* value) = 0;

        /// <summary>
        /// Checks whether the module is called in post or not
        /// </summary>
        virtual bool IsPost() = 0;

        /// <summary>
        /// Checks whether the module is called in pre or not
        /// </summary>
        virtual bool IsPre() = 0;
    };
}
