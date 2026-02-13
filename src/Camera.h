#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "InputSystem.h"
#include <iostream>



// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {

public:
	enum Camera_Movement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	// camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// euler Angles
	float Yaw;
	float Pitch;
	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	double idleTime;

	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ); // Position as seen in above diagram
		WorldUp = glm::vec3(upX, upY, upZ); // Up vector as seen in above diagram
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	glm::mat4 GetViewMatrix()
	{
		// eye=Position, center=lookatdirection, up=what is defined as up
		// eye is where the camera is located
		// front is where you're looking at
		// up is up vector
		return glm::lookAt(Position, Position + Front, Up);
	}

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
	}

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	void updateCamera(Actions actions, double deltaTime) {
		if (actions.moveForward > 0.0) {
			ProcessKeyboard(FORWARD, 0.1 * deltaTime*actions.moveForward);
		}

		if (actions.moveBackward > 0.0) {
			ProcessKeyboard(BACKWARD, 0.1 * deltaTime*actions.moveBackward);
		}

		if (glm::abs(actions.xRotation) > 0.0) {
			ProcessMouseMovement(actions.xRotation, 0.0);
		}

		if (glm::abs(actions.yRotation) > 0.0) {
			ProcessMouseMovement(0.0, -actions.yRotation);
		}
	}

	void updateCamera(glm::vec3 targetPos, glm::vec3 forwardD, float camXRot, double t, bool cameraReset) {
		float followingDistance = 3.0f;
		glm::vec3 cameraHeight = glm::vec3(0.0f, 0.5f, 0.0f);

		// front is where the camera is facing
		// front + position is where the camera is looking

		// we want the camera front and the car front to be the same
		// great that is easy, make the front vector of the camera the front vector of the car

		// but where does the camera go now?
		// well it would go behind the car in the direction opposite to front
		// in otherwords cameraPos = targetPos + (-forwardD)
		// since forwardD is a unit vector, this will make it go directly 1 unit behind the car
		// we can add a following distance d
		// and then add our height

		Yaw += camXRot*100*t;
		Yaw = glm::clamp(Yaw, -89.0f, 89.0f);

		
		// if camera reset setting is toggled on
		if (cameraReset) {
			// if camera is not moving
			if (camXRot == 0.0f) {
				// increment idle time
				idleTime += t;
				// if idle time exceeds two seconds
				if (idleTime > 2.0f) {
					// smoothly bring it back
					Yaw = glm::mix(Yaw, 0.0f, t);

					// if less than 0.5 degrees, it's pretty much 0
					if (glm::abs(Yaw) < 0.5f) Yaw = 0.0;
				}
			}
			else {
				// if camera is moving reset idle time to 0
				idleTime = 0.0;
			}
		}
		

	

		glm::vec3 newForward = glm::vec4(forwardD, 0.0f)*glm::rotate(glm::mat4(1.0f), glm::radians(Yaw), WorldUp);

		Position = targetPos - newForward * followingDistance + cameraHeight;

		
		
		updateCameraVectors(newForward);
	}

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
	
	void updateCameraVectors(glm::vec3 front) {
		Front = glm::normalize(front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};