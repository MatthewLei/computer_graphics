#version 150

in  vec4 color;
out vec4 fColor;
uniform int choice;
uniform vec4 objectColor;

void main()
{
    fColor = color;
    if (choice == 1) {
        fColor = objectColor;
    }
}
