#version 330 core
layout (location = 0) in float dt;   // delta time
layout (location = 1) in float prevPoint;

// Diff eq. parameter uniforms
uniform float a;
uniform float b;
uniform float c;
// Matrix uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * solve_lorenz();
}

vec3 solve_lorenz()
{
    float x = prevPoint.x;  float y = prevPoint.y;  float z = prevPoint.z;
    float newX = a * (y - x) * dt;
    float newY = (x * (b - z) - y) * dt;
    float newZ = (x*y - c*z) * dt;
    return vec3(newX, newY, newZ);
}