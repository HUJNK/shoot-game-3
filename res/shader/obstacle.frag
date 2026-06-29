#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
in vec4 PosLightSpace;

out vec4 FragColor;

uniform vec3 objColor;
uniform float objAlpha = 1.0;
uniform vec3 lightPos = vec3(0.0, 400.0, 150.0);
uniform vec3 viewPos = vec3(0.0, 10.0, 70.0);
uniform vec3 movingLightPos = vec3(0.0, 8.0, 25.0);
uniform sampler2D shadowMap;
uniform sampler2D obstacleTex;
uniform bool useTexture = false;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.008;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}

void main()
{
    vec3 lightColor = vec3(1.0, 0.95, 0.85);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // === Ambient (hemisphere-style for depth) ===
    float hemi = norm.y * 0.5 + 0.5;
    vec3 skyAmbient = vec3(0.35, 0.45, 0.6);
    vec3 groundAmbient = vec3(0.22, 0.18, 0.12);
    vec3 ambient = mix(groundAmbient, skyAmbient, hemi) * lightColor;

    // === Diffuse ===
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // === Specular (Blinn-Phong) ===
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 48.0);
    vec3 specular = spec * lightColor * 0.6;

    // === Rim light (Fresnel edge glow) ===
    float rim = 1.0 - max(dot(norm, viewDir), 0.0);
    rim = pow(rim, 3.5) * 0.3;
    vec3 rimColor = vec3(0.5, 0.6, 0.9);

    // === Shadow ===
    float shadow = ShadowCalculation(PosLightSpace);
    float shadowFactor = 1.0 - shadow;

    // === Compose static light ===
    vec3 result = ambient + shadowFactor * (diffuse + specular) + rim * rimColor * objColor;

    // === Moving warm point light (no shadow, illuminates dark areas) ===
    vec3 movingLightColor = vec3(1.0, 0.72, 0.45);
    vec3 movingLightDir = normalize(movingLightPos - FragPos);
    float movingDist = length(movingLightPos - FragPos);
    float movingAtten = 1.0 / (1.0 + 0.008 * movingDist + 0.0004 * movingDist * movingDist);
    float movingDiff = max(dot(norm, movingLightDir), 0.0);
    vec3 movingHalf = normalize(movingLightDir + viewDir);
    float movingSpec = pow(max(dot(norm, movingHalf), 0.0), 28.0);
    vec3 movingLight = movingAtten * movingLightColor * (movingDiff * 0.65 + movingSpec * 0.25);
    result += movingLight * objColor;

    // === Star point lights (subtle) ===
    vec3 star1 = vec3(-40.0, 50.0, -50.0);
    vec3 star2 = vec3( 40.0, 50.0, -50.0);
    vec3 starCol = vec3(1.0, 0.95, 0.7);
    float sd1 = length(star1 - FragPos);
    float sd2 = length(star2 - FragPos);
    result += starCol * objColor * 0.16 / (1.0 + 0.004 * sd1 * sd1);
    result += starCol * objColor * 0.16 / (1.0 + 0.004 * sd2 * sd2);

	// === Environment Mapping: metallic reflection of sky ===
	vec3 reflectDir = reflect(-viewDir, norm);
	float reflY = reflectDir.y;
	vec3 reflTop = vec3(0.15, 0.25, 0.55);
	vec3 reflHorizon = vec3(0.55, 0.72, 0.95);
	vec3 reflGround = vec3(0.3, 0.22, 0.15);
	float reflT = smoothstep(-0.2, 0.5, reflY);
	vec3 reflSky = mix(reflHorizon, reflTop, reflT);
	reflSky = mix(reflGround, reflSky, smoothstep(-0.1, 0.1, reflY));
	float reflSun = pow(max(dot(reflectDir, normalize(vec3(0.3, 0.6, 0.5))), 0.0), 32.0);
	reflSky += vec3(1.0, 0.9, 0.5) * reflSun * 0.6;
	float metallic = 0.10 + rim * 0.25;
	result = mix(result, reflSky * objColor, metallic);

    // === Texture: sample and mix with objColor tint ===
    vec3 texColor = texture(obstacleTex, TexCoord).rgb;
    vec3 brightTex = texColor * 1.8;  // compensate for dark bark texture
    vec3 tintedTex = brightTex * objColor;
    result *= mix(objColor, tintedTex, float(useTexture));

    FragColor = vec4(result, objAlpha);
}
