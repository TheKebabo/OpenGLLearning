#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum CameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  3.25f;
const float SENSITIVITY =  0.05f;
const float FOV         =  45.0f;

using namespace glm;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    vec3 Pos;
    vec3 Forward;
    vec3 Up;
    vec3 Right;
    vec3 WorldUp;
    // euler Angles
    vec2 YawPitch;
    // camera options
    float MoveSpeed;
    float Fov;
    
    struct {
        vec2 PrevPos;
        float Sensitivity;
        bool First = true;  // Checks if mouse input has been recieved yet, or if it has only just entered the screen
    } Mouse;

    // constructor with vectors
    Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Forward(vec3(0.0f, 0.0f, -1.0f)), YawPitch(vec2(yaw, pitch)), MoveSpeed(SPEED), Fov(FOV)
    {
        Pos = position;
        WorldUp = up;
        Mouse.Sensitivity = SENSITIVITY;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Forward(vec3(0.0f, 0.0f, -1.0f)), YawPitch(vec2(yaw, pitch)), MoveSpeed(SPEED), Fov(FOV)
    {
        Pos = vec3(posX, posY, posZ);
        WorldUp = vec3(upX, upY, upZ);
        Mouse.Sensitivity = SENSITIVITY;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    mat4 GetViewMatrix()
    {
        // Note: we translate the scene in the reverse direction of where we want to move the camera, and the camera faces along the -VE z dir
        // i.e. if camera should move back by 3 units, so 3 units in the +ve z dir, we should translate by -3 units in the +ve z dir
        return lookAt(Pos, Pos + Forward, Up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(CameraMovement direction, float deltaTime)
    {
        float velocity = MoveSpeed * deltaTime;
        switch (direction)
        {
            case FORWARD: {
                vec3 xzNormal = WorldUp;
                vec3 forwardProject = Forward - dot(Forward, xzNormal) * xzNormal; // Vector in same dir projected onto xz plane
                Pos += velocity * normalize(forwardProject);
            } break;
            case BACKWARD: {
                vec3 xzNormal = WorldUp;
                vec3 forwardProject = Forward - dot(Forward, xzNormal) * xzNormal; // Vector in same dir projected onto xz plane
                Pos -= velocity * normalize(forwardProject);
            } break;
            case RIGHT:
                Pos += velocity * Right;
                break;
            case LEFT:
                Pos -= velocity * Right;
                break;
            case UP:    
                Pos += velocity * WorldUp;
                break;
            case DOWN:
                Pos -= velocity * WorldUp;
                break;
        };
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        vec2 curPos = vec2((float)xoffset, (float)yoffset);

        if (Mouse.First) // Initially true
        {
            Mouse.PrevPos = curPos;
            Mouse.First = false;
        }

        vec2 deltaMouse = (curPos - Mouse.PrevPos) * Mouse.Sensitivity;
        deltaMouse.y *= -1; // Reverse since y-coordinates range from bottom to top
        Mouse.PrevPos = curPos;

        YawPitch += deltaMouse;

        if (constrainPitch)
            YawPitch.y = clamp(YawPitch.y, -89.0f, 89.0f);  // Constrain pitch

        // update Forward, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Fov = clamp(Fov - (float)yoffset, 1.0f, 45.0f);
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        vec3 newForward;
        newForward.x = cos(radians(YawPitch.x)) * cos(radians(YawPitch.y));
        newForward.y = sin(radians(YawPitch.y));
        newForward.z = sin(radians(YawPitch.x)) * cos(radians(YawPitch.y));
        Forward = normalize(newForward);
        // also re-calculate the Right and Up vector
        Right = normalize(cross(Forward, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = normalize(cross(Right, Forward));
    }
};
#endif