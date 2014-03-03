#include <cstring>

#include "../Default.hpp"
#include "HookContext.hpp"
#include "ConventionInfo.hpp"

namespace gm {
    HookContext::HookContext(IFunctionBase* function) :
        mFunctionBase(function),
        mHiddenReturn(false),
        mReturnSize(0)
    {
        mHiddenReturn = mFunctionBase->GetConventionInfo().GetReturnMethod() == ReturnMethod::Hidden;
        mReturnSize = mFunctionBase->GetConventionInfo().GetReturn().GetSize();

        if(mReturnSize > 0) {
            this->overrideReturn = new byte[mReturnSize];
            this->currentReturn = new byte[mReturnSize];

            if(!mHiddenReturn) {
                // The original return is overridden if the return method is hidden
                this->originalReturn = new byte[mReturnSize];
            }
        }

        this->Reset();
    }

    HookContext::~HookContext() {
        if(mReturnSize > 0) {
            delete[] this->overrideReturn;
            delete[] this->currentReturn;

            if(!mHiddenReturn) {
                delete[] this->originalReturn;
            }
        }
    }

    void* HookContext::GetContext() {
        return this->calleeContext;
    }

    void HookContext::SetParameter(unsigned int index, const void* value) {
        // TODO: Implement this
        if((this->tense & Tense::Pre) == 0) {
            std::cerr << "[WARNING] A plugin tried to set a hook context parameter in post\n";
        }
    }

    IModuleFunction* HookContext::GetFunction() {
        return this->module;
    }

    Result HookContext::GetHighestResult() {
        return this->highestResult;
    }

    Result HookContext::GetPreviousResult() {
        return this->previousResult;
    }

    void HookContext::SetResult(Result result) {
        if(this->IsPre()) {
            this->currentResult = clamp(result, Result::Ignored, Result::Supersede);
        } else /* Post */ {
            this->currentResult = clamp(result, Result::Ignored, Result::Override);
        }
    }

    void HookContext::GetOriginalReturn(void* value) {
        if(value == nullptr || mReturnSize == 0) {
            return;
        }

        std::memcpy(value, this->originalReturn, mReturnSize);
    }

    void HookContext::GetOverrideReturn(void* value) {
        if(value == nullptr || mReturnSize == 0) {
            return;
        }

        std::memcpy(value, this->overrideReturn, mReturnSize);
    }

    bool HookContext::IsPost() {
        return this->tense == Tense::Post;
    }

    bool HookContext::IsPre() {
        return this->tense == Tense::Pre;
    }

    void HookContext::Reset() {
        // Just set each member to its default value
        this->tense          = Tense::Pre;
        this->currentResult  = Result::Unset;
        this->previousResult = Result::Unset;
        this->highestResult  = Result::Unset;
        this->callerAddress  = nullptr;
        this->calleeContext  = nullptr;
        this->module         = nullptr;

        if(mReturnSize > 0) {
            // We don't want the memory to have invalid values
            std::memset(this->overrideReturn, 0, mReturnSize);
            std::memset(this->currentReturn,  0, mReturnSize);

            if(!mHiddenReturn) {
                std::memset(this->originalReturn, 0, mReturnSize);
            }
        }
    }
}
