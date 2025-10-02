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
         if (colOut <= 0.6)
            col = vec4(1.0, 1.0, 1.0, 0.6);
         else 
            col = vec4(1.0, 1.0, 1.0, 0.1);
    } else {
        col = vec4(0.0);
    }
        gl_FragColor = col;
}
