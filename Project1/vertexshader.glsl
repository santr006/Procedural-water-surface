//fft returns the amplitude and phase/offset as the real resp. the im part of a complex number
//http://stackoverflow.com/questions/10304532/why-does-fft-produce-complex-numbers-instead-of-real-numbers

#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;
layout(location = 3) in vec2 Height;

uniform mat4 MVP;
uniform vec3 light;
uniform vec3 camera;

out vec3 lightPosition;
out vec3 cameraPosition;
out vec3 interpolatedNormal;

void main(){
	vec3 pos = Position + vec3(0, Height.x, 0);
	gl_Position =  MVP * vec4(pos, 1.0);

	vec4 temp = MVP * vec4(light, 1.0);
	lightPosition = temp.xyz;

	temp = MVP * vec4(camera, 1.0);
	cameraPosition = temp.xyz;

	interpolatedNormal = Normal.xyz;
}

