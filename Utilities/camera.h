/*
camera.h
========
This file provides the implementation of the `Camera` class, an abstract representation
of a camera used in OpenGL. The class handles input processing and calculates
Euler angles, vectors, and matrices required for rendering 3D scenes.

SOURCE:
This file originates from the LearnOpenGL GitHub repository, authored by Joey De Vries.
It has been updated on February 27, 2024, and is distributed under the CC BY-NC 4.0 license.

LICENSE:
The CC BY-NC 4.0 license permits non-commercial use, as long as proper credit is given.
For details, refer to:
- Human-readable license: https://creativecommons.org/licenses/by-nc/4.0/
- Full license text: https://creativecommons.org/licenses/by-nc/4.0/legalcode

RESPONSIBILITIES:
- Process input from keyboards, mouse movement, and scroll events.
- Calculate the view matrix using Euler angles and the `LookAt` matrix.
- Handle camera movement, orientation changes, and zoom level adjustments.
- Provide an abstraction layer for window-system-independent camera controls.

DEFAULT VALUES:
- Movement speed: 2.5 units
- Mouse sensitivity: 0.1
- Zoom: 45.0 degrees
- Initial yaw: -90.0 degrees
- Initial pitch: 0.0 degrees

NOTES:
- This implementation depends on GLM for mathematical operations and GLAD for
  OpenGL context management.
- The `Camera` class is versatile and can be used for both FPS-style and free-camera
  implementations.
- Modifications made for CS330 include Up and Down movement and removal of glad requirement.
*/



#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
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

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
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
        if (direction == UP)
            Position += Up * velocity;
        if (direction == DOWN)
            Position -= Up * velocity;
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

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        MovementSpeed -= (float)yoffset;
        if (MovementSpeed < 1.0f)
            MovementSpeed = 1.0f;
        if (MovementSpeed > 45.0f)
            MovementSpeed = 45.0f;
    }

    /* Added this to allow zooming on lmb, more functionality could be added for rmb too */
    void ProcessMousePress(bool leftButtonPressed, bool rightButtonPressed)
    {
        // This function can be used to handle mouse button presses if needed
        // Currently, it does not perform any action
        // leftButtonPressed and rightButtonPressed can be used to determine which button was pressed
        if (rightButtonPressed && Zoom != 45.0f)
        {
            // Handle left button press
            Zoom = 45.0f;
		}
        else if (rightButtonPressed && Zoom == 45.0f)
        {
	        // reset zoom to default
			Zoom = 80.0f;
        }
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
};
#endif