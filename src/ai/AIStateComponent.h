#pragma once
#include "IDriveState.h"

// pointer to a sparks current drive state
struct AIDriveStateP {
	std::shared_ptr<IDriveState> currentState;
};