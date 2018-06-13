#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 2) in vec3 vertexNormal;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;
uniform vec3 cameraPos;

out vec3 interpPosition;
out vec3 interpNormal;
out vec2 interpTexCoord;
out vec3 camPos;

void main()
{
	gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1.0);
	interpNormal = (modelMatrix * vec4(vertexNormal, 0.0)).xyz;
	interpTexCoord = vertexTexCoord;
	camPos = cameraPos;
	interpPosition =  (modelMatrix * vec4(vertexPosition, 1.0)).xyz;

}



