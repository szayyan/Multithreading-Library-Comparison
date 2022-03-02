#version 330 core
out vec3 color;

in vec3 fragment_color;
in vec3 fragment_position;
in vec3 Normal_cameraspace;
in vec3 eye_dir_cameraspace;
in vec3 LightDirection_cameraspace;

uniform vec3 light_pos;

void main() 
{
	vec3 light_color = vec3(1.0,1.0,1.0);
	float dist = length( light_pos - fragment_position );
	vec3 l = normalize( LightDirection_cameraspace );
	vec3 n = normalize( Normal_cameraspace );
	float cosTheta = clamp( dot( n,l ), 0,1 );
	vec3 eye = normalize(eye_dir_cameraspace);
	vec3 reflectt = reflect(-l,n);
	float cosAlpha = clamp( dot( eye,reflectt ), 0,1 );
	
	color = 		vec3(0.25,0.25,0.25) * fragment_color +
		fragment_color * light_color * 50.0f * cosTheta / (dist) +
		vec3(0.35,0.35,0.35) * light_color * 50.0f * pow(cosAlpha,5) / (dist);
}