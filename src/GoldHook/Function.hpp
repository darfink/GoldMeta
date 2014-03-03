#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>

#include "CodeGenerator.hpp"
#include "ConventionInfo.hpp"
#include "../Interface/IFunctionBase.hpp"

namespace gm {
    // Forward declarations
    class HookContext;
    class ModuleFunction;

    class Function : public IFunctionBase {
    public:
        /// <summary>
        /// The base exception class for functions
        /// </summary>
        GM_DEFINE_EXCEPTION(Exception);

        /// <summary>
        /// The virtual destructor for this class
        /// </summary>
        virtual ~Function();

        /// <summary>
        /// Gets a plugin specific function module
        /// </summary>
        virtual IModuleFunction* GetModule(PluginId plugin) final;

        /// <summary>
        /// Removes a loaded function module
        /// </summary>
        virtual void RemoveModule(PluginId plugin) final;

        /// <summary>
        /// Calls the target function in a generic way
        /// </summary>
        virtual void Call(void* returnValue, const void* arguments[]) final;

        /// <summary>
        /// Gets the functions currently assigned convention info
        /// </summary>
        virtual const ConventionInfo& GetConventionInfo() final;

    protected:
        /// <summary>
        /// Constructs a function instance object
        /// </summary>
        Function(std::string name, ConventionInfo cinfo);

    private:
        /// <summary>
        /// This method gets called if a generated assembly results in an invalid ESP displacement
        /// </summary>
        virtual void InvalidESP() final;

        /// <summary>
        /// This method gets called when the assembly code is entered
        /// </summary>
        virtual HookContext* OnEntry() final;

        /// <summary>
        /// This method gets called when the assembly code is 'exited'
        /// </summary>
        virtual void OnExit() final;

        /// <summary>
        /// Iterates to the next module and returns it
        /// </summary>
        virtual IModuleFunction* IterateModule() final;

        /// <summary>
        /// Resets the iterator to point at the beginning
        /// </summary>
        virtual void ResetIterator() final;

        /// <summary>
        /// Applies (or removes) the function detour (i.e hook)
        /// </summary>
        virtual void SetDetour(bool enabled) = 0;

    protected:
        // Protected members
        std::unique_ptr<CodeGenerator> mCodeGenerator;
        void* mOriginal;
        bool mDetoured;

    private:
        // Private type definitions
        typedef std::map<PluginId, std::shared_ptr<ModuleFunction>> ModuleCollection;

        // Private members
        ModuleCollection mModules;
        std::vector<ModuleCollection::const_iterator> mModuleIters;
        std::vector<std::shared_ptr<HookContext>> mHookContexts;
        ConventionInfo mConventionInfo;
        FNCallHook mCallFunc;
        std::string mName;
        uint mCallCount;
    };
}
