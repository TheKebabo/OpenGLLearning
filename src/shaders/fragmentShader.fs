#version 430 core

out vec4 fragColor;

uniform vec4 color = vec4(1.0);

void main()
{
    fragColor = color;
}