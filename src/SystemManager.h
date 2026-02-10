#include "EntityManager.h"
#include <unordered_map>
#include "System.h"

class SystemManager {
public:

	// system needs to register
	template<typename T>
	std::shared_ptr<T> RegisterSystem();

	// system set signature
	template<typename T>
	void SetSignature(Signature signature);

	// erase entity in all system entity lists
	void EntityDestroyed(Entity entity);

	// notify each system that an entity signature has changed
	void EntitySignatureChanged(Entity entity, Signature entitySignature);
	
private:
	// map from system string pointer to a signature
	std::unordered_map<const char*, Signature> signatures{};

	// map from string system pointer to system pointer
	std::unordered_map<const char*, std::shared_ptr<System>> systems{};
};