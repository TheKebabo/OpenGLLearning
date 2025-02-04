#version 430 core

in vec2 texCoordToFrag;

out vec4 fragColor;

uniform sampler2D tex;

void main()
{
    vec3 texCol = texture(tex, texCoordToFrag).rgb;
    fragColor = vec4(texCol, 1.0f);
}