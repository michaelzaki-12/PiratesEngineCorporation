#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "GLFW/glfw3.h"
// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;

class Camera {
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Right;
    glm::vec3 Up;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
    {
        Position = position;
        Up = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    void ProcessKeyboard(GLFWwindow* window, float deltaTime);
    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix();
    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(GLFWwindow* winodw, double xposIn, double yposIn, bool ISbound = true);
    float width = 0, height = 0;
private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();
    bool firstMouse = true;
    float lastX = width / 2, lastY = height / 2;
    
};
