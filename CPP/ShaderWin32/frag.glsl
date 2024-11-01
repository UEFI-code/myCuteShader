#version 120
uniform float time;
void main() {
    float red = abs(sin(time));
    float green = abs(cos(time));
    gl_FragColor = vec4(red, green, 0.0, 1.0);
}