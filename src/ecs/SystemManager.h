#include "EntityManager.h"
#include <memory>
#include <unordered_map>
#include "System.h"

class SystemManager {
public:

	// system registration
	template<typename T>
	std::shared_ptr<T> registerSystem() {
		// todo asserts
		// get the system type
		const char* typeName = typeid(T).name();
		// printf("registering system %s\n", typeName);

		// creates a pointer to the system and returns that pointer
		auto system = std::make_shared<T>();
		systems.insert({ typeName, system });
		return system;

	}

	// system set signature
	template<typename T>
	void setSignature(Signature signature) {
		const char* typeName = typeid(T).name();

		// insert {systemtype, signature for systemtype} 
		signatures.insert({ typeName, signature });
	}


	// erase entity in all system entity lists
	void entityDestroyed(EcsEntity entity) {

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
	// notify each system that an entity signature has changed
	void entitySignatureChanged(EcsEntity entity, Signature entitySignature) {
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
	
private:
	// map from system string pointer to a signature
	std::unordered_map<const char*, Signature> signatures{};

	// map from string system pointer to system pointer
	std::unordered_map<const char*, std::shared_ptr<System>> systems{};
};
