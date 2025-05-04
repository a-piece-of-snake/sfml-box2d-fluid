uniform vec2 iResolution;  
uniform float iTime;    
uniform sampler2D tex;
uniform vec2 texSize;

float smin(float a, float b, float k) {
  float h = clamp(0.5 + 0.5*(a-b)/k, 0.0, 1.0);
  return mix(a, b, h) - k*h*(1.0-h);
}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    fragCoord.y = iResolution.y - fragCoord.y; 

    vec2 p = (fragCoord - iResolution.xy*.5) / iResolution.y;
    
    float time = iTime / 6.0;
    
    float colOut = 1.0;

    for (int y = 0; y < int(texSize.y); y++) {
        for (int x = 0; x < int(texSize.x); x++) {
            vec2 uv = (vec2(x, y) + 0.5) / texSize;
            vec4 color = texture(tex, uv);
            if (color == vec4(1.0, 1.0, 1.0, 1.0)) {

		        vec2 pos = uv * texSize;
                float dist = distance(p, pos);
                colOut = smin(colOut, dist, 0.1);

            }
        }
    }
    
    vec3 col = vec3(colOut);
    
    if (colOut <= 0.14) {
        
        if (colOut <= 0.09) {
            col = vec3(0.44, 0.1, 0.36);
        } else {
            col = vec3(0.93, 0.5, 0.8);
        }
        
    } else {
        float c = (1.0-colOut) * 0.9;
        col = vec3(c * 0.5, c / 2.0, c * 0.8);
    }

    gl_FragColor = vec4(col, 1.0);
}