#version 330 core

in vec3 lightPosition;
in vec3 cameraPosition;
in vec3 interpolatedNormal;

out vec4 color;

void main() {	
	vec3 LightIntensity = vec3(0.8f, 0.8f, 0.8f);
	float strength = 0.4f;
	float Shininess = 2.0f;
	
	vec3 Kd = vec3(0.7f, 0.7f, 0.7f);                
	vec3 Ka = vec3(0.1f, 0.1f, 0.1f);                
	vec3 Ks = vec3( 0.2f, 0.2f, 0.2f);	
	
	vec3 norm = normalize( interpolatedNormal );			
	vec3 viewDir = normalize(cameraPosition - gl_FragCoord.xyz);	
	vec3 lightDir = normalize( lightPosition - gl_FragCoord.xyz);
	vec3 reflectDir = reflect( -lightDir, norm );

	vec3 LI = LightIntensity * 
		(Kd * max( dot(lightDir, norm), 0.0 ))
		+ Ks * pow( max( dot(reflectDir, viewDir), 0.0 ), Shininess) * strength;
	
	color = vec4(LI, 1.0);
}


