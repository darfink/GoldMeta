#include <cstdlib> // 'alloca'
#include <cassert>

#include "Function.hpp"
#include "HookContext.hpp"
#include "ModuleFunction.hpp"

namespace gm {
    Function::Function(std::string name, ConventionInfo cInfo) :
        mConventionInfo(cInfo),
        mOriginal(nullptr),
        mDetoured(false),
        mCallCount(0),
        mCallFunc(nullptr),
        mName(name)
    {
        mCodeGenerator = std::make_unique<CodeGenerator>(this);
    }

    Function::~Function() { }

    IModuleFunction* Function::GetModule(PluginId plugin) {
        // Check if it already exists
        auto it = mModules.find(plugin);

        this->SetDetour(true);

        if(it != mModules.end()) {
            return it->second.get();
        } else {
            // The function module did not exist, so we add it to our module collection and return it
            return mModules.insert({ plugin, std::make_shared<ModuleFunction>(plugin, this) }).first->second.get();
        }
    }

    void Function::RemoveModule(PluginId plugin) {
        // No more, no less...
        mModules.erase(plugin);
    }

    void Function::Call(void* returnValue, const void* arguments[]) {
        if(mCallFunc == nullptr) {
            // Generate the assembly code for calling the function
            mCallFunc = mCodeGenerator->GenerateCallHook();
        }

        size_t returnSize = mConventionInfo.GetReturn().GetSize();

        if(returnValue == nullptr && returnSize > 0) {
            // In case the caller does not care about the return value
            returnValue = alloca(returnSize);
        }

        size_t argumentCount = mConventionInfo.GetParameters().size();

        if(arguments == nullptr && argumentCount > 0) {
            // We cannot guess the arguments to the function, so we issue a warning in case this occurs
            std::cerr << format("[WARNING] A plugin tried to call function '%s' with no arguments when %d were expected\n") % mName % argumentCount;
            return;
        }

        // Forward the parameters to the call function
        mCallFunc(returnValue, arguments);
    }

    const ConventionInfo& Function::GetConventionInfo() {
        return mConventionInfo;
    }

    void Function::InvalidESP() {
        std::exit(EXIT_FAILURE);
    }

    HookContext* Function::OnEntry() {
        mCallCount++;

        // Some events may lead to a recursive call within the generated assembly, and we must handle
        // this occasion because otherwise the hook context and module iterator will be overwritten.
        // The solution is to backup all 'call' specific member variables and use the backup later.
        if(mCallCount > mModuleIters.size()) {
            mModuleIters.push_back(mModules.cend());
        }

        if(mCallCount > mHookContexts.size()) {
            mHookContexts.emplace_back(std::make_shared<HookContext>(this));
        }

        // Retrieve the current hook context that will be used
        HookContext* context = mHookContexts[mCallCount - 1].get();
       
        context->Reset();
        return context;
    }

    void Function::OnExit() {
        mCallCount--;
    }

    IModuleFunction* Function::IterateModule() {
        // Retrieve the currently used iterator
        auto& iterator = mModuleIters[mCallCount - 1];

        if(iterator == mModules.cend()) {
            return nullptr;
        } else {
            ModuleFunction* result = iterator->second.get();
            ++iterator;

            return result;
        }
    }

    void Function::ResetIterator() {
        // Make the iterator point at the start again
        mModuleIters[mCallCount - 1] = mModules.cbegin();
    }
}