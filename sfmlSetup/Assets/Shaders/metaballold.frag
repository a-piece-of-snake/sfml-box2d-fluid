#define MAX_PARTICLES 1000
uniform vec2 particle[MAX_PARTICLES];
uniform vec2 iResolution;  
uniform float iTime;    
uniform int particleCount;

float smin(float a, float b, float k) {
  float h = clamp(0.5 + 0.5*(a-b)/k, 0.0, 1.0);
  return (mix(a, b, h) - k*h*(1.0-h));
}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    fragCoord.y = iResolution.y - fragCoord.y; 

    vec2 p = (fragCoord - iResolution.xy*.5) / iResolution.y;
    
    float time = iTime / 6.0;
    
    float colOut = 1.0;

    for (int i = 0; i < particleCount; i++) {
        float dist = distance(p, particle[i]);
        colOut = smin(colOut, dist, 0.005);
    }
    
    vec3 col = vec3(colOut);
    
    if (colOut <= 0.00875) {
        
        if (colOut <= 0.0075) {
            col = vec3(0.44, 0.1, 0.36);
        } else {
            col = vec3(0.93, 0.5, 0.8);
        }
        gl_FragColor = vec4(col, 1.0);
        
    } else {
        float c = (1.0-colOut) * 0.9;
        col = vec3(c * 0.5, c / 2.0, c * 0.8);
        gl_FragColor = vec4(col, 0.0);
    }

}