#include "ModuleFunction.hpp"
#include "../Interface/IFunctionBase.hpp"

namespace gm {
    ModuleFunction::ModuleFunction(PluginId id, IFunctionBase* functionBase) :
        mFunctionBase(functionBase),
        mPluginId(id)
    {
        mCallback = nullptr;
        mContext  = nullptr;
        mDisabled = false;
    }

    void* ModuleFunction::GetCallableAddress() {
        return mFunctionBase->GetCallableAddress();
    }

    void ModuleFunction::SetListener(void* function, void* context, int tense) {
        mCallback = function;
        mContext = context;
        mTense = tense;
    }

    void ModuleFunction::SetListenerTense(int tense) {
        mTense = tense;
    }

    void ModuleFunction::DisableListener() {
        mDisabled = true;
    }

    void ModuleFunction::EnableListener() {
        mDisabled = false;
    }

    void ModuleFunction::Call(void* returnValue, const void* arguments[]) {
        mFunctionBase->Call(returnValue, arguments);
    }

    void ModuleFunction::Release() {
        mFunctionBase->RemoveModule(mPluginId);
    }

    bool ModuleFunction::IsCallable(Tense::Type tense) {
        return (mTense & tense) && !mDisabled && mCallback != nullptr;
    }

    void* ModuleFunction::GetCallback() {
        return mCallback;
    }

    void* ModuleFunction::GetContext() {
        return mContext;
    }
}