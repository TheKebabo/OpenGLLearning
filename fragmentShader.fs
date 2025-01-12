#version 330 core

// From vertex shader
in vec3 colToFrag;
in vec2 texCoordToFrag;

out vec4 FragColor;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, texCoordToFrag);
}