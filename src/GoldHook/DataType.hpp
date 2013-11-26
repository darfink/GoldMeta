#pragma once

#include <type_traits>
#include <vector>

#include "../Exception.hpp"

namespace gm {
    class DataType {
    public:
        /// <summary>
        /// Describes all data types possible
        /// </summary>
        enum Type {
            Pointer,
            Integral,
            FloatingPoint,
            Structure,
            Class,
            Void,
        };

        /// <summary>
        /// The exception class that this object type throws
        /// </summary>
        GM_DEFINE_EXCEPTION(Exception);

        /// <summary>
        /// Constructs a data type instance
        /// </summary>
        DataType();

        /// <summary>
        /// Constructs a data type from a template argument
        /// </summary>
        template <typename T>
        static DataType FromType();

        /// <summary>
        /// Constructs a data type from a string (e.g 'int', 'void', 'char')
        /// </summary>
        static DataType FromString(const std::string& type);

        /// <summary>
        /// Gets the size on the stack of the data type
        /// </summary>
        size_t GetStackSize() const;

        /// <summary>
        /// Gets the size of the data type
        /// </summary>
        size_t GetSize() const;

        /// <summary>
        /// Sets the size of the data type
        /// </summary>
        void SetSize(size_t size);

        /// <summary>
        /// Gets the data type
        /// </summary>
        Type GetType() const;

        /// <summary>
        /// Sets the data type
        /// </summary>
        void SetType(Type type);

        /// <summary>
        /// Gets whether this data type could be used in a register
        /// </summary>
        bool IsRegisterEligible() const;

        /// <summary>
        /// Gets whether this data type is unsigned or not (only valid for integral types)
        /// </summary>
        bool IsUnsigned() const;

    private:
        // Private members
        bool mIsUnsigned;
        Type mDataType;
        size_t mSize;
    };

    template <typename T>
    DataType DataType::FromType() {
        DataType result;
        result.mSize = sizeof(T);

        if(std::is_void<T>::value) {
            result.mDataType = Void;
        } else if(std::is_integral<T>::value || std::is_enum<T>::value) {
            result.mIsUnsigned = std::is_unsigned<T>::value;
            result.mDataType = Integral;
        } else if(std::is_pointer<T>::value || std::is_array<T>::value || std::is_reference<T>::value || std::is_member_pointer<T>::value) {
            // Since pointers larger than 4 bytes may contain all kind of fuzz, we disallow such pointers
            assert(result.mSize == sizeof(void*));
            result.mDataType = Pointer;
        } else if(std::is_floating_point<T>::value) {
            result.mDataType = FloatingPoint;
        } else if(std::is_class<T>::value) {
            result.mDataType = (std::is_pod<T>::value ? Structure : Class);
        } else {
            throw Exception("couldn't identify data type");
        }

        return result;
    }
}
