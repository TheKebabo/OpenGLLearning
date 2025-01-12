#version 330 core

in vec3 ourColor;
out vec4 FragColor;

uniform float redVal;

void main()
{
    FragColor = vec4(redVal, ourColor.yz, 1.0f);
}