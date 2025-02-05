#version 430 core

layout (local_size_x = 10, local_size_y = 10, local_size_z = 1) in;

// images
layout (rgba32f, binding = 0) uniform image2D imgOutput;    // internal format is the same as the host program's texutre format

// variables
layout (location = 0) uniform float time;
layout (location = 1) uniform float textureWidth;

void main() {
    vec4 colour = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
	
    float speedX = 100;
    float speedY = 100;

    colour.x = mod(float(texelCoord.x) + time * speedX, textureWidth) / (gl_NumWorkGroups.x * gl_WorkGroupSize.x);
    colour.y = mod(float(texelCoord.y) + time * speedY, textureWidth) / (gl_NumWorkGroups.y * gl_WorkGroupSize.y);
	
    imageStore(imgOutput, texelCoord, colour);
}