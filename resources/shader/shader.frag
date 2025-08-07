#version 330 core

uniform bool hovered;
uniform bool rotating;
uniform bool isImage;
uniform sampler2D tex0; 

uniform bool enableSaturation;

in vec2 TexCoord;
out vec4 FragColor;

vec3 rgb2hsl( in vec3 c ){
  float h = 0.0;
    float s = 0.0;
    float l = 0.0;
    float r = c.r;
    float g = c.g;
    float b = c.b;
    float cMin = min( r, min( g, b ) );
    float cMax = max( r, max( g, b ) );

    l = ( cMax + cMin ) / 2.0;
    if ( cMax > cMin ) {
        float cDelta = cMax - cMin;
        
        //s = l < .05 ? cDelta / ( cMax + cMin ) : cDelta / ( 2.0 - ( cMax + cMin ) ); Original
        s = l < .0 ? cDelta / ( cMax + cMin ) : cDelta / ( 2.0 - ( cMax + cMin ) );
        
        if ( r == cMax ) {
            h = ( g - b ) / cDelta;
        } else if ( g == cMax ) {
            h = 2.0 + ( b - r ) / cDelta;
        } else {
            h = 4.0 + ( r - g ) / cDelta;
        }

        if ( h < 0.0) {
            h += 6.0;
        }
        h = h / 6.0;
    }
    return vec3( h, s, l );
}

vec3 hsl2rgb( in vec3 c )
{
    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );

    return c.z + c.y * (rgb-0.5)*(1.0-abs(2.0*c.z-1.0));
}



void main() {
    if (isImage) {
        FragColor = texture(tex0, TexCoord);

        if (enableSaturation) {
            vec3 hsl = rgb2hsl(FragColor.rgb);
            hsl.y = 1.f;
            FragColor = vec4(hsl2rgb(hsl) * FragColor.a, FragColor.a);

        }
        return;
    }

    if (hovered) {
        FragColor = vec4(0, 1, 0, 1);
    } else if (rotating) {
        FragColor = vec4(0, 0, 1, 1);
    } else {
        FragColor = vec4(0.2, 0.8, 0.34, 1);
    }
}