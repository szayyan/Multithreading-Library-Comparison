#version 330 core
layout(location = 0) in vec3 model_vertex_pos;
layout(location = 1) in vec3 in_colour;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in mat4 instance_matrix;

uniform mat4 mvp;
uniform mat4 view;
uniform vec3 light_pos;

out vec3 fragment_color;
out vec3 fragment_position;
out vec3 Normal_cameraspace;
out vec3 eye_dir_cameraspace;
out vec3 LightDirection_cameraspace;

void main() 
{
	gl_Position = mvp * instance_matrix * vec4(model_vertex_pos,1.0);
	
	vec3 light_color = vec3(1.0,1.0,1.0);

	fragment_position = (instance_matrix * vec4(model_vertex_pos,1.0)).xyz;

	vec3 vertex_position_cameraspace = ( view * instance_matrix * vec4(model_vertex_pos,1)).xyz;
	eye_dir_cameraspace = -vertex_position_cameraspace;

	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 LightPosition_cameraspace = ( view * vec4(light_pos,1)).xyz;
	LightDirection_cameraspace = LightPosition_cameraspace + eye_dir_cameraspace;
	
	// Normal of the the vertex, in camera space
	Normal_cameraspace = ( view * instance_matrix * vec4(normalize(in_normal),0)).xyz;

	fragment_color = in_colour; // adding ambient white light to scene
}

