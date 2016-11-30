#version 120

uniform float time;
varying vec3 v_texCoord3D;

void main( void )
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    v_texCoord3D = gl_Vertex.xyz;
}

/*// Procedural shading demo, Stefan Gustavson 2011.
// This code is in the public domain.

uniform float texw, texh;
varying vec2 dims;  // In absence of textureSize()
varying vec2 one;
varying vec2 st;

void main( void )
{
  // Get the texture coordinates
  gl_TexCoord[0] = gl_MultiTexCoord0;
  dims = vec2(texw, texh);
  one = 1.0 / dims; // Saves two divisions in the fragment shader
  gl_Position = ftransform();
}*/
