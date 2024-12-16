#version 330 core

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

#define NUM_LIGHTS 5
uniform PointLight lights[NUM_LIGHTS];
uniform vec3 viewPos; // Camera position

in vec3 FragPos;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture1;

void main()
{
    vec3 objectColor = texture(texture1, TexCoord).rgb;
    vec3 result = vec3(0.0);

    for (int i = 0; i < NUM_LIGHTS; ++i) {
        // Calculate light direction
        vec3 lightDir = normalize(lights[i].position - FragPos);

        // Diffuse lighting
        float diff = max(dot(normalize(FragPos), lightDir), 0.0);
        vec3 diffuse = lights[i].color * diff * lights[i].intensity;

        // Add contribution
        result += diffuse;
    }

    // Ambient lighting
    vec3 ambient = 0.1 * objectColor;

    // Combine lighting and object color
    vec3 finalColor = (ambient + result) * objectColor;
    FragColor = vec4(finalColor, 1.0);
}
