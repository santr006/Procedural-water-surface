#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices=48) out;

uniform mat4 MV;
uniform mat4 P;

in vec3 v_interpolatedNormal[];
in vec2 v_st[];
in vec3 v_fPos[];
out vec3 interpolatedNormal;
out vec2 st;
out vec3 fPos;

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

struct myTriangle {
	vec4 vertecies[3];
	vec3 normals[3];
	vec2 uvs[3];
};

myTriangle[4] subdivide(myTriangle t){
	myTriangle list[4];
	
	vec4 pos1 = t.vertecies[0] + (t.vertecies[1] - t.vertecies[0]) / 2.0f;
	vec4 pos2 = t.vertecies[1] + (t.vertecies[2] - t.vertecies[1]) / 2.0f;
	vec4 pos3 = t.vertecies[2] + (t.vertecies[0] - t.vertecies[2]) / 2.0f;

	vec3 n1 = normalize(t.normals[0] + t.normals[1]);
	vec3 n2 = normalize(t.normals[1] + t.normals[2]);
	vec3 n3 = normalize(t.normals[2] + t.normals[0]);

	vec2 uv1 = t.uvs[0] + (t.uvs[1] - t.uvs[0]) / 2.0f;
	vec2 uv2 = t.uvs[1] + (t.uvs[2] - t.uvs[1]) / 2.0f;
	vec2 uv3 = t.uvs[2] + (t.uvs[0] - t.uvs[2]) / 2.0f;
	
	list[0].vertecies[0] = t.vertecies[0];
	list[0].vertecies[1] = pos1;
	list[0].vertecies[2] = pos3;
	list[0].normals[0] = t.normals[0];
	list[0].normals[1] = n1;
	list[0].normals[2] = n3;
	list[0].uvs[0] = t.uvs[0];
	list[0].uvs[1] = uv1;
	list[0].uvs[2] = uv3;
	
	list[1].vertecies[0] = pos1;
	list[1].vertecies[1] = t.vertecies[1];
	list[1].vertecies[2] = pos2;
	list[1].normals[0] = n1;
	list[1].normals[1] = t.normals[1];
	list[1].normals[2] = n2;
	list[1].uvs[0] = uv1;
	list[1].uvs[1] = t.uvs[1];
	list[1].uvs[2] = uv2;
	
	list[2].vertecies[0] = pos1;
	list[2].vertecies[1] = pos2;
	list[2].vertecies[2] = pos3;
	list[2].normals[0] = n1;
	list[2].normals[1] = n2;
	list[2].normals[2] = n3;
	list[2].uvs[0] = uv1;
	list[2].uvs[1] = uv2;
	list[2].uvs[2] = uv3;
	
	list[3].vertecies[0] = pos3;
	list[3].vertecies[1] = pos2;
	list[3].vertecies[2] = t.vertecies[2];
	list[3].normals[0] = n3;
	list[3].normals[1] = n2;
	list[3].normals[2] = t.normals[2];
	list[3].uvs[0] = uv3;
	list[3].uvs[1] = uv2;
	list[3].uvs[2] = t.uvs[2];

	return list;
}

void main()
{
	//create 4 new triangles in the old one
	myTriangle t;
	for(int i = 0; i < 3; i++){
		t.vertecies[i] = gl_in[i].gl_Position;
		t.normals[i] = v_interpolatedNormal[i];
		t.uvs[i] = v_st[i];
	}
	myTriangle list[4] = subdivide(t);

	myTriangle largeList[16];
	for(int i = 0; i < 4; i++){
		myTriangle tArr[4] = subdivide(list[i]);
		largeList[0 + i * 4] = tArr[0];
		largeList[1 + i * 4] = tArr[1];
		largeList[2 + i * 4] = tArr[2];
		largeList[3 + i * 4] = tArr[3];
	}

	for(int i = 0; i < 16; i++){
		for(int j = 0; j < 3; j++){
			vec3 Position = vec3(largeList[i].vertecies[j].x, largeList[i].vertecies[j].y, largeList[i].vertecies[j].z);
			gl_Position = vec4(Position + 0.05*largeList[i].normals[j]*snoise(40.0*Position), 1.0);
			interpolatedNormal = mat3(MV) * largeList[i].normals[j];
			st = largeList[i].uvs[j];
			fPos = vec3(gl_Position.x, gl_Position.y, gl_Position.z);
			gl_Position = (P * MV) * gl_Position;
			
			EmitVertex();
		}
		EndPrimitive();
	}
}