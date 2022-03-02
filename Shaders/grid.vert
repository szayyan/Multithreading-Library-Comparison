#version 330 core
layout(location = 0) in vec3 vertex_pos_model;
layout(location = 1) in int cell_data;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec3 in_normal;

uniform mat4 mvp;
uniform mat4 view;
uniform vec3 light_pos;
uniform int cell_dimension;
uniform float grid_spacing = 2;

out vec3 fragment_color;
out vec3 fragment_position;
out vec3 Normal_cameraspace;
out vec3 eye_dir_cameraspace;
out vec3 LightDirection_cameraspace;


void main() 
{
	if( cell_data == 0 )
	{
		gl_Position = vec4(-1);
		return;
	}

	int a = gl_InstanceID / (cell_dimension * cell_dimension);
	int b = gl_InstanceID % (cell_dimension * cell_dimension);
	int c = b / cell_dimension;
	int d = b % cell_dimension;

	fragment_position = vertex_pos_model+(vec3(d,c,a)*grid_spacing);

	gl_Position = mvp * vec4(fragment_position,1.0);
		

	vec3 vertex_position_cameraspace = ( view  * vec4(vertex_pos_model,1)).xyz + fragment_position;
	eye_dir_cameraspace = -vertex_position_cameraspace;

	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 LightPosition_cameraspace = ( view * vec4(light_pos,1)).xyz;
	LightDirection_cameraspace = LightPosition_cameraspace + eye_dir_cameraspace;
	
	// Normal of the the vertex, in camera space
	Normal_cameraspace = ( view * vec4(normalize(in_normal),0)).xyz + fragment_position;

	fragment_color = inColor; // adding ambient white light to scene
}