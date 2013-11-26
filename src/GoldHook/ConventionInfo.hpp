#pragma once

#include <vector>

#include "DataType.hpp"

namespace gm {
    enum class CallingConvention {
        Thiscall,
        Fastcall,
        Stdcall,
        CDecl,
    };

    enum class ReturnMethod {
        FSTP,
        Hidden,
        EAX_EDX,
        EAX,
    };

    class ConventionInfo {
    public:
        /// <summary>
        /// Constructs an empty convention info instance
        /// </summary>
        ConventionInfo();

        /// <summary>
        /// Constructs a convention info instance
        /// </summary>
        ConventionInfo(CallingConvention cc, DataType returnType, std::vector<DataType> parameterTypes);

        /// <summary>
        ///
        /// </summary>
        CallingConvention GetConvention() const;

        /// <summary>
        ///
        /// </summary>
        size_t GetStackSize() const;

        /// <summary>
        ///
        /// </summary>
        const std::vector<DataType>& GetParameters() const;

        /// <summary>
        ///
        /// </summary>
        const DataType& GetReturn() const;

        /// <summary>
        ///
        /// </summary>
        ReturnMethod GetReturnMethod() const;

        /// <summary>
        ///
        /// </summary>
        bool IsCalleClean() const;

        /// <summary>
        ///
        /// </summary>
        bool IsMethod() const;

        /// <summary>
        ///
        /// </summary>
        bool IsRTL() const;

    private:
        // Private members
        CallingConvention mConvention;
        std::vector<DataType> mParameters;
        DataType mReturn;
    };
}
