#version 330 core

in vec3 Normal;
in vec2 TexCoord;
in vec3 Position;
in vec4 PosLightSpace;

out vec4 FragColor;

uniform vec3 color;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    return shadow;
}

void main() {
    vec3 lightColor = vec3(1.0);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - Position);
    vec3 viewDir = normalize(viewPos - Position);

    // hemisphere ambient: top-down gradient (sky vs ground)
    float hemi = norm.y * 0.5 + 0.5;
    vec3 skyAmbient = vec3(0.25, 0.35, 0.55);
    vec3 groundAmbient = vec3(0.15, 0.12, 0.08);
    vec3 ambient = mix(groundAmbient, skyAmbient, hemi) * lightColor;

    // diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor * 0.5;

    // rim light: edge glow gives 3D sphere feel even in shadow
    float rim = 1.0 - max(dot(norm, viewDir), 0.0);
    rim = pow(rim, 3.0) * 0.35;
    vec3 rimColor = vec3(0.6, 0.7, 0.9);

    // shadow
    float shadow = ShadowCalculation(PosLightSpace);
    float shadowFactor = 1.0 - shadow;

    // compose: ambient always visible, shadow dims diffuse+spec, rim always visible
    vec3 result = ambient + shadowFactor * (diffuse + specular) + rim * rimColor * color;
    result *= color;

    FragColor = vec4(result, 1.0);
}
