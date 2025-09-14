#version 330

uniform sampler2D screenTexture;
uniform int blurRadius;
uniform vec2 iResolution;

void main()
{
    vec2 texelSize = 1.0 / iResolution;

    vec4 result = vec4(0.0);
    float count = 0.0;

    vec2 uv = gl_FragCoord.xy / iResolution.xy;

    for (int x = -blurRadius; x <= blurRadius; ++x) {
        for (int y = -blurRadius; y <= blurRadius; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            result += texture(screenTexture, uv + offset);
            count += 1.0;
        }
    }
    gl_FragColor = result / count;
}