#version 330 core
layout(location = 0) in vec3 vertex_pos_model;
layout(location = 1) in vec3 offset;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec3 inNormal;

uniform mat4 mvp;

out vec3 fragment_color;
out vec3 normal;
out vec3 frag_pos;

void main() 
{
	gl_Position = mvp * vec4(vertex_pos_model+offset,1.0);
	frag_pos = vertex_pos_model+offset;
	fragment_color = inColor;
	normal = inNormal;
}