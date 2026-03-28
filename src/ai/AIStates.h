#pragma once

#include "GameState.h"
#include "ai/AISparkComponents.h"
#include "vehicles/SparkComponents.h"
#include "ecs/System.h"
#include "world/CurveLoader.h"
#include "AIDrivingStates.h"



/*
* Base AI state class for states in the top level of the HFSM.
*/
class AIState {

public: 

	virtual void run(AIDriveContext& ctx) {};
	virtual std::pair<Direction, glm::vec3> detect(AIDriveContext& ctx) { return { NONE, glm::vec3(0.f) }; };
	
	void checkRoute(AIController& ai);
	std::unique_ptr<TrackCurve> path;
private:

};


/*
* Derived state classes.
* These are high-level states used by the ai sparks to determine the 'style' of driving to use.
*/
class DefenseState : public AIState {

public:
	void run(AIDriveContext& ctx) override;
	std::pair<Direction, glm::vec3> detect(AIDriveContext& ctx) override;
	
	std::unique_ptr<IDriveState> currentState = std::make_unique<S_Driving>();

	std::unique_ptr<TrackCurve> path;
};

class OvertakeState : public AIState {

public:
	void run(AIDriveContext& ctx) override ;
	std::pair<Direction, glm::vec3> detect(AIDriveContext& ctx) override;

	std::unique_ptr<IDriveState> currentState = std::make_unique<S_Driving>();

	std::unique_ptr<TrackCurve> path;
};

class MaintainState : public AIState {

public:
	void run(AIDriveContext& ctx) override;

	std::unique_ptr<IDriveState> currentState = std::make_unique<S_Driving>();

	std::unique_ptr<TrackCurve> path;
};

