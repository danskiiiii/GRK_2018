#version 430 core

uniform vec3 objectColorA;
uniform vec3 objectColorB;
uniform float stripeWidth;
uniform vec3 lightDir;

in vec3 interpNormal;
in vec3 vertexPos;

void main()
{
	vec3 normal = normalize(interpNormal);
	float diffuse = max(dot(normal, -lightDir), 0.3);
	
	float t = (1 + sin(3.14 *vertexPos.z / stripeWidth) )/2;
	gl_FragColor = (1-t) * vec4(objectColorA *diffuse, 1.0) + t * vec4(objectColorB * diffuse, 1.0);

}
