#version 330 core
in vec3 texCoords;
out vec4 FragColor;

uniform vec3 sunDirection;
uniform float time;

void main()
{
    vec3 dir = normalize(texCoords);
    float height = dir.y;

    // sky gradient: deep blue at top → light blue at horizon → white haze
    vec3 topColor = vec3(0.15, 0.25, 0.55);
    vec3 horizonColor = vec3(0.55, 0.72, 0.95);
    vec3 groundColor = vec3(0.75, 0.82, 0.90);

    float t = smoothstep(-0.1, 0.3, height);
    vec3 sky = mix(horizonColor, topColor, t);

    // horizon glow
    float horizonGlow = exp(-abs(height) * 6.0);
    sky = mix(sky, vec3(0.9, 0.8, 0.5), horizonGlow * 0.4);

    // sun
    float sunAngle = dot(dir, normalize(sunDirection));
    float sunSize = 0.02;
    float sun = smoothstep(sunSize, sunSize * 0.3, sunAngle);
    vec3 sunColor = vec3(1.0, 0.95, 0.6);
    sky += sunColor * sun * 1.5;

    // sun glow
    float sunGlow = smoothstep(0.1, -0.1, sunAngle) * 0.4;
    sky += sunColor * sunGlow;

    // sun halo
    float halo = exp(-(1.0 - sunAngle) * 8.0) * 0.25;
    sky += vec3(1.0, 0.9, 0.6) * halo;

    // subtle clouds
    float cloud = 0.0;
    vec2 cloudCoord = dir.xz / (abs(dir.y) + 0.3) * 0.8;
    cloud += sin(cloudCoord.x * 3.5 + time * 0.02) * cos(cloudCoord.y * 2.8 + time * 0.015);
    cloud += sin(cloudCoord.x * 7.2 - time * 0.03) * cos(cloudCoord.y * 5.1 - time * 0.02);
    cloud = smoothstep(0.3, 0.9, abs(cloud)) * 0.5;
    cloud *= smoothstep(0.0, 0.4, height);
    sky = mix(sky, vec3(1.0), cloud * 0.25);

    FragColor = vec4(sky, 1.0);
}
