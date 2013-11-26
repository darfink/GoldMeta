// Standard includes
#include <boost/filesystem.hpp>
#include <iostream>
#include <cstring>
#include <cassert>

// Custom includes
#include "SharedAPI.hpp"
#include "PluginBase.hpp"
#include "../PathManager.hpp"

namespace fs = boost::filesystem;

namespace GoldMeta { namespace Plugin {
	// Sadly we must use this approach since the Metamod API function table consists of regular function pointers
	// (which cannot be assigned a member function). So we combine a global variable with lambdas to solve this.
	SharedAPI* gSharedAPI = nullptr;

	SharedAPI::SharedAPI(std::shared_ptr<Hypodermic::IContainer> container, HL::globalvars_t* engineGlobals) :
		mEngineGlobals(engineGlobals),
		mUserMessage(nullptr),
		mRequestCounter(0)
	{
		assert(engineGlobals != nullptr);
		assert(gSharedAPI == nullptr); /* If the shared API is not null, that means a 'SharedAPI' instance has already been created */
		
		mPluginManager = container->resolve<Interface::IPluginManager>();
		mModuleManager = container->resolve<Interface::IModuleManager>();
		mGameLibrary = container->resolve<Interface::IGameLibrary>();

		uint address = mModuleManager->FindSignature(Interface::IModuleManager::Engine, { 0x74, 0x00, 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x3B, 0xCB, 0x75 }, "x?xx????xxx", 4);

		if(address != 0) {
			// Wohoo there, talk about pointerception
			mUserMessage = *reinterpret_cast<HL::UserMessage***>(address);
		}

		gSharedAPI = this;

		std::memset(&mMetaApi, 0, sizeof(APIMeta::MetaAPI));
		this->SetupMetaAPI();
	}

	void SharedAPI::SetupMetaAPI() {
		// The same type definition seen in Metamod
		typedef APIMeta::PluginInfo* PLID;

		mMetaApi.CallGameEntity = [](PLID plid, const char* entity, HL::entvars_t* vars) -> int {
			return gSharedAPI->CallGameEntity(entity, vars);
		};

		mMetaApi.GetUserMessageId = [](PLID plid, const char* message, int* size) {
			return gSharedAPI->GetUserMessageIndex(message, size);
		};

		mMetaApi.GetUserMessageName = [](PLID plid, int index, int* size) {
			return gSharedAPI->GetUserMessage(index, size);
		};

		mMetaApi.GetGameInfo = [](PLID plid, API::GameInfo info) {
			return gSharedAPI->GetGameInfo(info);
		};

		mMetaApi.MakeRequestId = [](PLID plid) {
			return gSharedAPI->GetUniqueRequestID();
		};
	}

	HL::globalvars_t* SharedAPI::GetEngineGlobals() {
		assert(mEngineGlobals != nullptr);
		return mEngineGlobals;
	}

	const char* SharedAPI::GetGameInfo(API::GameInfo info) {
		// Use a local variable with static duration so we don't return a temporary string to the caller
		static std::string result;

		// Ensure that we don't return an old path
		result.clear();

		switch(info) {
			case API::GameInfo::Name: /* Return the short name of the game (i.e the game directory, such as 'cstrike') */
				result = PathManager::GetInstance().GetPath(PathManager::GameRoot, PathManager::Naked);
				break;

			case API::GameInfo::Description: /* Return the description of the game (e.g 'Counter-Strike') */
				result = mGameLibrary->GetGameDescription();
				break;

			case API::GameInfo::Directory: /* Return the full pathname to the game directory (e.g 'C:/SteamCMD/cs/cstrike') */
				result = PathManager::GetInstance().GetPath(PathManager::GameRoot, PathManager::Absolute);
				break;

			case API::GameInfo::LibraryPath: /* Return the full path to the game library (e.g 'C:/SteamCMD/cs/cstrike/dlls/mp.dll') */
				result = mGameLibrary->GetPath();
				break;

			case API::GameInfo::LibraryFilename: /* Return the file name of the game library file (e.g 'mp.dll', 'cs.dylib', 'cs.so') */
				result = fs::path(mGameLibrary->GetPath()).filename().string();
				break;

			default: /* We just return a null pointer and hope for the best in case the plugins requests invalid information */
				std::cerr << format("[ERROR] A plugin requested invalid game information (%d)\n") % static_cast<int>(info);
				return nullptr;
		}

		return result.c_str();
	}

	size_t SharedAPI::GetUserMessageCount() {
		if(*mUserMessage == nullptr) {
			std::cerr << format("[WARNING] A plugin called 'GetUserMessageCount' before the engine was initialized");
			return 0;
		}

		// The message id for the 'top' one, equals the message count
		return (*mUserMessage)->index;
	}

	int SharedAPI::GetUserMessageIndex(const char* name, int* size) {
		if(name == nullptr || std::strlen(name) == 0) {
			std::cerr << format("[WARNING] A plugin called 'GetUserMessageIndex' with invalid arguments");
			return 0;
		}

		for(HL::UserMessage* message = *mUserMessage; message; message = message->prev) {
			if(std::strcmp(message->name, name) != 0) {
				continue;
			}

			if(size != nullptr) {
				*size = message->size;
			}

			return message->index;
		}

		return 0;
	}

	const char* SharedAPI::GetUserMessage(int index, int* size) {
		for(HL::UserMessage* message = *mUserMessage; message; message = message->prev) {
			if(message->index != index) {
				continue;
			}

			if(size != nullptr) {
				*size = message->size;
			}

			// Let's pray that the engine doesn't move this memory
			return message->name;
		}

		return nullptr;
	}

	const char* SharedAPI::GetPluginPath(API::PluginId id) {
		auto plugin = mPluginManager->FindPlugin(id);

		if(plugin) {
			return plugin->GetPath().c_str();
		} else {
			return nullptr;
		}
	}

	bool SharedAPI::CallGameEntity(const char* entity, HL::entvars_t* vars) {
		return this->CallEntity(entity, vars, mGameLibrary.get(), "CallGameEntity");
	}

	bool SharedAPI::CallPluginEntity(const char* entity, HL::entvars_t* vars) {
		return this->CallEntity(entity, vars, mPluginManager.get(), "CallPluginEntity");
	}

	const API::Meta::MetaAPI& SharedAPI::GetMetaAPI() const {
		return mMetaApi;
	}

	// Since 'CallGameEntity' and 'CallPluginEntity' methods are more or less identical, we comply to the DRY principle by using this method
	bool SharedAPI::CallEntity(const char* entity, HL::entvars_t* vars, Interface::IEntityExporter* exporter, const std::string& function) {
		if(entity == nullptr || vars == nullptr) {
			std::cerr << format("[WARNING] A plugin called '%s' with invalid arguments\n") % function;
			return false;
		}

		HL::FNEntity fnEntity = exporter->GetEntity(entity);

		if(fnEntity == nullptr) {
			return false;
		}

		fnEntity(vars);
		return true;
	}

	int SharedAPI::GetUniqueRequestID() {
		// The offset is to distinguish requests from the game library
		return std::abs(0xBEEF << 16) + (++mRequestCounter);
	}

	bool SharedAPI::AddServerCommand(const char* command, API::FastFunc<void()> callback) {
		return false;
	}
} }