#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;

uniform vec2 pos;
uniform vec2 size;
uniform mat4 model;

out vec2 TexCoord;

void main() {
    gl_Position = model * vec4(aPos, 0.0, 1.0);
    TexCoord = aTex;
}