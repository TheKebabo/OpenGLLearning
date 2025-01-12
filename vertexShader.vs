#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec2 aTexCoord;

// Both passed to fragment shader
out vec3 colToFrag;
out vec2 texCoordToFrag;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    colToFrag = aCol;
    texCoordToFrag = aTexCoord;
}