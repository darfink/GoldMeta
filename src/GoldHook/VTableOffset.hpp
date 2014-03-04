#pragma once

namespace gm {
    /// <summary>
    /// Gets the offset of any method within a class virtual table (max 10 virtual methods)
    /// </summary>
    template <class T, typename F>
    size_t VTableOffset(F function) {
        struct /* Anonymous */ {
            virtual size_t Get1()  { return 0; }
            virtual size_t Get2()  { return 1; }
            virtual size_t Get3()  { return 2; }
            virtual size_t Get4()  { return 3; }
            virtual size_t Get5()  { return 4; }
            virtual size_t Get6()  { return 5; }
            virtual size_t Get7()  { return 6; }
            virtual size_t Get8()  { return 7; }
            virtual size_t Get9()  { return 8; }
            virtual size_t Get10() { return 9; }
        } vt;

        T* object = reinterpret_cast<T*>(&vt);

        typedef size_t (T::*GetIndex)();
        GetIndex getIndex = reinterpret_cast<GetIndex>(function);
        return (object->*getIndex)() * sizeof(uintptr_t);
    }
}
