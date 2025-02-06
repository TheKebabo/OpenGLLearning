#version 430 core

layout (local_size_x = 1) in;

layout (std430, binding = 0) buffer Pos {   // An SSBO (input & output of compute shader)
    vec4 Position[];
};


void main() {
    uint idX = gl_GlobalInvocationID.x;   // Current work group x position

    vec3 p = Position[idX].xyz; // grab input pos

    // p += vec3(0.001);   // update in some way

    Position[idX].xyz = p;  // set as output pos
}