#pragma once

#include <GoldMeta/Shared.hpp>

namespace gm {
    // Forward declarations
    class IModuleFunction;
    class ConventionInfo;
    class HookContext;

    class IFunctionBase {
    public:
        /// <summary>
        /// Gets a plugin specific function module
        /// </summary>
        virtual IModuleFunction* GetModule(PluginId plugin) = 0;

        /// <summary>
        /// Gets a callable address to the function
        /// </summary>
        virtual void* GetCallableAddress() = 0;

        /// <summary>
        /// Removes a loaded function module
        /// </summary>
        virtual void RemoveModule(PluginId plugin) = 0;

        /// <summary>
        /// Calls the target function in a generic way
        /// </summary>
        virtual void Call(void* returnValue, const void* arguments[]) = 0;

        /// <summary>
        /// Gets the functions currently assigned convention info
        /// </summary>
        virtual const ConventionInfo& GetConventionInfo() = 0;

        /// <summary>
        /// This method gets called if a generated assembly results in an invalid ESP displacement
        /// </summary>
        virtual void InvalidESP() = 0;

        /// <summary>
        /// This method gets called when the assembly code is entered
        /// </summary>
        virtual HookContext* OnEntry() = 0;

        /// <summary>
        /// This method gets called when the assembly code is 'exited'
        /// </summary>
        virtual void OnExit() = 0;

        /// <summary>
        /// Iterates to the next module and returns it
        /// </summary>
        virtual IModuleFunction* IterateModule() = 0;

        /// <summary>
        /// Iterates to the next module and returns it
        /// </summary>
        virtual void ResetIterator() = 0;
    };
}
