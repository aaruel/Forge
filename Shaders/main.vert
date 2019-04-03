#version 410 core
//layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
//layout (location = 1) in vec3 aNorm;
//out vec4 vertexColor; // specify a color output to the fragment shader
//out float light;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
//
//void main()
//{
//    vec3 camPos = vec3(0.0, 0.0, 6.0);
//    vec3 fpos = vec3(model * vec4(aPos, 1.0));
//    vec3 norm = normalize(aNorm);
//    vec3 lightDir = normalize(camPos - fpos);
//    float ambient = 0.4;
//
//    light = max(dot(norm, lightDir), 0.0) + ambient;
//    gl_Position = projection * view * model * vec4(aPos, 1.0);
//    vertexColor = vec4(0.5, 0.5, 0.5, 1.0);
//}

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTex;

#define LIGHTCOUNT 1

out Data {
    vec3 FrontColor;
    vec3 BackColor;
    vec2 TexCoord;
} data;

struct LightInfo {
    vec3 Position;    //Light Position in eye-coords
    vec3 La;          //Ambient light intensity
    vec3 Ld;          //Diffuse light intensity
    vec3 Ls;          //Specular light intensity
};

struct MaterialInfo {
    vec3 Ka;          //Ambient reflectivity
    vec3 Kd;          //Diffuse reflectivity
    vec3 Ks;          //Specular reflectivity
    float Shininess;  //Specular shininess factor
};

LightInfo Light[LIGHTCOUNT];
uniform MaterialInfo Material;

mat4 ModelViewMatrix;
mat3 NormalMatrix;

void getEyeSpace( out vec3 norm, out vec3 position ) {
    norm = normalize( NormalMatrix * VertexNormal );
    position = vec3( ModelViewMatrix * vec4( VertexPosition, 1 ) );
}

vec3 light( int lightIndex, vec3 position, vec3 norm ) {
    vec3 s = normalize( vec3( Light[lightIndex].Position - position ) );
    vec3 v = normalize( -position.xyz );
    vec3 r = reflect( -s, norm );

    vec3 ambient = Light[lightIndex].La * Material.Ka;
    
    float sDotN = max( dot( s, norm ), 0.0 );
    vec3 diffuse = Light[lightIndex].Ld * Material.Kd * sDotN;
    
    vec3 spec = vec3( 0.0 );
    if ( sDotN > 0.0 )
        spec = Light[lightIndex].Ls * Material.Ks * pow( max( dot(r,v) , 0.0 ), Material.Shininess );

    return ambient + diffuse + spec;
}

void main() {
////// Init special matrices
    ModelViewMatrix = inverse(model) * model;
    NormalMatrix = inverse( transpose ( mat3(ModelViewMatrix) ) );
////////////////////////////

////// setup light
    Light[0].Position = vec3(view[3].xyz);
    Light[0].La = vec3(0.2);
    Light[0].Ld = vec3(0.8);
    Light[0].Ls = vec3(0.5);
//////////////////

    vec3 eyeNorm;
    vec3 eyePosition;
    getEyeSpace( eyeNorm, eyePosition );
    
    data.FrontColor = vec3(0);
    data.BackColor = vec3(0);
    
    for( int i=0; i<LIGHTCOUNT; ++i )
    {
        data.FrontColor += light( i, eyePosition, eyeNorm );
        data.BackColor  += light( i, eyePosition, -eyeNorm );
    }

    data.TexCoord = VertexTex;
    gl_Position = projection * view * model * vec4( VertexPosition, 1 );
}
