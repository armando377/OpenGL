#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 ourColor;
uniform float offset;
uniform vec3 robotPosition;
void main()
{
    gl_Position = vec4(aPos.x + robotPosition.x, aPos.y + robotPosition.y, aPos.z, 1.0);
    ourColor = aColor;
}