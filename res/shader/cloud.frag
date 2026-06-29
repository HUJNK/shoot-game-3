#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D diffuse;
uniform vec3 objColor = vec3(1.0);

void main() {
    vec4 tex = texture(diffuse, TexCoord);
    FragColor = vec4(tex.rgb * objColor, tex.a);
}
