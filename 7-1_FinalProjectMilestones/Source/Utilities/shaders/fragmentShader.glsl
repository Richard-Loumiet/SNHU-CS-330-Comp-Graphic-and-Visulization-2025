#version 460 core
out vec4 FragColor;

in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec2 vUV;

// ------------------------------
// CONFIG
// ------------------------------
#define MAX_LIGHTS 8

// ------------------------------
// UNIFORMS
// ------------------------------
struct Light {
    vec3  position;
    vec3  ambientColor;
    vec3  diffuseColor;
    vec3  specularColor;
    float focalStrength;      // shininess
    float specularIntensity;  // scales specular term
};

uniform int   lightCount;                 // <-- number of active lights [0..MAX_LIGHTS]
uniform Light lightSources[MAX_LIGHTS];   // <-- array of lights

uniform bool        bUseLighting;

uniform bool        bUseTexture;
uniform sampler2D   objectTexture;
uniform vec4        objectColor;          // RGBA
uniform vec3        viewPosition;         // camera position (world space)

// ------------------------------
// MAIN
// ------------------------------
void main()
{
    // Base color (with alpha)
    vec4 base = bUseTexture ? texture(objectTexture, vUV) : objectColor;

    if (!bUseLighting) {
        FragColor = base;
        return;
    }

    vec3 N = normalize(vWorldNormal);
    vec3 V = normalize(viewPosition - vWorldPos);

    vec3 ambientAccum  = vec3(0.0);
    vec3 diffuseAccum  = vec3(0.0);
    vec3 specularAccum = vec3(0.0);

    // Accumulate contribution from each active light
    for (int i = 0; i < lightCount && i < MAX_LIGHTS; ++i) {
        vec3 L = normalize(lightSources[i].position - vWorldPos);
        float NdotL = max(dot(N, L), 0.0);

        // Ambient + Diffuse
        ambientAccum += lightSources[i].ambientColor * base.rgb;
        diffuseAccum += lightSources[i].diffuseColor * base.rgb * NdotL;

        // Specular (Phong)
        vec3 R = reflect(-L, N);
        float specPow   = max(dot(R, V), 0.0);
        float shininess = max(lightSources[i].focalStrength, 1.0);
        specularAccum  += lightSources[i].specularColor
                        *  lightSources[i].specularIntensity
                        *  pow(specPow, shininess);
    }

    vec3 lighting = ambientAccum + diffuseAccum + specularAccum;

    FragColor = vec4(lighting, base.a);
}
