#version 330 core
out vec4 color;
in vec3 vertex_pos;
in vec2 vertex_tex;

uniform sampler2D tex;

void main()
{

color.rgb = texture(tex, vertex_tex).rgb;
color.a = clamp(length(color.rgb), 0, 1.0);
}
