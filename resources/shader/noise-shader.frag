#version 330 core

uniform sampler1D noiseTex; 
uniform float stepDist;
uniform float variance;

in vec2 TexCoord;
out vec4 FragColor;

void main() {
    float sampLoc = TexCoord.x;
    if (TexCoord.y > 0.9) {
        float v = floor(TexCoord.x / stepDist);
        sampLoc = v * stepDist;
    } else if (TexCoord.y > 0.6 && mod(TexCoord.x, stepDist) < 0.005f) {
        FragColor = vec4(0, 1, 0, 1);
        return;
    }
    float val = texture(noiseTex, sampLoc).r;
    val = (val - 0.5f) * (variance * 100.f) + 0.5f;

    vec4 lowCol = vec4(97, 20, 191, 255) / 255.f;
    vec4 highCol = vec4(244, 225, 75, 255) / 255.f;
    FragColor = mix(lowCol, highCol, val);
    //FragColor = vec4(val, 0, 1.f - val, 1.f);
    //FragColor = vec4(val * 10.f, val * 10.f, val * 10.f, 1.f);
}