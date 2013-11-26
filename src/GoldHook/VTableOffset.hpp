#pragma once

namespace gm {
    /// <summary>
    /// Gets the offset of any method within a class virtual table (max 10 virtual methods)
    /// </summary>
    template <class T, typename F>
    size_t VTableOffset(F function) {
        struct /* Anonymous */ {
            virtual size_t Get1()  { return 0 * sizeof(uintptr_t); }
            virtual size_t Get2()  { return 1 * sizeof(uintptr_t); }
            virtual size_t Get3()  { return 2 * sizeof(uintptr_t); }
            virtual size_t Get4()  { return 3 * sizeof(uintptr_t); }
            virtual size_t Get5()  { return 4 * sizeof(uintptr_t); }
            virtual size_t Get6()  { return 5 * sizeof(uintptr_t); }
            virtual size_t Get7()  { return 6 * sizeof(uintptr_t); }
            virtual size_t Get8()  { return 7 * sizeof(uintptr_t); }
            virtual size_t Get9()  { return 8 * sizeof(uintptr_t); }
            virtual size_t Get10() { return 9 * sizeof(uintptr_t); }
        } vt;

        T* object = reinterpret_cast<T*>(&vt);

        typedef size_t (T::*GetIndex)();
        GetIndex getIndex = reinterpret_cast<GetIndex>(function);
        return (object->*getIndex)();
    }
}
