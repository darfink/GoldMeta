#pragma once

#include <GoldMeta/Gold/IModuleFunction.hpp>
#include <asmjit/asmjit.h>
#include <memory>

#include "DataType.hpp"
#include "ConventionInfo.hpp"
#include "../Interface/IFunctionBase.hpp"
#include "../OS/OS.hpp"

namespace gm {
    /// <summary>
    /// The type of a 'call hook' function pointer
    /// </summary>
    typedef void(STDCALL *FNCallHook)(void*, const void*[]);

    class CodeGenerator {
    public:
        /// <summary>
        /// Constructs a hook generator instance
        /// </summary>
        CodeGenerator(IFunctionBase* function);

        /// <summary>
        /// Generates the assembly for a call hook
        /// </summary>
        FNCallHook GenerateCallHook();

        /// <summary>
        /// Generates the assembly for the hook handler
        /// </summary>
        void* GenerateHookHandler();

    private:
        /// <summary>
        /// Generates assembly for calling all plugins
        /// </summary>
        void CallModules(Tense::Type tense);

        /// <summary>
        /// Pushes a parameter on the stack from a specified source
        /// </summary>
        size_t PushParameter(const DataType& type, asmjit::host::Mem source);

        /// <summary>
        /// Retrieves a return value and stores it in the specified destination
        /// </summary>
        void SaveReturn(const DataType& type, asmjit::host::Mem destination);

        /// <summary>
        /// Sets the hook handler return value from the specified source
        /// </summary>
        void SetReturn(const DataType& type, asmjit::host::Mem source);

        /// <summary>
        /// Performs a bitwise copy procedure
        /// </summary>
        void PerformBitwiseCopy(size_t size, const asmjit::host::Mem& source, const asmjit::host::Mem& destination);

        /// <summary>
        /// Copies a data from from source to destination in the most efficient way for the type
        /// </summary>
        void CopyData(const DataType& type, asmjit::host::Mem source, asmjit::host::Mem destination);

        // Private members
        IFunctionBase* mFunctionBase;
        ConventionInfo mConventionInfo;
        asmjit::JitRuntime mJitRuntime;
        std::unique_ptr<asmjit::host::Assembler> mAssembler;
        std::shared_ptr<void> mHookHandler;
        std::shared_ptr<void> mCallHook;
        bool mHasNonHiddenReturn;
        size_t mLastArgument;
    };
}
