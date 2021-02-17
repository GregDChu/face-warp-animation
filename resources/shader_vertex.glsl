#version 330 core
layout(location = 0) in vec3 face;
layout(location = 1) in vec3 happy;
layout(location = 2) in vec3 sad;
layout(location = 3) in vec3 mad;
layout(location = 4) in vec3 scared;
layout(location = 5) in vec3 vertNor;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform float t;
uniform vec3 faceState;
uniform vec3 anchors[8];
uniform vec3 vertOffset[8];
uniform int selectedAnchor;

out vec3 vertex_pos;
out vec3 vertex_normal;
out float highlight;
void main()
{
	vec3 faceAPos = face;
	vec3 faceBPos = face;
	if(faceState.x == 0){ faceAPos = happy; }
	else if(faceState.x == 1){faceAPos = sad;}
	else if(faceState.x == 2){faceAPos = mad;}
	else if(faceState.x == 3){faceAPos = scared;}
	if(faceState.z == 0){faceBPos = happy;}
	else if(faceState.z == 1){faceBPos = sad;}
	else if(faceState.z == 2){faceBPos = mad;}
	else if(faceState.z == 3){faceBPos = scared;}
	float v = pow(faceState.y * 0.1, 5);
	vec3 emotPos = (faceAPos * (1.0 - v)) + (faceBPos * v);
	vec3 pos = (face * (1.0 - t)) + (emotPos * t);

	for(int anchor = 0; anchor < 8; anchor++){
		float dist = distance(pos, anchors[anchor]);
		dist = clamp(dist, 0, 0.5);
		float weight = 1.0 - (dist / 0.5);
		weight = pow(weight, 3);
		pos += (vertOffset[anchor] * weight);
		// Highlight selected area
		if(anchor == selectedAnchor)
		{
			highlight = clamp(weight, 0, 1.0);
		}
	}

	vertex_normal = vec4(M * vec4(vertNor,0.0)).xyz;
	vec4 tpos =  M * vec4(pos, 1.0);
	vertex_pos = tpos.xyz;
	gl_Position = P * V * tpos;
}
