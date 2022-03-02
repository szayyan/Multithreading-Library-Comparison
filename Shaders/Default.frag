#version 330 core
out vec3 color;
in vec3 fragment_color;
in vec3 normal;
in vec3 frag_pos;

uniform vec3 light_pos;
uniform vec3 light_color;

void main() 
{

	// ambient strenght 0.1
	vec3 ambient = 0.1 * light_color;

	vec3 light_dir = normalize(light_pos-frag_pos);
	float diff = max(dot(normalize(normal),light_dir),0.0);
	vec3 diffuse = diff * light_color;

	color = fragment_color * (diffuse + ambient);//texture2D(texture, uv);
}