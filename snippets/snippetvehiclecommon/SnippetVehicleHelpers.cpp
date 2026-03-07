// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2025 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#include <cstdio>
#include <ctype.h>
#include <iostream>

#include "../snippetvehiclecommon/SnippetVehicleHelpers.h"
#include "PxFiltering.h"
#include "debugUtils/Logger.h"

using namespace physx;

namespace snippetvehicle {
// docs:
// https://nvidia-omniverse.github.io/PhysX/physx/5.1.0/docs/RigidBodyCollision.html

// this is a filter shader for the entire scene, not just the vehicle!
// it should probably be moved to physics becuase its not really vehicle code
//
// its also probably important to note that this is like a 1st pass for
// collision, so things aren't always colliding. use the callbacks for actual
// collision logic
PxFilterFlags VehicleFilterShader(PxFilterObjectAttributes attributes0,
								  PxFilterData filterData0,
								  PxFilterObjectAttributes attributes1,
								  PxFilterData filterData1,
								  PxPairFlags &pairFlags,
								  const void *constantBlock,
								  PxU32 constantBlockSize) {
	PX_UNUSED(constantBlock);
	PX_UNUSED(constantBlockSize);

	// ignore wheel colisions because they break everything
	if (filterData0.word0 == COLLISION_FLAG_WHEEL ||
		filterData1.word0 == COLLISION_FLAG_WHEEL) {
		return PxFilterFlag::eSUPPRESS;
	}

	pairFlags = PxPairFlag::eCONTACT_DEFAULT;

	// if ((filterData0.word0 == COLLISION_FLAG_OBSTACLE &&
	// 	 filterData1.word0 == COLLISION_FLAG_CHASSIS) ||
	// 	(filterData0.word0 == COLLISION_FLAG_CHASSIS &&
	// 	 filterData1.word0 == COLLISION_FLAG_OBSTACLE)) {
	// 	pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
	// 	std::cout << "Collision Detected" << std::endl;
	// }

	// // finish line collision
	if ((filterData0.word0 == COLLISION_FLAG_FINISH &&
		 filterData1.word0 == COLLISION_FLAG_CHASSIS) ||
		(filterData0.word0 == COLLISION_FLAG_CHASSIS &&
		 filterData1.word0 == COLLISION_FLAG_FINISH)) {
		// notify callbacks to handle finish line
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
		dbug::log("PHYS", -1, "Finish line close");
	}

	// callback if chassis hit another chassis, or the ground/wall
	if ((filterData0.word0 == COLLISION_FLAG_CHASSIS &&
		 filterData1.word0 == COLLISION_FLAG_CHASSIS) ||
		(filterData0.word0 == COLLISION_FLAG_GROUND &&
		 filterData1.word0 == COLLISION_FLAG_CHASSIS) ||
		(filterData0.word0 == COLLISION_FLAG_CHASSIS &&
		 filterData1.word0 == COLLISION_FLAG_GROUND)) {
		// notify callbacks to handle crashing into a wall
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
		// dbug::log("GAME", 0, "BONK?");
	}

	// done
	return PxFilterFlags();
}

bool parseVehicleDataPath(int argc, const char *const *argv,
						  const char *snippetName,
						  const char *&vehicleDataPath) {
	if (argc != 2 || 0 != strncmp(argv[1], "--vehicleDataPath",
								  strlen("--vehicleDataPath"))) {
		printf("%s usage:\n"
			   "%s "
			   "--vehicleDataPath=<path to the [PHYSX_ROOT]/assets/vehicledata "
			   "folder containing the vehiclejson files to be loaded> \n",
			   snippetName, snippetName);
		return false;
	}
	vehicleDataPath = argv[1] + strlen("--vehicleDataPath=");
	return true;
}

} // namespace snippetvehicle
