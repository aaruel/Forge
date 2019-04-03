#version 410 core
//out vec4 FragColor;
//in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)
//in float light;
//
//struct Material {
//
//};
//
//uniform Material material;
//
//void main() {
//    FragColor = vertexColor * light;
//}

in Data
{
    vec3 FrontColor;
    vec3 BackColor;
    vec2 TexCoord;
} data;


out vec4 FragColor;

uniform sampler2D Tex;

void main()
{
    if ( gl_FrontFacing )
        FragColor = vec4(data.FrontColor, 1);
    else
        FragColor = vec4(data.BackColor, 1);

//    FragColor *= texture(Tex, data.TexCoord);
}
