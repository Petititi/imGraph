#version 330
 
 uniform Matrices {
 	mat4 projMatrix;
 	mat4 modelViewMatrix;
 };
 
 in vec3 position;
 in vec3 normal;
 in vec2 texCoord;
 
 out vec4 vertexPos;
 out vec2 TexCoord;
 out vec3 Normal;
 
 void main()
 {
 	Normal = normalize(vec3(modelViewMatrix * vec4(normal,0.0)));	
 	TexCoord = vec2(texCoord);
 	gl_Position = projMatrix * modelViewMatrix * vec4(position,1.0);
 }