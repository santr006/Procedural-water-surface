//fft returns the amplitude and phase/offset as the real resp. the im part of a complex number
//http://stackoverflow.com/questions/10304532/why-does-fft-produce-complex-numbers-instead-of-real-numbers

#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;
layout(location = 3) in vec2 Height;

uniform mat4 MV;
uniform mat4 P;
uniform float time;
uniform vec3 light;
uniform vec3 camera;
uniform float stepSize;
uniform vec2 waveDirections[10];
uniform vec2 waveHeights[10];
uniform	vec2 windDir;
uniform	float windSpeed;
uniform	float amplitude;

out vec3 lightPosition;
out vec3 cameraPosition;
out vec3 interpolatedNormal;
//out vec2 st;
//out vec3 fPos;

float g = 9.82;

float atan2(in float y, in float x)
{
    return x == 0.0 ? sign(y)*1.57 : atan(y, x);
}

float positionHeight(vec2 pos){
	vec2 height = vec2(0,0);
	//float height = 0;

	for(int i = 0; i < 10; i++){
		float wh = waveHeights[i];
		//The dot product tells you what amount of one vector goes in the direction of another
		float dotTerm = dot(waveDirections[i], pos);
		float cosTerm = cos(dotTerm);
		float sinTerm = sin(dotTerm);
		//float xTerm = wh.x * cosTerm - wh.y * sinTerm;
		//float yTerm = wh.y * cosTerm + wh.x * sinTerm;
		height += cosTerm * sin(3.14 + sinTerm);
		//height += vec2(xTerm, yTerm);
		//height += wh * 
	}

	return sqrt(pow(height.x, 2) + pow(height.y, 2)) * sin(3.14 + atan2(height.y, height.x));
	//return height;
}

vec3 displacementForSingleWave(vec2 x0)
{
	return vec3(x0.x, Height.x, x0.y);
}

vec3 calculateNormal()
{
	vec3 posMid = Position + displacementForSingleWave(vec2(Position.x, Position.z));//0.05*Normal*snoise(40.0*Position + time * 0.5);
	vec3 posUp = Position + displacementForSingleWave(vec2(Position.x, Position.z-stepSize));//vec3(0,0,-stepSize) + 0.05*Normal*snoise(40.0*(Position + vec3(0,0,-stepSize)) + time * 0.5);
	vec3 posDown = Position + displacementForSingleWave(vec2(Position.x, Position.z+stepSize));//vec3(0,0,stepSize) + 0.05*Normal*snoise(40.0*(Position + vec3(0,0,stepSize)) + time * 0.5);
	vec3 posLeft = Position + displacementForSingleWave(vec2(Position.x-stepSize, Position.z));//vec3(-stepSize,0,0) + 0.05*Normal*snoise(40.0*(Position + vec3(-stepSize,0,0)) + time * 0.5);
	vec3 posRight = Position + displacementForSingleWave(vec2(Position.x+stepSize, Position.z));//vec3(stepSize,0,0) + 0.05*Normal*snoise(40.0*(Position + vec3(stepSize,0,0)) + time * 0.5);

	vec3 vecUp = normalize(posUp - posMid);
	vec3 vecDown = normalize(posDown - posMid);
	vec3 vecLeft = normalize(posLeft - posMid);
	vec3 vecRight = normalize(posRight - posMid);

	vec3 normNE = cross(vecUp, vecRight);
	vec3 normNW = cross(vecLeft, vecUp);
	vec3 normSW = cross(vecDown, vecLeft);
	vec3 normSE = cross(vecUp, vecDown);

	return normalize(normNE + normNW + normSW + normSE);
}

void main(){
	vec3 pos = Position + displacementForSingleWave(vec2(Position.x, Position.z));//0.05*Normal*snoise(40.0*Position + time * 0.5);
	gl_Position = (P * MV) * vec4(pos, 1.0);

	vec4 temp = (P * MV) * vec4(light, 1.0);
	lightPosition = temp.xyz;

	temp = (P * MV) * vec4(camera, 1.0);
	cameraPosition = temp.xyz;

	temp = (P * MV) * vec4(calculateNormal(), 1.0);
	interpolatedNormal = temp.xyz;

	//fPos = pos;
	//st = TexCoord;
}

