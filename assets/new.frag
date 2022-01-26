#version 460 core

// Interpolated values from the vertex shaders
in vec3 Normal;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform vec3 LightPosition_worldspace;

void main()
{
    vec3 lightPos = normalize(vec3(1.0, 1.0, 1.0));

    float diffuseFloat = max(dot(Normal, lightPos), 0.0);
    vec3 diffuseColor = diffuseFloat * vec3(1.0, 1.0, 1.0); // would be light color

	float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

    vec3 result = (ambient + diffuseColor) * vec3(1.0, 0.0, 0.0);
    color = vec4(result, 1.0);
}