#version 330

//in  vec4 color;
out vec4 fColor;
in vec4 manip_color;
uniform int id_mode;
uniform vec4 object_id_color;
uniform int num_shades;
uniform int toon_mode;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform vec4 LightPosition;
uniform float Shininess;

in vec4 N;
in vec4 L;
in vec4 E;

uniform float ConstantAttenuation;
uniform float LinearAttenuation;
uniform float QuadraticAttenuation;

uniform vec3 ConeDirection;
uniform float SpotCosCutoff;
uniform float SpotExponent;

const float OUTLINE = .03;

void main()
{
//    float lightDistance = length(L);
    
    vec4 NN = normalize(N);
    vec4 EE = normalize(-E);
    vec4 LL = normalize(L);
    
//    float attenuation = 1 / (ConstantAttenuation +
//                             (LinearAttenuation * lightDistance) +
//                             (QuadraticAttenuation * lightDistance * lightDistance));
//    float spotCos = dot(LL, -ConeDirection);
//    if (spotCos < SpotCosCutoff) {
//        attenuation = 0.0;
//    } else {
//        attn    *= pow(spotCos, SpotExponent);
//    }
    
    vec4 ambient, diffuse, specular;
    vec4 H = normalize(LL + EE);
    float Kd = max(dot(LL, NN), 0.0);
    
    if (toon_mode == 1) {
        //discretize the diffuse value
        float increment = 1.0 / num_shades;
        float color_r, color_g, color_b;
        color_r = 1.00f;
        color_g = 0.50f;
        color_b = 0.1f;
        float tmp = 0;
        for (int i = 1; i <= num_shades; i++) {
            if (Kd > tmp) {
                tmp += increment;
            } else {
                color_r = color_r * tmp;
                color_g = color_g * tmp;
                color_b = color_b * tmp;
                break;
            }
        }
        fColor = vec4(color_r, color_g, color_b, 1);
        
        //check for border
        if (dot(EE, NN) < OUTLINE) {
            fColor = vec4(0.0, 0.0, 0.0, 1.0);
        }
    } else {
        Kd = dot(LL, NN);
        float Ks = pow(max(dot(NN, H), 0.0), Shininess);
        ambient = AmbientProduct;
        diffuse = Kd * DiffuseProduct;
        if (dot(LL, NN) < 0.0) {
            specular = vec4(0.0, 0.0, 0.0, 1.0);
        } else {
            specular = Ks * SpecularProduct;
        }
        fColor = ambient + diffuse + specular;
    }
    
}
