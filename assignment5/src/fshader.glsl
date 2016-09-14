#version 150

in  vec4 color;
out vec4 fColor;
uniform int id_mode;
uniform vec4 objectColor;

void main()
{
    fColor = color;
    if (id_mode == 1) {
        fColor = objectColor;
    }
}
