#include "EntityManager.h"
#include <unordered_map>
#include "System.h"

class SystemManager {
public:

	// system needs to register
	template<typename T>
	std::shared_ptr<T> registerSystem();

	// system set signature
	template<typename T>
	void setSignature(Signature signature);

	// erase entity in all system entity lists
	void entityDestroyed(Entity entity);

	// notify each system that an entity signature has changed
	void entitySignatureChanged(Entity entity, Signature entitySignature);
	
private:
	// map from system string pointer to a signature
	std::unordered_map<const char*, Signature> signatures{};

	// map from string system pointer to system pointer
	std::unordered_map<const char*, std::shared_ptr<System>> systems{};
};