#version 450

layout(location =0) in vec3 Position;
layout(binding = 0) uniform UniformBufferObject {
    vec4 color;
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;
layout(location =0) out vec4 color;

void main() {
    gl_Position =ubo.proj * ubo.view * ubo.model * vec4(Position, 1.0);
    color = vec4(1.0,1.0,ubo.model[0].r,1.0);
}