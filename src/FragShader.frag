// FragShader.frag
#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

in float FurLength;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 Color;
uniform vec3 lightColor;
uniform sampler2D clothTexture;  
uniform int useTexture;  

uniform int isBackFace;

uniform int isFur; 
uniform float furLayer;
uniform float furShadowStrength;
uniform float furAlpha;

void main()
{
    vec3 norm = normalize(Normal);
    if (isBackFace == 1) {
        norm = -norm;
    }
    
    // ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.3;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 baseColor;
    if (useTexture == 1) {
        baseColor = texture(clothTexture, TexCoord).rgb;
    } else {
        baseColor = Color;
    }
    
    // Apply fur-specific processing
    float alpha = 1.0;
    
    if (isFur == 1) {
        // For fur, reduce alpha based on fur length to create tapered look
        alpha = 1.0 - FurLength * furAlpha;
        
        // Darken fur towards the tips
        baseColor *= (1.0 - FurLength * 0.2);
    }
    else {
        // For cloth, check if it should receive fur shadows
        // Calculate shadow value based on angle between normal and light direction
        // This simulates shadows cast by fur onto the cloth
        float shadowFactor = max(0.0, 1.0 - max(0.0, dot(norm, lightDir)));
        baseColor *= (1.0 - shadowFactor * furShadowStrength);
    }
    
    vec3 result = (ambient + diffuse + specular) * baseColor;
    // Prevent over-bright areas
    result = min(result, vec3(1.0));
    FragColor = vec4(result, alpha);
}