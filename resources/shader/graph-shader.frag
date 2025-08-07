#version 330 core

uniform sampler1D powerTex; 

in vec2 TexCoord;
out vec4 FragColor;

void main() {
    float sampLoc = TexCoord.x;
    float val = texture(powerTex, sampLoc).r;
    float lineWidth = 0.01;

    if (TexCoord.y < val + lineWidth && TexCoord.y > val - lineWidth) {
        FragColor = vec4(0, 0, 0, 1);
        return;
    }

    vec4 lowCol = vec4(97, 20, 191, 255) / 255.f;
    vec4 highCol = vec4(244, 225, 75, 255) / 255.f;
    FragColor = mix(lowCol, highCol, val);
}