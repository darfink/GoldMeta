#ifndef ASMJIT_API
# define ASMJIT_API
#endif
#include <asmjit/asmjit.h>
#include <GoldMeta/Gold/IModuleFunction.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <cstddef>
#include <cassert>
#include <vector>

#include "HookContext.hpp"
#include "CodeGenerator.hpp"
#include "VTableOffset.hpp"

// Usually these declarations should be avoided
using namespace AsmJit;

namespace gm {
    CodeGenerator::CodeGenerator(IFunctionBase* function) :
        mAssembler(new X86Assembler),
        mHasNonHiddenReturn(false),
        mFunctionBase(function),
        mLastArgument(0)
    {
        assert(mFunctionBase != nullptr);
        assert(mAssembler);

        mConventionInfo = mFunctionBase->GetConventionInfo();
        mHasNonHiddenReturn = mConventionInfo.GetReturnMethod() != ReturnMethod::Hidden && mConventionInfo.GetReturn().GetType() != DataType::Void;

        // Calculate where the first argument will be relative to EBP
        for(const DataType& parameter : mConventionInfo.GetParameters()) {
            mLastArgument += parameter.GetStackSize();
        }
    }

    FNCallHook CodeGenerator::GenerateCallHook() {
        if(!mCallHook) {
            mAssembler->clear();

            // We use this to avoid to several memory accesses
            Label callerContext = mAssembler->newLabel();

            mAssembler->push(ebp);
            mAssembler->mov(ebp, esp);

            // We need to push the arguments in reverse order (and the first argument should be the context, if required)
            size_t index = mConventionInfo.GetParameters().size() + (mConventionInfo.IsMethod() ? sizeof(uintptr_t) : 0);

            if(index > 0) {
                // Copy the 'arguments' array pointer to ECX
                mAssembler->mov(ecx, dword_ptr(ebp, 12));

                for(const DataType& parameter : mConventionInfo.GetParameters()) {
                    mAssembler->mov(eax, ptr(ecx, --index * sizeof(uintptr_t)));
                    this->PushParameter(parameter, ptr(eax));
                }

                if(mConventionInfo.IsMethod()) {
                    mAssembler->mov(eax, ptr(ecx, 0));
                    mAssembler->mov(eax, dword_ptr(eax));
                    mAssembler->mov(dword_ptr(callerContext), eax);
                }
            }

            // Call 'IFunctionBase::GetCallableAddress' to retrieve the address that we should use
            // for calling the original function. The result will be stored in EAX for later usage.
            mAssembler->mov(ecx, reinterpret_cast<uintptr_t>(mFunctionBase));
            mAssembler->mov(eax, dword_ptr(ecx));
            mAssembler->call(dword_ptr(eax, VTableOffset<IFunctionBase>(&IFunctionBase::GetCallableAddress)));

            if(mConventionInfo.IsMethod()) {
                // If it is a method, we need to supply the context
                mAssembler->mov(ecx, dword_ptr(callerContext));
            }

            if(mConventionInfo.GetReturnMethod() == ReturnMethod::Hidden) {
                // If the return is done by a hidden parameter, push it!
                mAssembler->push(dword_ptr(ebp, 8));
            }

            // Call the original function
            mAssembler->call(eax);

            if(!mConventionInfo.IsCalleClean()) {
                mAssembler->add(esp, mConventionInfo.GetStackSize());
            }

            if(mHasNonHiddenReturn == true) {
                // We need to save the return value
                mAssembler->mov(ecx, dword_ptr(ebp, 8));
                this->SaveReturn(mConventionInfo.GetReturn(), ptr(ecx));
            }

            mAssembler->pop(ebp);
            mAssembler->ret(8);

            // ------------------------------------------------------

            if(mConventionInfo.IsMethod()) {
                mAssembler->bind(callerContext);
                mAssembler->dptr(nullptr);
            }

            // Retrieve the assembly code, ready for execution
            mCallHook.reset(mAssembler->make(), [](void* code) { MemoryManager::getGlobal()->free(code); });
        }

        return reinterpret_cast<FNCallHook>(mCallHook.get());
    }

    void* CodeGenerator::GenerateHookHandler() {
        if(!mHookHandler) {
            mAssembler->clear();

            // Because hooks can be called recursively, we cannot store any data in the assembly, so we only store it
            // temporarily at these label addresses until we have received a hook context, then we copy the values.
            Label returnData     = mAssembler->newLabel();
            Label callerAddress  = mAssembler->newLabel();
            Label calleeContext  = mAssembler->newLabel();
            Label skipOrigCall   = mAssembler->newLabel();
            Label returnToCaller = mAssembler->newLabel();
            Label skipCopyReturn = mAssembler->newLabel();

            // Save the callback address for later
            mAssembler->pop(dword_ptr(callerAddress));

            if(mConventionInfo.IsMethod()) {
                // The object instance is in ecx, so copy it to the hook context
                mAssembler->mov(dword_ptr(calleeContext), ecx);
            }

            if(mConventionInfo.GetReturnMethod() == ReturnMethod::Hidden) {
                // We need the return address for later
                mAssembler->pop(dword_ptr(returnData));
            }

            // Call the function method 'OnEntry'. This function setups some necessary data, but above all,
            // it returns the current hook context in EAX. This is where we store all data for this call. To avoid
            // heap allocations, the hook context is only allocated when necessary, otherwise it is reused.
            mAssembler->mov(ecx, reinterpret_cast<uintptr_t>(mFunctionBase));
            mAssembler->mov(eax, dword_ptr(ecx));
            mAssembler->call(dword_ptr(eax, VTableOffset<IFunctionBase>(&IFunctionBase::OnEntry)));

            // Function prolog - Normally the function parameters start at [EBP + 8], but since we have
            // popped the caller return address from the stack, the location as been dislocated by 4 bytes.
            // So this means that the first argument can be accessed at [EBP + 4] instead.
            mAssembler->push(ebp);
            mAssembler->mov(ebp, esp);

            // ------------------------------------------------------

            // Copy the hook context to EBX
            mAssembler->push(ebx);
            mAssembler->push(esi);
            mAssembler->push(edi);
            mAssembler->mov(ebx, eax);

            // Copy the caller address to the hook context
            mAssembler->mov(eax, dword_ptr(callerAddress));
            mAssembler->mov(dword_ptr(ebx, offsetof(HookContext, callerAddress)), eax);

            if(mConventionInfo.IsMethod()) {
                // Copy the callee context to the hook context
                mAssembler->mov(eax, dword_ptr(calleeContext));
                mAssembler->mov(dword_ptr(ebx, offsetof(HookContext, calleeContext)), eax);
            }

            if(mConventionInfo.GetReturnMethod() == ReturnMethod::Hidden) {
                // Copy the hidden return address to the hook context
                mAssembler->mov(eax, dword_ptr(returnData));
                mAssembler->mov(dword_ptr(ebx, offsetof(HookContext, originalReturn)), eax);
            }

            // Generate the assembly code for calling all modules that are listed as 'pre' hooks
            this->CallModules(Tense::Pre);

            // Check whether we should call the original function or not.
            mAssembler->cmp(dword_ptr(ebx, offsetof(HookContext, highestResult)), static_cast<int>(Result::Supersede));
            mAssembler->je(skipOrigCall);
            {
                size_t stackDisplacement = mLastArgument;

                for(const DataType& parameter : boost::adaptors::reverse(mConventionInfo.GetParameters())) {
                    // Push each function parameter to the target module, and add the EBP offset (first argument at [EBP + 4])
                    stackDisplacement -= this->PushParameter(parameter, ptr(ebp, stackDisplacement));
                }

                // Call 'IFunctionBase::GetCallableAddress' to retrieve the address that we should use
                // for calling the original function. The result will be stored in EAX for later usage.
                mAssembler->mov(ecx, reinterpret_cast<uintptr_t>(mFunctionBase));
                mAssembler->mov(eax, dword_ptr(ecx));
                mAssembler->call(dword_ptr(eax, VTableOffset<IFunctionBase>(&IFunctionBase::GetCallableAddress)));

                if(mConventionInfo.IsMethod()) {
                    // If it is a method, we need to supply the context
                    mAssembler->mov(ecx, dword_ptr(ebx, offsetof(HookContext, calleeContext)));
                }

                if(mConventionInfo.GetReturnMethod() == ReturnMethod::Hidden) {
                    // In case the return value is hidden, push the result data address
                    mAssembler->push(dword_ptr(ebx, offsetof(HookContext, originalReturn)));
                }

                // Call the original function!
                mAssembler->call(eax);

                if(!mConventionInfo.IsCalleClean()) {
                    // We need to clean up the stack after us
                    mAssembler->add(esp, mConventionInfo.GetStackSize());
                }

                if(mHasNonHiddenReturn == true) {
                    // We need to copy the original return value to the hook context so function modules may access the value
                    mAssembler->mov(ecx, dword_ptr(ebx, offsetof(HookContext, originalReturn)));
                    this->SaveReturn(mConventionInfo.GetReturn(), ptr(ecx));
                }
            }

            mAssembler->jmp(skipCopyReturn);
            mAssembler->bind(skipOrigCall);

            if(mConventionInfo.GetReturn().GetType() != DataType::Void) {
                // The function has been superseded, so we set the original return value to the overridden one (since no original value exists)
                mAssembler->mov(eax, dword_ptr(ebx, offsetof(HookContext, overrideReturn)));
                mAssembler->mov(edx, dword_ptr(ebx, offsetof(HookContext, originalReturn)));
                this->CopyData(mConventionInfo.GetReturn(), ptr(eax), ptr(edx));
            }

            mAssembler->bind(skipCopyReturn);

            // Generate the assembly code for calling all modules that are listed as 'post' hooks
            this->CallModules(Tense::Post);

            // ------------------------------------------------------

            if(mConventionInfo.GetReturn().GetType() != DataType::Void) {
                // Check whether we should use a overridden return value or the original one, returned by the function
                mAssembler->cmp(dword_ptr(ebx, offsetof(HookContext, highestResult)), static_cast<int>(Result::Override));
                mAssembler->cmovne(eax, dword_ptr(ebx, offsetof(HookContext, originalReturn)));
                mAssembler->cmove(eax, dword_ptr(ebx, offsetof(HookContext, overrideReturn)));
                mAssembler->mov(dword_ptr(returnData), eax);
            }

            // Since we pop EBX, we set the 'caller' address so we can use it later
            mAssembler->mov(eax, dword_ptr(ebx, offsetof(HookContext, callerAddress)));
            mAssembler->mov(dword_ptr(callerAddress), eax);

            // We are going to call member functions of 'IFunctionBase'
            mAssembler->mov(ecx, reinterpret_cast<uintptr_t>(mFunctionBase));

            // Reset preserved registers
            mAssembler->pop(edi);
            mAssembler->pop(esi);
            mAssembler->pop(ebx);

            // Ensure that the stack isn't unbalanced
            mAssembler->cmp(esp, ebp);
            mAssembler->je(returnToCaller);
            {
                // If the stack has become displaced, we cannot return execution to the caller. This should
                // actually never happen, but just in case it does, I have created a check for it. The 'InvalidESP'
                // member function will be called and report this error and forcefully exit the application.
                // If this happens, there is (probably) something wrong with the assembly code, or a user callback
                // that has specified a wrong calling convention which results in an invalid ESP value.
                mAssembler->mov(eax, dword_ptr(ecx));
                mAssembler->call(dword_ptr(eax, VTableOffset<IFunctionBase>(&IFunctionBase::InvalidESP)));
            }
            mAssembler->bind(returnToCaller);
            mAssembler->pop(ebp);

            // Call the 'OnExit' method
            mAssembler->mov(eax, dword_ptr(ecx));
            mAssembler->call(dword_ptr(eax, VTableOffset<IFunctionBase>(&IFunctionBase::OnExit)));

            if(mConventionInfo.GetReturn().GetType() != DataType::Void) {
                // If it is a hidden return, we just need to return the pointer
                mAssembler->mov(eax, dword_ptr(returnData));
          
                if(mHasNonHiddenReturn == true) {
                    // Otherwise we need to copy the data to the appropriate register
                    this->SetReturn(mConventionInfo.GetReturn(), ptr(eax));
                }
            }

            if(mConventionInfo.IsCalleClean()) {
                size_t stackSize = 0;

                // We can't use 'ConventionInfo::GetStackSize', because it accounts for the (possible) hidden return parameter that we popped earlier
                for(const DataType& parameter : mConventionInfo.GetParameters()) {
                    stackSize += parameter.GetStackSize();
                }

                // We need to clean up the stack space ourself
                mAssembler->add(esp, stackSize);
            }

            // Return to the calling address
            mAssembler->jmp(dword_ptr(callerAddress));

            // ------------------------------------------------------

            // ... and append the '.data' section
            mAssembler->bind(callerAddress);
            mAssembler->dptr(nullptr);

            if(mConventionInfo.GetReturn().GetType() != DataType::Void) {
                mAssembler->bind(returnData);
                mAssembler->dptr(nullptr);
            }

            if(mConventionInfo.IsMethod()) {
                mAssembler->bind(calleeContext);
                mAssembler->dptr(nullptr);
            }

            mHookHandler.reset(mAssembler->make(), [](void* code) { MemoryManager::getGlobal()->free(code); });
        }

        return mHookHandler.get();
    }

    void CodeGenerator::CallModules(Tense::Type tense) {
        // Define all labels that we are utilizing
        Label iterateModule  = mAssembler->newLabel();
        Label skipHighResult = mAssembler->newLabel();
        Label endCallModule  = mAssembler->newLabel();

        // Update the hook context with the current tense
        mAssembler->mov(dword_ptr(ebx, offsetof(HookContext, tense)), tense);

        // Call 'ResetIterator' (required since we call both Pre & Post)
        mAssembler->mov(ecx, reinterpret_cast<uintptr_t>(mFunctionBase));
        mAssembler->mov(eax, dword_ptr(ecx));
        mAssembler->call(dword_ptr(eax, VTableOffset<IFunctionBase>(&IFunctionBase::ResetIterator)));

        // Copy 'IFunctionBase' to ECX and call 'IterateModule'
        mAssembler->bind(iterateModule);
        mAssembler->mov(ecx, reinterpret_cast<uintptr_t>(mFunctionBase));
        mAssembler->mov(eax, dword_ptr(ecx));
        mAssembler->call(dword_ptr(eax, VTableOffset<IFunctionBase>(&IFunctionBase::IterateModule)));

        // The return value of type 'IModuleFunction' is NULL when we have reached the end
        mAssembler->cmp(eax, NULL);
        mAssembler->je(endCallModule);
        {
            // Set the currently active function module in the hook context
            mAssembler->mov(dword_ptr(ebx, offsetof(HookContext, module)), eax);

            // Call the method 'IsCallable' to determine whether we should handle this module or not
            mAssembler->push(tense);
            mAssembler->mov(ecx, eax);
            mAssembler->mov(eax, dword_ptr(ecx));
            mAssembler->call(dword_ptr(eax, VTableOffset<IModuleFunction>(&IModuleFunction::IsCallable)));
            mAssembler->cmp(al, false);
            mAssembler->je(iterateModule);

            size_t stackDisplacement = mLastArgument;

            for(const DataType& parameter : boost::adaptors::reverse(mConventionInfo.GetParameters())) {
                // Push each function parameter to the target module, and add the EBP offset (first argument at [EBP + 4])
                stackDisplacement -= this->PushParameter(parameter, ptr(ebp, stackDisplacement));
            }

            // The last argument is the hook context
            mAssembler->push(ebx);

            if(mConventionInfo.GetReturnMethod() == ReturnMethod::Hidden) {
                // In case the return value is hidden, push the result data address
                mAssembler->push(dword_ptr(ebx, offsetof(HookContext, currentReturn)));
            }

            // Call the module function with the associated context
            mAssembler->mov(ecx, dword_ptr(ebx, offsetof(HookContext, module)));
            mAssembler->mov(edx, dword_ptr(ecx));
            mAssembler->call(dword_ptr(edx, VTableOffset<IModuleFunction>(&IModuleFunction::GetCallback)));
            mAssembler->push(eax);
            mAssembler->call(dword_ptr(edx, VTableOffset<IModuleFunction>(&IModuleFunction::GetContext)));
            mAssembler->mov(ecx, eax);
            mAssembler->pop(eax);
            mAssembler->call(eax);

            // Assign the current module result to the 'previous' result
            mAssembler->mov(ecx, dword_ptr(ebx, offsetof(HookContext, currentResult)));
            mAssembler->mov(dword_ptr(ebx, offsetof(HookContext, currentResult)), static_cast<int>(Result::Unset));

            // Replace 'previous' result with the current module function result
            mAssembler->mov(dword_ptr(ebx, offsetof(HookContext, previousResult)), ecx);

            // Check if we need to replace the highest result with the one returned
            mAssembler->cmp(ecx, dword_ptr(ebx, offsetof(HookContext, highestResult)));
            mAssembler->jbe(skipHighResult);
            mAssembler->mov(dword_ptr(ebx, offsetof(HookContext, highestResult)), ecx);
            mAssembler->bind(skipHighResult);

            if(mHasNonHiddenReturn == true) {
                // We need to retrieve the return value so we can replace the original return value
                // if necessary. Although we handle the return value of all functions, because floating point
                // return values demand that they are popped from the floating point stack.
                mAssembler->push(ecx);
                mAssembler->mov(ecx, dword_ptr(ebx, offsetof(HookContext, currentReturn)));
                this->SaveReturn(mConventionInfo.GetReturn(), ptr(ecx));
                mAssembler->pop(ecx);
            }

            if(mConventionInfo.GetReturn().GetType() != DataType::Void) {
                // Check if the module function should override the original return value
                mAssembler->cmp(ecx, static_cast<int>(Result::Override));
                mAssembler->jb(iterateModule);
                mAssembler->cmp(ecx, dword_ptr(ebx, offsetof(HookContext, highestResult)));
                mAssembler->jb(iterateModule);

                // We need to dereference the addresses twice, so we move them to EAX:EDX first
                mAssembler->mov(eax, dword_ptr(ebx, offsetof(HookContext, currentReturn)));
                mAssembler->mov(edx, dword_ptr(ebx, offsetof(HookContext, overrideReturn)));

                // Then we copy data data between the source to the destination
                this->CopyData(mConventionInfo.GetReturn(), ptr(eax), ptr(edx));
            }
        }
        mAssembler->jmp(iterateModule);
        mAssembler->bind(endCallModule);
    }

    // NOTE: This method may only touch the EDX register
    size_t CodeGenerator::PushParameter(const DataType& type, Mem source) {
        size_t displacement = source.getDisplacement();
        size_t typeSize = type.GetSize();

        switch(type.GetType()) {
            default: assert(false);

            case DataType::Pointer:
            case DataType::Integral:
            case DataType::Structure: {
                // Update the memory operand size
                source.setSize(std::min(typeSize, sizeof(size_t)));

                switch(typeSize) {
                    default: {
                        assert(type.GetType() == DataType::Structure);

                        // Make room on the stack for our data
                        mAssembler->sub(esp, type.GetStackSize());

                        // Since arguments aren't pushed in bytes, but in double words, we take that into consideration
                        source.setDisplacement(displacement - (type.GetStackSize() - sizeof(uint)));

                        if(typeSize <= sizeof(uint) * 6 && typeSize % sizeof(uint) == 0) {
                            // We push one double word at a time
                            source.setSize(sizeof(uint));
                            mAssembler->mov(eax, esp);

                            displacement = source.getDisplacement();

                            for(size_t index = 0; index < typeSize; index += sizeof(uint)) {
                                source.setDisplacement(displacement + index);

                                mAssembler->mov(edx, source);
                                mAssembler->mov(dword_ptr(eax, index), edx);
                            }
                        } else /* It's more effective with a bitwise copy operation */ {
                            this->PerformBitwiseCopy(typeSize, source, ptr(esp));
                        }

                        break;
                    }

                    case sizeof(byte):
                    case sizeof(ushort):
                        mAssembler->movzx(edx, source);
                        mAssembler->push(edx);
                        break;

                    case sizeof(uint):
                        mAssembler->push(source);
                        break;

                    case sizeof(uint64):
                        source.setDisplacement(displacement - sizeof(size_t));
                        mAssembler->push(source);
                        source.setDisplacement(displacement);
                        mAssembler->push(source);

                        break;
                }

                break;
            }

            case DataType::FloatingPoint: {
                source.setSize(typeSize);
                mAssembler->fld(source);
                mAssembler->sub(esp, typeSize);
                mAssembler->fstp(ptr(esp, 0, typeSize));
                break;
            }
        }

        return type.GetStackSize();
    }

    // May not touch the ECX register
    void CodeGenerator::SaveReturn(const DataType& type, Mem destination) {
        size_t typeSize = type.GetSize();

        switch(type.GetType()) {
            default: assert(false);

            case DataType::Pointer:
            case DataType::Integral:
            case DataType::Structure: {
                switch(typeSize) {
                    default:
                        assert(type.GetType() == DataType::Structure);
                        break;

                    case sizeof(byte):
                        mAssembler->mov(destination, al);
                        break;

                    case sizeof(ushort):
                        mAssembler->mov(destination, ax);
                        break;

                    case sizeof(uint):
                        mAssembler->mov(destination, eax);
                        break;

                    case sizeof(uint64):
                        mAssembler->mov(destination, eax);
                        destination.setDisplacement(sizeof(size_t));
                        mAssembler->mov(destination, edx);
                        break;
                }

                break;
            }

            case DataType::FloatingPoint: {
                destination.setSize(typeSize);
                mAssembler->fstp(destination);
                break;
            }
        }
    }

    void CodeGenerator::SetReturn(const DataType& type, AsmJit::Mem source) {
        size_t typeSize = type.GetSize();

        switch(type.GetType()) {
            default: assert(false);

            case DataType::Pointer:
            case DataType::Integral:
            case DataType::Structure: {
                switch(typeSize) {
                    default:
                        assert(type.GetType() == DataType::Structure);
                        break;

                    case sizeof(byte):
                        mAssembler->mov(al, source);
                        break;

                    case sizeof(ushort):
                        mAssembler->mov(ax, source);
                        break;

                    case sizeof(uint):
                        mAssembler->mov(eax, source);
                        break;

                    case sizeof(uint64):
                        mAssembler->mov(eax, source);
                        source.setDisplacement(sizeof(size_t));
                        mAssembler->mov(edx, source);
                        break;
                }

                break;
            }

            case DataType::FloatingPoint: {
                source.setSize(typeSize);
                mAssembler->fld(source);
                break;
            }
        }
    }

    void CodeGenerator::PerformBitwiseCopy(size_t size, const AsmJit::Mem& source, const AsmJit::Mem& destination) {
        uint dwords = size / sizeof(size_t);
        uint bytes = size % sizeof(size_t);

        mAssembler->lea(edi, destination);
        mAssembler->lea(esi, source);

        if(dwords > 0) {
            mAssembler->mov(ecx, dwords);
            mAssembler->rep_movsd();
        }

        if(bytes > 0) {
            mAssembler->mov(ecx, bytes);
            mAssembler->rep_movsb();
        }
    }

    void CodeGenerator::CopyData(const DataType& type, AsmJit::Mem source, AsmJit::Mem destination) {
        size_t typeSize = type.GetSize();

        switch(type.GetType()) {
            case DataType::Pointer:
            case DataType::Structure:
            case DataType::Integral: {
                switch(typeSize) {
                    default: {
                        assert(type.GetType() == DataType::Structure);
                        this->PerformBitwiseCopy(typeSize, source, destination);

                        break;
                    }

                    case sizeof(byte):
                        mAssembler->mov(al, source);
                        mAssembler->mov(destination, al);
                        break;

                    case sizeof(ushort):
                        mAssembler->mov(ax, source);
                        mAssembler->mov(destination, ax);
                        break;

                    case sizeof(uint) :
                        mAssembler->mov(eax, source);
                        mAssembler->mov(destination, eax);
                        break;

                    case sizeof(uint64):
                        // We copy two double words
                        for(int i = 0; i < 2; i++) {
                            destination.setDisplacement(sizeof(size_t) * i);
                            source.setDisplacement(sizeof(size_t) * i);

                            mAssembler->mov(eax, source);
                            mAssembler->mov(destination, eax);
                        }

                        break;
                }
                break;
            }

            case DataType::FloatingPoint: {
                source.setSize(typeSize);
                destination.setSize(typeSize);

                mAssembler->fld(source);
                mAssembler->fstp(destination);

                break;
            }
        }
    }
}