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
	

   if (sin(3.14 *vertexPos.y / stripeWidth) > 0){
		gl_FragColor = vec4(objectColorA *diffuse , 1.0);}
   else{
		gl_FragColor = vec4(objectColorB * diffuse, 1.0);}
		
	

	}
