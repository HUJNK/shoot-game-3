#version 330 core

in vec3 Normal;
in vec2 TexCoord;
in vec3 Position;
in vec4 PosLightSpace;

out vec4 FragColor;

uniform vec3 color;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 movingLightPos;
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
    vec3 ambient = mix(groundAmbient, skyAmbient, hemi) * lightColor * 1.6;

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

    // moving light: warm point light always visible (no shadow), illuminates shadow areas
    vec3 movingLightColor = vec3(1.0, 0.7, 0.4);
    vec3 movingLightDir = normalize(movingLightPos - Position);
    float movingDist = length(movingLightPos - Position);
    float movingAtten = 1.0 / (1.0 + 0.01 * movingDist + 0.0005 * movingDist * movingDist);
    float movingDiff = max(dot(norm, movingLightDir), 0.0);
    vec3 movingHalf = normalize(movingLightDir + viewDir);
    float movingSpec = pow(max(dot(norm, movingHalf), 0.0), 32.0);
    vec3 movingLight = movingAtten * movingLightColor * (movingDiff * 0.7 + movingSpec * 0.3);
    result += movingLight * color;

    // === Star point lights (subtle) ===
    vec3 star1 = vec3(-40.0, 50.0, -50.0);
    vec3 star2 = vec3( 40.0, 50.0, -50.0);
    vec3 starColor = vec3(1.0, 0.95, 0.7);
    float sd1 = length(star1 - Position);
    float sd2 = length(star2 - Position);
    result += starColor * color * 0.20 / (1.0 + 0.004 * sd1 * sd1);
    result += starColor * color * 0.20 / (1.0 + 0.004 * sd2 * sd2);

	// === Environment Mapping: metallic reflection of sky ===
	vec3 reflectDir = reflect(-viewDir, norm);
	float reflY = reflectDir.y;
	// sky gradient sampled by reflection direction
	vec3 reflTop = vec3(0.15, 0.25, 0.55);
	vec3 reflHorizon = vec3(0.55, 0.72, 0.95);
	vec3 reflGround = vec3(0.3, 0.22, 0.15);
	float reflT = smoothstep(-0.2, 0.5, reflY);
	vec3 reflSky = mix(reflHorizon, reflTop, reflT);
	reflSky = mix(reflGround, reflSky, smoothstep(-0.1, 0.1, reflY));
	// sun highlight in reflection
	float reflSun = pow(max(dot(reflectDir, normalize(vec3(0.3, 0.6, 0.5))), 0.0), 32.0);
	reflSky += vec3(1.0, 0.9, 0.5) * reflSun * 0.6;
	// metallic Fresnel: edge reflection stronger than center
	float metallic = 0.15 + rim * 0.35;  // rim is already computed
	result = mix(result, reflSky * color, metallic);

    result *= color;

    FragColor = vec4(result, 1.0);
}
