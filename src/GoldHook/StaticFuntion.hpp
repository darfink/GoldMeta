#include <vector>
#include <string>

#include "Function.hpp"

namespace gm {
    class StaticFunction : public Function {
    public:
        /// <summary>
        /// The exception that this class throws
        /// </summary>
        GM_DEFINE_EXCEPTION_INHERIT(Exception, Function::Exception);

        /// <summary>
        /// Constructs a static function instance
        /// </summary>
        StaticFunction(std::string name, ConventionInfo cInfo, void* address);

        /// <summary>
        /// Gets a callable address to the function
        /// </summary>
        virtual void* GetCallableAddress();

    private:
        /// <summary>
        /// Applies (or removes) the function detour (i.e hook)
        /// </summary>
        virtual void SetDetour(bool enabled);

        /// <summary>
        /// Applies the hook
        /// </summary>
        void ApplyHook();

        /// <summary>
        /// Removes the hook
        /// </summary>
        void RemoveHook();

        // Private members
        std::shared_ptr<byte> mTrampoline;
        size_t mBytesDisassembled;

        // Static members
        static const byte PatchRelative[];
    };
}
