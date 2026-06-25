#version 330 core
in vec4 fragColor;
out vec4 FragColor;

void main()
{
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    if (dist > 0.5)
        discard;

    // Three-layer compositing for visible glow
    // Core: bright hot center
    float core = 1.0 - smoothstep(0.0, 0.13, dist);
    // Body: main colored region
    float body = 1.0 - smoothstep(0.06, 0.47, dist);
    // Glow: soft outer halo
    float glow = exp(-dist * 4.5) * 0.45;

    float alpha = fragColor.a * (body * 0.55 + core * 0.95 + glow);
    // Boost brightness at center for luminous look
    vec3 color = fragColor.rgb * (1.0 + core * 0.8 + glow * 0.3);

    FragColor = vec4(color, alpha);
}
