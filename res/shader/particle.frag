#version 330 core
in vec4 fragColor;
out vec4 FragColor;

void main()
{
    vec2 coord = gl_PointCoord - vec2(0.5);

    // Blade shape: long thin diamond like a windmill fan
    // Elongated along X, narrow along Y
    float bladeX = coord.x / 0.48;
    float bladeY = coord.y / 0.12;
    float bladeDist = bladeX * bladeX + bladeY * bladeY;
    if (bladeDist > 1.0)
        discard;

    // Also draw a small round core at center
    float coreDist = length(coord) / 0.08;
    float coreShape = 1.0 - smoothstep(0.0, 1.0, coreDist);

    // Blade body glow
    float body = 1.0 - smoothstep(0.6, 1.0, bladeDist);
    // Outer glow on blade
    float glow = exp(-bladeDist * 2.0) * 0.6;

    float alpha = fragColor.a * (body * 0.8 + coreShape * 1.5 + glow * 2.5);
    vec3 color = fragColor.rgb * (2.5 + coreShape * 4.0 + glow * 2.0);

    FragColor = vec4(color, alpha);
}
