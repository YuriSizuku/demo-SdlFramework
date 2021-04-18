#version 330 core
in VS_OUT{
    vec3 aPosition;
    vec2 aTexcoord;
    vec3 aNormal;
    vec3 aTangent;
    vec4 fragPos; // world pos
    mat3 worldTBN;
} fs_in;

layout (std140) uniform BLOCK1{
    float other;
};
uniform sampler2D defaultTexture;
uniform vec3 viewPos;
#define MAX_LIGHTS 20
uniform int lightsNum;
uniform struct Light
{
    vec4 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
    float cutoff, outerCutoff;
}lights[MAX_LIGHTS];
uniform struct MaterialPhong 
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float alpha;
}material;

out vec4 FragColor;

vec3 CalcDirectionLight(Light light, MaterialPhong m, vec3 normal, vec3 viewDirection)
{
    // Half-Lambert diffuse shading
    vec3 lightDirection = normalize(light.direction);
    float diff = max(dot(normal, -lightDirection), 0);
    // specular shading
    vec3 reflectDirection = reflect(lightDirection, normal);
    float spec = pow(max(dot(-viewDirection, reflectDirection), 0.f),m.shininess);
    // combine shading
    vec3 ambient = light.ambient * m.ambient;
    vec3 diffuse = light.diffuse * diff * m.diffuse;
    vec3 specular = light.specular *sign(diff)* spec * m.specular;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(Light light, MaterialPhong m, vec3 normal, vec3 viewDirection, vec3 fragPos)
{
    // Half-Lambert diffuse shading
    vec3 lightDirection = normalize(fragPos - light.position.xyz);
    float diff = max(dot(normal, -lightDirection), 0);
    // specular Blinn-Phong shading
    vec3 halfDirection = -normalize(lightDirection+viewDirection);
    float spec = pow(max(dot(normal, halfDirection), 0.f), m.shininess); 
    // attenuation
    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.f/(light.attenuation[0] + 
        light.attenuation[1]*distance + light.attenuation[1]*distance*distance);
    // combine shading
    vec3 ambient = light.ambient * m.ambient;
    vec3 diffuse = light.diffuse * diff * m.diffuse;
    vec3 specular = light.specular*sign(diff)*spec*m.specular; // no backside
    return (ambient + diffuse + specular)*attenuation;
}

vec3 CalcSpotLight(Light light, MaterialPhong m, vec3 normal, vec3 viewDirection, vec3 fragPos)
{
    // Half-Lambert diffuse shading
    vec3 lightDirection = normalize(fragPos - light.position.xyz);
    float diff = max(dot(normal, -lightDirection), 0);
    // specular Blinn-Phong shading
    vec3 halfDirection = -normalize(lightDirection+viewDirection);
    float spec = pow(max(dot(normal, halfDirection), 0.f), m.shininess);
    // attenuation
    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.f/(light.attenuation[0] + 
        light.attenuation[1]*distance + light.attenuation[1]*distance*distance);
    // spot intensity, I = 1 - (theta-outerCutoff)/(outerCutoff-cutoff)
    float theta = dot(lightDirection, normalize(light.direction));
    float epsilon = cos(light.cutoff) - cos(light.outerCutoff);
    float intensity = clamp((theta - cos(light.outerCutoff))/epsilon, 0.f, 1.f);
    // combine shading
    vec3 ambient = light.ambient * m.ambient;
    vec3 diffuse = light.diffuse * diff * m.diffuse;
    vec3 specular = light.specular *sign(diff)* spec * m.specular;
    return (ambient + diffuse + specular)*attenuation*intensity;
}

void main()
{
    vec3 texcolor = texture(defaultTexture, fs_in.aTexcoord).rgb; 
    vec3 outcolor = vec3(0.f);
    vec3 normal = fs_in.worldTBN[2];
    vec3 viewDirection = normalize(fs_in.fragPos.xyz - viewPos);
    MaterialPhong m = material;
    m.ambient *= texcolor;
    m.diffuse *= texcolor;
    for(int i=0;i<lightsNum;i++)
    {
       if(abs(lights[i].position.w) <= 0.001f)
        {
            outcolor += CalcDirectionLight(lights[i], m, normal, viewDirection);
        }
        else if(lights[i].cutoff>0.f) 
        {
            outcolor += CalcSpotLight(lights[i], m, normal, viewDirection, fs_in.fragPos.xyz);
        }
        else
        {
            outcolor += CalcPointLight(lights[i], m, normal, viewDirection, fs_in.fragPos.xyz);
        }
    }
    FragColor = vec4(outcolor, material.alpha);
    //FragColor = vec4(0.f, 1.f, 1.f, 1.f);
}