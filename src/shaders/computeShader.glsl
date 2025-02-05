#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;   // this shader is executed for every pixel of image (1 px per work group)

// images
layout (rgba32f, binding = 0) uniform image2D imgOutput;    // internal format is the same as the host program's texutre format

// variables
layout (location = 0) uniform float time;
layout (location = 1) uniform float textureWidth;

void main() {
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
	
    float speed = 100;

    value.x = mod(float(texelCoord.x) + t * speed, textureWidth) / (gl_NumWorkGroups.x);
    value.y = float(texelCoord.y)/(gl_NumWorkGroups.y);
	
    imageStore(imgOutput, texelCoord, value);
}