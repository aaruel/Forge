#version 410

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out Data {
    vec3 Position;
    vec3 Normal;
    vec2 TexCoord;
} data;

void main() {
////// Init special matrices
    mat4 ModelViewMatrix = inverse(model) * model;
    mat3 NormalMatrix = inverse( transpose ( mat3(ModelViewMatrix) ) );
////////////////////////////

    data.Normal = normalize( NormalMatrix * VertexNormal );
    data.Position = vec3( ModelViewMatrix * vec4( VertexPosition, 1 ) );
    data.TexCoord = VertexTex;
    
    gl_Position = projection * view * model * vec4( VertexPosition, 1 );
}
