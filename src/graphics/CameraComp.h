#pragma once

#include <cstdio>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/trigonometric.hpp>

// all the data that a camera should need
struct CameraComp {

	int camNumber = 0;
	// values directly used to find view matrix
	glm::vec3 position; // current location of the camera (not its target)
	glm::vec3 up = glm::vec3(0, 1, 0);
	float fov = 50;
	glm::vec3 lookPos;

	// values for finding movement and stuff

	// how far away from the car to be
	float targetDist = 4;
	// angle of the camera from the forward direction of the car (degrees)
	float pitch = 45;
	float yaw = 0;
	// how far ahead of the car to look
	float lookAheadDist = 3;
	// how fast to go to target position [0,1]
	float posEasing = 0.2;
	float yawEasing = posEasing;


	// float angleSpeed = 90;

	glm::mat4 GetViewMatrix() {
		// eye=Position, center=lookatdirection, up=what is defined as up
		// eye is where the camera is located
		// front is where you're looking at
		// up is up vector
		// return glm::lookAt(position, carPosition - position, up);
		return glm::lookAt(position, lookPos, up);
	}

	// where camera wants to be based on car and camera rotation
	// this also updaes the look at position auomatically
	glm::vec3 targetPosition(glm::vec3 loc, glm::quat rot) {

		// transform from car model to global coordinates
		glm::mat4 carTransform = glm::toMat4(rot);

		glm::mat4 yawRot = glm::rotate(glm::mat4(1.f), glm::radians(-yaw),
									   glm::vec3(0.f, 1.f, 0.f));

		// look at position is only affected by the yaw
		glm::vec4 target = yawRot * glm::vec4(0, 0, lookAheadDist, 1);

		glm::vec4 offset =
			glm::rotate(yawRot, glm::radians(pitch), glm::vec3(1, 0, 0)) *
			glm::vec4(0, 0, -targetDist, 1);

		// printf("offset:[%f, %f, %f]\n", offset.x, offset.y, offset.z);
		// printf("look:[%f, %f, %f]\n", target.x, target.y, target.z);
		//
		lookPos = loc + glm::vec3(carTransform * target);
		offset = carTransform * offset;

		return loc + glm::vec3(offset);
	}
};
