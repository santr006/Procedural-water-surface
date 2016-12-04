#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;

uniform mat4 MV;
uniform mat4 P;
uniform float time;
uniform vec3 light;
uniform vec3 camera;
uniform float stepSize;

out vec3 lightPosition;
out vec3 cameraPosition;
out vec3 interpolatedNormal;
//out vec2 st;
//out vec3 fPos;

//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
  }

// ======================================================================

vec3 displacementForSingleWave(vec2 x0)
{
	//three waves
	float waveLength = 2;
	float waveLength2 = 2;
	float waveLength3 = 2;
	float A = 0.4;
	float A2 = 0.4;
	float A3 = 0.4;
	vec2 k = vec2(2 * 3.14 / waveLength, 0);//direction
	vec2 k2 = vec2(3.14 / waveLength, 3.14 / waveLength);//direction
	vec2 k3 = vec2(0.5 * 3.14 / waveLength, 1.5 * 3.14 / waveLength);//direction
	float w = 3.14;//speed
	float w2 = 3.14;//speed
	float w3 = 3.14;//speed
	float phi = 0;//förskjutning
	float phi2 = 3.14 / 2;//förskjutning
	float phi3 = 0;//förskjutning

	vec2 x = x0 - (k / k.length()) * A * sin(k * x0 - w * time + phi)
				/*- (k2 / k2.length()) * A2 * sin(k2 * x0 - w2 * time + phi2)
				- (k3 / k3.length()) * A3 * sin(k3 * x0 - w3 * time + phi3)*/;
	float y = A * cos(dot(k, x0) - w * time + phi)
			/*+ A2 * cos(dot(k2, x0) - w2 * time + phi2)
			+ A3 * cos(dot(k3, x0) - w3 * time + phi3)*/;

	return vec3(x.x, y, x.y);
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

