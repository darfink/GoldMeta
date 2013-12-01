#pragma once

#include <GoldMeta/Shared.hpp>

namespace HL {
    // Forward declarations
    typedef struct globalvars_s globalvars_t;
    typedef struct entvars_s entvars_t;
    class IFileSystem;
}

namespace gm {
    /// <summary>
    /// The GoldMeta API functions
    /// </summary>
    class IGoldAPI {
    public:
        /// <summary>
        /// Gets the HL engine globals
        /// </summary>
        virtual HL::globalvars_t* GetEngineGlobals() = 0;

        /// <summary>
        /// Gets the HL engine file system API
        /// </summary>
        virtual HL::IFileSystem* GetEngineFileSystem() = 0;

        /// <summary>
        /// Gets the specific game information
        /// </summary>
        virtual const char* GetGameInfo(GameInfo info) = 0;

        /// <summary>
        /// Gets the user message count
        /// </summary>
        virtual size_t GetUserMessageCount() = 0;

        /// <summary>
        /// Gets the index of a specific user message
        /// </summary>
        virtual int GetUserMessageIndex(const char* name, int* size = nullptr) = 0;

        /// <summary>
        /// Gets the string representation of a message index
        /// </summary>
        virtual const char* GetUserMessage(int index, int* size = nullptr) = 0;

        /// <summary>
        /// Gets the path to the plugin library file
        /// </summary>
        virtual const char* GetPluginPath(PluginId id) = 0;

        /// <summary>
        /// Calls a specific game entity
        /// </summary>
        virtual bool CallGameEntity(const char* entity, HL::entvars_t* vars) = 0;

        /// <summary>
        /// Calls a specific plugin entity
        /// </summary>
        virtual bool CallPluginEntity(const char* entity, HL::entvars_t* vars) = 0;

        /// <summary>
        /// Gets a unique integer that can be used for querying client configurable variables
        /// </summary>
        virtual int GetUniqueRequestID() = 0;

        /// <summary>
        /// Creates a new server command with a user specified callback
        /// </summary>
        virtual bool AddServerCommand(const char* command, void* callback) = 0;
    };
}
