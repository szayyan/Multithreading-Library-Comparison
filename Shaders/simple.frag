#version 330 core
out vec3 color;
in vec3 fragment_color;

void main() 
{
	color = fragment_color;//texture2D(texture, uv);
}