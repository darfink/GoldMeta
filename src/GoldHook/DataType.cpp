#include <boost/algorithm/string.hpp>
#include <unordered_map>
#include <cassert>

#include "DataType.hpp"

namespace gm {
    DataType::DataType() :
        mIsUnsigned(false),
        mDataType(Pointer),
        mSize(0)
    {
    }

    DataType DataType::FromString(const std::string& type) {
        // Use a static map so we only have to create and initialize it once
        static std::unordered_map<std::string, DataType> mapping = {
            { "char",    FromType<char>  () },
            { "uchar",   FromType<byte>  () },
            { "byte",    FromType<byte>  () },
            { "short",   FromType<short> () },
            { "ushort",  FromType<ushort>() },
            { "int",     FromType<int>   () },
            { "uint",    FromType<uint>  () },
            { "long",    FromType<int64> () },
            { "ulong",   FromType<uint64>() },
            { "float",   FromType<float> () },
            { "double",  FromType<double>() },
            { "pointer", FromType<void*> () }
        };

        // Attempt to find the associated type (and we are case-insensitive)
        auto it = mapping.find(boost::algorithm::to_lower_copy(type));

        if(it == mapping.end()) {
            throw Exception(format("couldn't identify data type: %s") % type);
        } else {
            return it->second;
        }
    }

    size_t DataType::GetStackSize() const {
        // The stack size is always a multiple of 4
        return (mSize + 3) & ~3;
    }

    size_t DataType::GetSize() const {
        return mSize;
    }

    void DataType::SetSize(size_t size) {
        mSize = size;
    }

    DataType::Type DataType::GetType() const {
        return mDataType;
    }

    void DataType::SetType(Type type) {
        mDataType = type;
    }

    bool DataType::IsRegisterEligible() const {
        // Only certain data types are stored in machine registers, this does not include any floating point types
        return (mDataType == Integral || mDataType == Pointer || mDataType == Structure) && mSize <= sizeof(size_t);
    }

    bool DataType::IsUnsigned() const {
        assert(mDataType == Integral);
        return mIsUnsigned;
    }
}