#version 330 core
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform vec3 objColor;
uniform vec3 lightPos = vec3(0.0, 400.0, 150.0);
uniform vec3 viewPos = vec3(0.0, 10.0, 70.0);

void main()
{
    vec3 lightColor = vec3(1.0, 0.95, 0.85);
    vec3 ambient = 0.3 * lightColor;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 result = (ambient + diffuse) * objColor;
    FragColor = vec4(result, 1.0);
}
