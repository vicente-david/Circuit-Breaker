#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

// all the data that a camera should need
struct CameraComp {

	glm::vec3 position; // current location of the camera (not its target)
	glm::vec3 carPosition;
	glm::vec3 carUp;
	glm::vec3 up; // this may need to change during hard banked turns and such

	// target location realitive to the target
	// (in the coordinate frame of the car)
	glm::vec3 targetOffset = glm::vec3(0.0f, 2.00f, -5.f);

	// how far ahead of the car to look
	float lookAtdistance = 2;

	// extra angle we have moved the camera (ex with the 2nd stick)
	float yaw = 0; // from [-1, 1]
	float pitch = 0; // from [-1, 1]

	// movement related data
	int camNumber = 0;
	float posEasing = 0.1;

	glm::mat4 GetViewMatrix() {
		// eye=Position, center=lookatdirection, up=what is defined as up
		// eye is where the camera is located
		// front is where you're looking at
		// up is up vector
		// return glm::lookAt(position, carPosition - position, up);
		return glm::lookAt(position, carPosition, up);
	}

	// where the camera wants to be
	// TODO: use car velocity, and joystick to influence the rotation (done but needs work)
	glm::vec3 targetPosition(glm::quat carRot) {

		// this is the distance the camera will be from the car
		// we transform the offset form model to global coordinates
		glm::mat4 carTransform = glm::toMat4(carRot);

		yaw = glm::clamp(yaw, -135.f, 135.f);
		pitch = glm::clamp(pitch, -30.f, 90.f);
		glm::mat4 yawRot = glm::rotate(glm::mat4(1.f), glm::radians(yaw), glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 pitchRot = glm::rotate(glm::mat4(1.f), glm::radians(pitch), glm::vec3(1.f, 0.f, 0.f));
		
		glm::vec3 rotatedOffset = glm::vec3(yawRot * glm::vec4(targetOffset, 1.f));
		rotatedOffset = glm::vec3(pitchRot * glm::vec4(rotatedOffset, 1.f));

		auto offset = carTransform * glm::vec4(rotatedOffset, 1);

		// getting the car's up vector doesn't seem to be working, so i just use 
		// the world up, idk
		carUp = glm::vec3(0,1,0);
		// carUp = glm::vec4(0,-1,0, 1) * carTransform;

		return carPosition + glm::vec3(offset);
	}
};
