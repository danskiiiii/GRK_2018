#version 430 core

uniform sampler2D textureSampler;
uniform vec3 lightDir;

in vec3 interpNormal;
in vec2 interpTexCoord;
in vec3 camPos;
in vec3 interpPosition;

void main()
{
	vec3 v = normalize(camPos - interpPosition);
	vec3 r = normalize(reflect(-lightDir, interpNormal));
	float specular = pow(max(dot(v, r), 0.0), 5);  //ostatnie to potega 

	vec2 modifiedTexCoord = vec2(interpTexCoord.x, 1.0 - interpTexCoord.y); //odwrocenie tekstur
	vec3 color = texture2D(textureSampler, modifiedTexCoord).rgb;
	vec3 normal = normalize(interpNormal);
	float diffuse = max(dot(normal, -lightDir), 0.3);
	gl_FragColor = vec4(color, 1.0)* diffuse + vec4(1.0) * specular;
}






	
