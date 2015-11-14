uniform vec2 lightPos;
uniform vec4 lightColor;
uniform float radius;
uniform vec2 viewport;
uniform float height;
void main()
{
    vec2 pos = vec2(gl_FragCoord.x, -gl_FragCoord.y);
    pos.y += height;
    float dist = distance(pos, lightPos - viewport);
    float attenuation = max(radius/(dist*30), 0.3);
    gl_FragColor = lightColor * attenuation * gl_Color;
}
