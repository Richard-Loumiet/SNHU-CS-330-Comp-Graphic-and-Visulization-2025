#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;

out vec3 vWorldPos;
out vec3 vWorldNormal;
out vec2 vUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 worldPos  = model * vec4(aPos, 1.0);
    vWorldPos      = worldPos.xyz;
    vWorldNormal   = mat3(transpose(inverse(model))) * aNormal;
    vUV            = aTex;
    gl_Position    = projection * view * worldPos;
}
