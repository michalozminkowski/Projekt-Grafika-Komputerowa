#version 330 core
layout(location = 0) in vec3 vertexPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
    // Depth bias: pull the line slightly towards the camera to prevent clipping into the mountain
    gl_Position.z -= 0.005 * gl_Position.w;
}
