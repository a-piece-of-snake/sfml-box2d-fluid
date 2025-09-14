#version 330

uniform vec2 iResolution;  
uniform float iTime;    
uniform sampler2D metaballT;

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;

    vec2 uv = fragCoord / iResolution;  

    vec4 col = texture(metaballT, uv);
    
    float colOut = dot(col.rgb, vec3(0.299, 0.587, 0.114));
    
    if (colOut >= 0.5) {
         col = vec4(0.0, 0.38, 0.83, 0.8);
         if (colOut <= 0.6)
            col = vec4(0.0, 0.42, 0.85, 0.9);
         else 
            col = vec4(0.0, 0.38, 0.73, 0.25);
    } else {
        col = vec4(0.0); 
    }
        gl_FragColor = col;
}
