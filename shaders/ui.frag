#version 330 core
in vec3 color;
out vec4 col;


void main()
{    
    col = vec4(color, 1.0);
}