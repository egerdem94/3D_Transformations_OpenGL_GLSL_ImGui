#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main() {
    // Colors for Gooch Shading
    vec3 coolColor = vec3(0.0, 0.0, 0.55);
    vec3 warmColor = vec3(0.3, 0.3, 0.0);
    float diffuseWarm = 0.6;
    float diffuseCool = 0.6;
    
    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Gooch Shading
    vec3 kcool = min(coolColor + diffuseCool * diffuse, 1.0);
    vec3 kwarm = min(warmColor + diffuseWarm * diffuse, 1.0);
    vec3 result = mix(kcool, kwarm, (dot(norm, lightDir) + 1.0) / 2.0);

    FragColor = vec4(result, 1.0);
}
