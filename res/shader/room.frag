#version 330 core

in vec3 Normal;
in vec2 TexCoord;
in vec3 Position;
in vec4 PosLightSpace;

out vec4 FragColor;

uniform sampler2D diffuse;     // upper wall texture (wall_1.jpg / wall_2.jpg)
uniform sampler2D shadowMap;
uniform sampler2D schoolTex;    // front wall (school.png)
uniform sampler2D floorTex;     // floor texture
uniform sampler2D sideWallTex;  // left/right walls
uniform sampler2D ceilingTex;    // ceiling base
uniform sampler2D signatureTex;  // signature on ceiling

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

    vec3 lightColor = vec3(1.0, 0.88, 0.72);
    vec3 ambient = 0.7 * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - Position);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseLight = diff * lightColor;

    vec3 viewDir = normalize(viewPos - Position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * lightColor * 0.4;

    // Floor: use floor texture
    float upness = dot(norm, vec3(0.0, 1.0, 0.0));
    float isFloor = smoothstep(0.7, 1.0, upness);
    vec2 floorUV = vec2(Position.x / 8.0 + 0.5, Position.z / 8.0 + 0.5);
    vec3 floorColor = texture(floorTex, floorUV).rgb;
    texColor = mix(texColor, floorColor, isFloor);

    // Ceiling: signature centered, rest uses base ceiling texture
    float isCeiling = 1.0 - smoothstep(-1.0, -0.7, upness);
    vec2 sigUV = vec2(Position.x / 40.0 + 0.5, 1.0 - (Position.z / 40.0 + 0.5));
    vec3 sigColor = texture(signatureTex, sigUV).rgb;
    vec2 ceilUV = vec2(Position.x / 8.0, Position.z / 8.0);
    vec3 ceilColor = texture(ceilingTex, ceilUV).rgb;
    // Show signature in center, base ceiling elsewhere
    float inSig = step(0.0,sigUV.x)*step(sigUV.x,1.0)*step(0.0,sigUV.y)*step(sigUV.y,1.0);
    vec3 finalCeil = mix(ceilColor, sigColor, inSig);
    texColor = mix(texColor, finalCeil, isCeiling);

    // === Side walls (normal +/-X): sidewall texture ===
    float isSideWall = smoothstep(0.85, 1.0, abs(norm.x));
    vec2 sideUV = vec2(Position.z / 8.0, Position.y / 8.0);
    vec3 sideColor = texture(sideWallTex, sideUV).rgb;
    texColor = mix(texColor, sideColor, isSideWall);

    // === Front wall: school.png full height ===
    float isFrontWall = smoothstep(0.85, 1.0, norm.z);
    float imgW = 60.0, imgH = 60.0;
    vec2 schoolUV = vec2(Position.x / imgW + 0.5, 1.0 - (Position.y / imgH + 0.4));
    vec3 schoolColor = texture(schoolTex, schoolUV).rgb;
    texColor = mix(texColor, schoolColor, isFrontWall);

    // === Star point lights (subtle warm glow) ===
    vec3 star1Pos = vec3(-40.0, 50.0, -50.0);
    vec3 star2Pos = vec3( 40.0, 50.0, -50.0);
    vec3 starColor = vec3(1.0, 0.95, 0.7);

    vec3 star1Dir = normalize(star1Pos - Position);
    float star1Dist = length(star1Pos - Position);
    float star1Atten = 1.0 / (1.0 + 0.004 * star1Dist + 0.0002 * star1Dist * star1Dist);
    float star1Diff = max(dot(norm, star1Dir), 0.0);
    vec3 star1Light = star1Atten * starColor * star1Diff * 0.7;

    vec3 star2Dir = normalize(star2Pos - Position);
    float star2Dist = length(star2Pos - Position);
    float star2Atten = 1.0 / (1.0 + 0.004 * star2Dist + 0.0002 * star2Dist * star2Dist);
    float star2Diff = max(dot(norm, star2Dir), 0.0);
    vec3 star2Light = star2Atten * starColor * star2Diff * 0.7;

    float shadow = ShadowCalculation(PosLightSpace);

    vec3 result = (ambient + (1.0 - shadow) * (diffuseLight + specular)) * texColor;
    result += (star1Light + star2Light) * texColor;

    FragColor = vec4(result, 1.0);
}
