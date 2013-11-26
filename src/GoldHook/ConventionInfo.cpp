#include <cassert>

#include "ConventionInfo.hpp"

namespace gm {
    ConventionInfo::ConventionInfo() :
        mConvention(CallingConvention::CDecl)
    {
    }

    ConventionInfo::ConventionInfo(CallingConvention cc, DataType returnType, std::vector<DataType> parameterTypes) :
        mParameters(parameterTypes),
        mReturn(returnType),
        mConvention(cc)
    {

    }

    CallingConvention ConventionInfo::GetConvention() const {
        return mConvention;
    }

    size_t ConventionInfo::GetStackSize() const {
        size_t totalStackSize = 0;

        for(const DataType& parameter : mParameters) {
            totalStackSize += parameter.GetStackSize();
        }

        // On GCC, the callee cleans the hidden parameter
        if(this->GetReturnMethod() == ReturnMethod::Hidden) {
            totalStackSize += sizeof(uintptr_t);
        }

        return totalStackSize;
    }

    const std::vector<DataType>& ConventionInfo::GetParameters() const {
        return mParameters;
    }

    const DataType& ConventionInfo::GetReturn() const {
        return mReturn;
    }

    ReturnMethod ConventionInfo::GetReturnMethod() const {
        size_t returnSize = mReturn.GetSize();

        switch(mReturn.GetType()) {
            default: assert(false);

            case DataType::Pointer:
            case DataType::Integral:
            case DataType::Structure: {
                if(returnSize <= sizeof(uint) && returnSize != 3) {
                    return ReturnMethod::EAX;
                } else if(returnSize == sizeof(uint64)) {
                    return ReturnMethod::EAX_EDX;
                } else if(mReturn.GetType() == DataType::Structure) {
                    return ReturnMethod::Hidden;
                } else /* Unsupported size */ {
                    assert(false);
                }
            }

            case DataType::FloatingPoint:
                return ReturnMethod::FSTP;
        }
    }

    bool ConventionInfo::IsCalleClean() const {
        switch(mConvention) {
            default: assert(false);

            case CallingConvention::Thiscall:
            case CallingConvention::Fastcall:
            case CallingConvention::Stdcall:
                return true;

            case CallingConvention::CDecl:
                return false;
        }
    }

    bool ConventionInfo::IsMethod() const {
        return mConvention == CallingConvention::Thiscall;
    }

    bool ConventionInfo::IsRTL() const {
        switch(mConvention) {
            default: assert(false);

            case CallingConvention::Thiscall:
            case CallingConvention::Fastcall:
            case CallingConvention::Stdcall:
            case CallingConvention::CDecl:
                return true;
        }
    }
}