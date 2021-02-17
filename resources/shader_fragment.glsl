#version 330 core
out vec4 color;

uniform vec3 campos;

in vec3 vertex_normal;
in vec3 vertex_pos;
in float highlight;

void main()
{
//color = vec4(1, 1, 0, 1);

	//The normal vector of the surface
	vec3 norm = normalize(vertex_normal);
	//The position of the light in world space
	vec3 light_source = vec3(-10, 10, 30);
	//The vector created by the point being drawn and the light source
	vec3 light_vec = normalize(light_source - vertex_pos);
	vec3 camera_vec = normalize(vertex_pos - campos);
	vec3 half_vec = normalize(camera_vec + light_vec);
	float specular = dot(norm, half_vec);
	float diffuse = dot(norm, light_vec);

	vec3 light_2 = vec3(20, 30, 50);
	vec3 light_vec2 = normalize(light_2 - vertex_pos);
	float diffuse2 = dot(norm, light_vec2);

	vec3 light_3 = vec3(10, 0, -5);
	vec3 light_vec3 = normalize(light_3 - vertex_pos);
	float diffuse3 = dot(norm, light_vec3);

	if(vertex_pos.z < -0.1)
		discard;
	specular = clamp(specular, 0, 1.0);
	specular = pow(specular, 300);
	float ambient = clamp(diffuse3, 0.5, 1.0);

	color = (vec4(1.0, 1.0, 1.0, 0.5) * 0.8) * ambient;
	color += (vec4(0.5, 1.0, 1.0, 0.5) * 1.3) * diffuse;
	color += (vec4(1.0, 0.2, 0.2, 0.5) * 1.3) * diffuse2;

	color += (vec4((vec3(1, 1, 1) * specular), 1 * specular) * 0.5);
	color = vec4(color.rgb * ambient * 0.7, 1);
	color.rgb += vec3(0, 0.75, 0.75) * highlight;
}
