#version 330 core

// From vertex shader
in vec3 colToFrag;
in vec2 texCoordToFrag;

out vec4 fragColor;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    fragColor = mix(texture(texture1, texCoordToFrag), texture(texture2, texCoordToFrag), 0.2);
}