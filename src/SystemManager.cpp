#include "SystemManager.h"
#include <memory>

// system registration
template<typename T>
std::shared_ptr<T> SystemManager::RegisterSystem() {
	// todo asserts
	// get the system type
	const char* typeName = typeid(T).name();

	// creates a pointer to the system and returns that pointer
	auto system = std::make_shared<T>();
	systems.insert({ typeName, system });
	return system;

}

// system set signature
template<typename T>
void SystemManager::SetSignature(Signature signature) {
	const char* typeName = typeid(T).name();

	// insert {systemtype, signature for systemtype} 
	signatures.insert({ typeName, signature });
}

// erase entity in all system entity lists
void SystemManager::EntityDestroyed(Entity entity) {

	// for every pair of {systemType, ptr to system} 
	for (auto const& pair : systems) {
		// get the pointer
		auto const& system = pair.second;
		// erase that entity from the system
		system->entities.erase(entity);
	}

	// because entities is a set, there is no need to check for duplicate entities
}

// notify each system that an entity signature has changed
void SystemManager::EntitySignatureChanged(Entity entity, Signature entitySignature) {
	// for every pait of {systemType, ptr to system}
	for (auto const& pair : systems) {
		// system type
		auto const& type = pair.first;
		// ptr to system
		auto const& system = pair.second;
		// signature of system
		auto const& systemSignature = signatures[type];

		// recall signatures are bitfields
		// so we check if the entity's signature matches the system required signature
		if ((entitySignature & systemSignature) == systemSignature) {
			system->entities.insert(entity);
		}
		else {
			// if the entity signature doesn't match the signature required for the system
			// remove the entity
			system->entities.erase(entity);
		}

	}
}