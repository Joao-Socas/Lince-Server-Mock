#version 460 core
out vec4 FragColor;

in vec2 TextureCoords;
in vec3 FragmentPosition;
in vec3 Normal;
in vec3 TangentLightPosition;
in vec3 TangentViewPosition;
in vec3 TangentFragPosition;
in vec3 tangent_directional_light_direction;

uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform sampler2D Texture2;
uniform sampler2D Texture3;
uniform sampler2D Texture4;
uniform sampler2D Texture5;
uniform sampler2D Texture6;
uniform sampler2D Texture7;
uniform sampler2D Texture8;
uniform sampler2D Texture9;
uniform sampler2D Texture10;
uniform sampler2D Texture11;
uniform sampler2D Texture12;
uniform sampler2D Texture13;
uniform sampler2D Texture14;

uniform int NTextureAlbedo;
uniform int NTextureNormal;
uniform int NTextureMetalic;
uniform int NTextureRoughness;
uniform int NTextureAmbientOclusion;


uniform float shininess;
uniform vec3 specular_strength;

uniform vec3 light_1_position;
uniform vec3 light_1_color;
uniform float light_1_strength;

uniform vec3 directional_light_direction;
uniform vec3 directional_light_color;

layout(location = 0) out vec4 diffuseColor0;

void main()
{
    //
    vec3 light_fragment = TangentLightPosition - TangentFragPosition;
    float light_distancy = length(light_fragment);
    float squared_light_radius = 10.f;

    float attenuation_helper = light_distancy * light_distancy + squared_light_radius;
    float attenuation = 2 / (light_distancy*(sqrt(attenuation_helper)) + attenuation_helper);

    // simple difuse light_1
    vec3 color = texture(Texture0, TextureCoords).rgb;
    //vec3 lightDirection = normalize(light_1_position - FragmentPosition);
    //float Perpendicularity = max(dot(lightDirection, Normal), 0.0);
    //vec3 diffuse = (light_1_color * color) * (Perpendicularity);

    // normal map difuse
    vec3 normalTexture = texture(Texture1, TextureCoords).rgb;
    normalTexture = normalize(normalTexture * 2.0 - 1.0); // 0 -> -1  |  0.5 -> 0  |  1 -> 1
    vec3 tangent_lightDirection = normalize(light_fragment);
    float tangent_Perpendicularity = max(dot(tangent_lightDirection, normalTexture), 0.0);
    vec3 normal_diffuse = (light_1_color * color) * (tangent_Perpendicularity);
    float light_multiplier = light_1_strength * attenuation;
    normal_diffuse = normal_diffuse * light_multiplier;

    // normal map diffuse directional
    tangent_Perpendicularity = max(dot(tangent_directional_light_direction, normalTexture), 0.0);
    vec3 normal_diffuse_directional = (directional_light_color * color) * (tangent_Perpendicularity);

    vec3 viewDir    = normalize(TangentViewPosition - TangentFragPosition);
    vec3 halfwayDir = normalize(tangent_lightDirection + viewDir);
    float specularity = pow(max(dot(normalTexture, halfwayDir), 0.0), 128);
    vec3 specular = specularity * texture(Texture2, TextureCoords).rgb;
    specular = specular * light_multiplier;

    // for depth testing
    float near = 0.1;
    float far = 100;
    float ndc = gl_FragCoord.z * 2.0 - 1.0; 
    float linearDepth = (2.0 * near * far) / (far + near - ndc * (far - near));	
    linearDepth = (linearDepth / 10);
    
    
    //FragColor = vec4(vec3(linearDepth), 1.0);
    //FragColor = vec4(specular + normal_diffuse, 1);
    //vec3 rgb = normal_diffuse + normal_diffuse_directional + specular;
    //float Y = 0.299 * rgb.x + 0.587 * rgb.y + 0.114 * rgb.z;
    //float U = -0.148 * rgb.x - 0.28886 * rgb.y + 0.436 * rgb.z;
    //float V = 0.615 * rgb.x - 0.51499 * rgb.y - 0.10001 * rgb.z;
    //FragColor = vec4(1);
    FragColor = vec4(normal_diffuse + normal_diffuse_directional + specular, 1);
    //FragColor = vec4(1.0,0.5,0.5, 1);

}