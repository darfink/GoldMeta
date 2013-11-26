#pragma once

#include <unordered_map>
#include <memory>
#include <string>

#include "../Exception.hpp"

namespace gm {
    /// <summary>
    /// Class for managing dynamic cross-platform library loading
    /// </summary>
    class Library {
    public:
        /// <summary>
        /// The base exception class for the library
        /// </summary>
        GM_DEFINE_EXCEPTION(Exception);

        /// <summary>
        /// The exception thrown when a symbol could not be found
        /// </summary>
        GM_DEFINE_EXCEPTION_INHERIT(SymbolNotFound, Exception);

        /// <summary>
        /// The exception thrown when the library could not be loaded
        /// </summary>
        GM_DEFINE_EXCEPTION_INHERIT(LibraryNotLoaded, Exception);

        /// <summary>
        /// Constructs a library instance
        /// </summary>
        Library();

        /// <summary>
        /// Constructs and loads a library from a path
        /// </summary>
        Library(std::string libraryPath);

        /// <summary>
        /// Loads a specific library
        /// </summary>
        void Load(std::string libraryPath);

        /// <summary>
        /// Unloads the current library
        /// </summary>
        void Unload();

        /// <summary>
        /// Constructs and loads a library from a path
        /// </summary>
        const std::string& GetPath() const;

        /// <summary>
        /// Constructs and loads a library from a path
        /// </summary>
        void* GetSymbol(const std::string& symbol, bool throwIfNotFound = true);

        /// <summary>
        /// Checks whether the library is loaded or not
        /// </summary>
        bool IsLoaded() const;

        /// <summary>
        /// Gets the code base and (optional) the size of the code section
        /// </summary>
        void* GetCodeBase(size_t* size = nullptr);

        /// <summary>
        /// Constructs and loads a library from a path
        /// </summary>
        static const std::string& GetExtension();

    private:
        /// <summary>
        /// Constructs and loads a library from a path
        /// </summary>
        std::string GetLibraryError() const;

        // Private members
        std::unordered_map<std::string, void*> mSymbols;
        std::shared_ptr<void> mLibrary;
        std::string mPath;
    };
}
