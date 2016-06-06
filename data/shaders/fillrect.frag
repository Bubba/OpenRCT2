#version 330

uniform vec4i uClip;

uniform int   uFlags;
uniform vec4  uColour;

in vec2i fPosition;

layout (location = 0) out vec4 oColour;

void main()
{
    if (fPosition.x < uClip.x || fPosition.x > uClip.z ||
        fPosition.y < uClip.y || fPosition.y > uClip.w)
    {
        discard;
    }

    oColour = uColour;
}
