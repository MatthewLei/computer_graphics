#version 150

in vec4 vPosition;
in vec4 vNormal;

out vec4 N;
out vec4 L;
out vec4 E;

//uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 LightPosition;
//uniform float Shininess;

void main()
{
    gl_Position = Projection * ModelView * vPosition;
    N = ModelView * vNormal;
    L = LightPosition - vPosition;
    if (LightPosition.w == 0.0) {
        L = LightPosition;
    }
    E = ModelView * vPosition;

}
