#version 330

layout (std140) uniform Material {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform	sampler2D texUnit;

in vec3 Normal;
in vec2 TexCoord;




uniform vec3 mainColor;
uniform float specularExponent;
uniform vec3 specularColor;
uniform sampler2D mainTexture;
uniform mat3 _dLight;
uniform vec3 _ambient;

void getDirectionalLight(vec3 normal, mat3 dLight, float specularExponent, out vec3 diffuse, out float specular){
    vec3 ecLightDir = dLight[0]; // light direction in eye coordinates
    vec3 colorIntensity = dLight[1];
    vec3 halfVector = dLight[2];
    float diffuseContribution = max(dot(normal, ecLightDir), 0.0);
    float specularContribution = max(dot(normal, halfVector), 0.0);
    specular =  pow(specularContribution, specularExponent);
	diffuse = (colorIntensity * diffuseContribution);
}

void main(void)
{
    vec3 diffuse;
    float specular;
    getDirectionalLight(normalize(Normal), _dLight, specularExponent, diffuse, specular);
    vec3 color = max(diffuse,_ambient.xyz)*mainColor;
    
    gl_FragColor = texture2D(mainTexture,TexCoord)*vec4(color, 1.0)+vec4(specular*specularColor,0.0);
}
 
 
 