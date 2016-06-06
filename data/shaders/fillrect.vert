#version 330

uniform vec2i uScreenSize;

in vec2i vPosition;

out vec2i fPosition;

void main()
{
    fPosition = vPosition;

    // Transform screen coordinates to viewport
    vec2 pos = vPosition;
    pos.x = (pos.x * (2.0 / uScreenSize.x)) - 1.0;
    pos.y = (pos.y * (2.0 / uScreenSize.y)) - 1.0;
    pos.y *= -1;

    gl_Position = vec4(pos, 0, 1);
}
