#version 330 core

uniform float val;

in vec2 TexCoord;
out vec4 FragColor;

void main() {
    vec2 st = 2.f * TexCoord - 1.f;
    float angle = atan(st.y, st.x) + radians(45);

    // Normalize angle to be between 0 and 2*PI
    if (angle < 0.0) {
       angle += 2.0 * 3.14159;
    }

    vec2 center = vec2(0.5, 0.5);
    float width = mix(0.05, 0.005, angle / radians(270));
    float circleRadius = 0.45f - width;
    float outerCircleRadius = 0.45f + width;
    float dist = length(TexCoord - center);
    
    float edgeWidth = 0.005; 
    float circle = smoothstep(circleRadius - edgeWidth, circleRadius + edgeWidth, dist);
    float outerCircle = 1 - smoothstep(outerCircleRadius - edgeWidth, outerCircleRadius + edgeWidth, dist);



    // Check if the point is within the three-quarter circle
    //float // + (step(angle, radians(315)) * -1 + 1))
    float is_in_circle = step(angle, radians(270));
    is_in_circle *= step(angle, radians(270.f * (1.f - val))) * -1 + 1;

    vec4 highCol = vec4(97, 20, 191, 255) / 255.f;
    vec4 lowCol = vec4(244, 225, 75, 255) / 255.f;
    FragColor = mix(lowCol, highCol, angle / radians(270));
    FragColor.a = circle * outerCircle * is_in_circle;
}