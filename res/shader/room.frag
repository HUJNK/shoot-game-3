#version 330 core

in vec3 Normal;
in vec2 TexCoord;
in vec3 Position;
in vec4 PosLightSpace;

out vec4 FragColor;

uniform sampler2D diffuse;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

float ShadowCalculation(vec4 PosLightSpace) {
    vec3 projCoords = PosLightSpace.xyz / PosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    return shadow;
}

void main() {
    vec3 texColor = texture(diffuse, TexCoord).rgb;

    // warm beige light
    vec3 lightColor = vec3(1.0, 0.88, 0.72);

    // ambient
    vec3 ambient = 0.45 * lightColor;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - Position);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseLight = diff * lightColor;

    // specular
    vec3 viewDir = normalize(viewPos - Position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * lightColor * 0.4;

    // floor: warmer brown tone like wood/stone floor
    float upness = dot(norm, vec3(0.0, 1.0, 0.0));
    float isFloor = smoothstep(0.7, 1.0, upness);
    float isWall = 1.0 - smoothstep(0.1, 0.9, abs(upness));

    // floor gets warm brown tint, walls stay neutral beige
    texColor = mix(texColor, texColor * vec3(1.08, 1.03, 0.92), isFloor * 0.35);
    // walls: beige-brown like real painted walls
    texColor = mix(texColor, texColor * vec3(0.92, 0.87, 0.78), isWall * 0.4);

    // shadow
    float shadow = ShadowCalculation(PosLightSpace);

    vec3 result = (ambient + (1.0 - shadow) * (diffuseLight + specular)) * texColor;

    // subtle brightness
    result = result * 1.0;

    FragColor = vec4(result, 1.0);
}
