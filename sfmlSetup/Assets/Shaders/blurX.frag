#version 330

uniform sampler2D screenTexture;
uniform vec2 iResolution; 
uniform float blurStrength; 

out vec4 FragColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec2 texelSize = 1.0 / iResolution; 

    float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

    vec3 finalColor = texture(screenTexture, uv).rgb * weights[0]; 

    for (int i = 1; i < 5; ++i)
    {
        finalColor += texture(screenTexture, uv + vec2(texelSize.x * float(i) * blurStrength, 0.0)).rgb * weights[i];
        finalColor += texture(screenTexture, uv - vec2(texelSize.x * float(i) * blurStrength, 0.0)).rgb * weights[i];
    }

    FragColor = vec4(finalColor, 1.0);
}