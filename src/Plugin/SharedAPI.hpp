#pragma once

// Standard includes
#include <string>
#include <memory>

// Custom includes
#include "../Exception.hpp"
#include "../API/Shared.hpp"
#include "../Interface/ISharedAPI.hpp"
#include "../Interface/IGameLibrary.hpp"
#include "../Interface/IPluginManager.hpp"

namespace /* Anonymous */ {
	namespace APIMeta = GoldMeta::API::Meta;
	namespace APIGold = GoldMeta::API::Gold;
}

namespace GoldMeta { namespace Plugin {
	class SharedAPI : public Interface::ISharedAPI {
	public:
		/// <summary>
		/// Constructs a plugin API instance associated with a plugin
		/// </summary>
		SharedAPI(std::shared_ptr<PluginBase> plugin, HL::globalvars_t* engineGlobals);

		/// <summary>
		/// Gets the HL engine globals
		/// </summary>
		virtual HL::globalvars_t* GetEngineGlobals();

		/// <summary>
		/// Gets the specific game information
		/// </summary>
		virtual const char* GetGameInfo(API::GameInfo info);

		/// <summary>
		/// Gets the user message count
		/// </summary>
		virtual size_t GetUserMessageCount();

		/// <summary>
		/// Gets the index of a specific user message
		/// </summary>
		virtual int GetUserMessageIndex(const char* name, int* size = nullptr);

		/// <summary>
		/// Gets the string representation of a message index
		/// </summary>
		virtual const char* GetUserMessage(int index, int* size = nullptr);

		/// <summary>
		/// Gets the path to the plugin library file
		/// </summary>
		virtual const char* GetPluginPath(API::PluginId id);

		/// <summary>
		/// Calls a specific game entity
		/// </summary>
		virtual bool CallGameEntity(const char* entity, HL::entvars_t* vars);

		/// <summary>
		/// Calls a specific plugin entity
		/// </summary>
		virtual bool CallPluginEntity(const char* entity, HL::entvars_t* vars);

		/// <summary>
		/// Gets a unique integer that can be used for querying client configurable variables
		/// </summary>
		virtual int GetUniqueRequestID();

		/// <summary>
		/// Creates a new server command with a user specified callback
		/// </summary>
		virtual bool AddServerCommand(const char* command, API::FastFunc<void()> callback);

		/// <summary>
		/// Gets the meta API function table
		/// </summary>
		virtual const API::Meta::MetaAPI& GetMetaAPI() const;

	private:
		/// <summary>
		/// Setups the pointer function table for the Metamod API
		/// </summary>
		void SetupMetaAPI();

		/// <summary>
		/// Calls a specific entity instance
		/// </summary>
		bool CallEntity(const char* entity, HL::entvars_t* vars, Interface::IEntityExporter* exporter, const std::string& function);

		// Private members
		APIMeta::MetaAPI mMetaApi;
		HL::globalvars_t* mEngineGlobals;
		std::shared_ptr<Interface::IGameLibrary> mGameLibrary;
		std::shared_ptr<Interface::IPluginManager> mPluginManager;
		std::shared_ptr<Interface::IModuleManager> mModuleManager;
		HL::UserMessage** mUserMessage;
		int mRequestCounter;
	};
} }