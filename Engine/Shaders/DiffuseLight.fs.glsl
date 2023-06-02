// FRAGMENT SHADER

// Texture Sampling
vec3 color = texture(color_texture, texture_coords).rgb;
vec3 normal = texture(normal_texture, texture_coords).rgb;
normal = normalize(normal_texture * 2.0 - 1.0); // 0 -> -1  |  0.5 -> 0  |  1 -> 1

// Common parameters calculation
vec3 light_fragment = light_position - frag_position;
vec3 light_distancy = length(light_fragment);
vec3 light_direction = normalize(light_fragment);
float attenuation_helper = light_distancy * light_distancy + squared_light_radius;
float attenuation = 2 / (light_distancy*(sqrt(attenuation_helper)) + attenuation_helper);

// Specific diffuse calculation
float perpendicularity = max(dot(light_direction, normalTexture), 0.0);
vec3 diffuse = (light * color) * perpendicularity * attenuation;

