#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColor;
layout (location = 3) out vec4 gSpecular;
layout (location = 4) out vec4 gEmissive;
layout (location = 5) out vec3 gDiffuseEnv;
layout (location = 6) out vec3 gSpecularEnv;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse;
uniform sampler2D normals;
uniform sampler2D ambient;
uniform sampler2D roughness;
uniform sampler2D heightmap;
uniform samplerCube cubemap;
uniform vec3 camPos;
uniform vec3 eye;
uniform mat4 view;

// http://www.thetenthplanet.de/archives/1180
mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv ) {
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
    // construct a scale-invariant frame
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord ) {
    // assume N, the interpolated vertex normal and
    // V, the view vector (vertex to eye)
    vec3 map = texture( normals, texcoord ).xyz;
    map = map * 255./127. - 128./127.;
    mat3 TBN = cotangent_frame( N, -V, texcoord );
    return normalize( TBN * map );
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float height =  texture(heightmap, texCoords).r;
    vec2 p = viewDir.xy / viewDir.z * (height * 5.0);
    return texCoords - p;
}

void main() {
    vec2 texCoords = FragPos.xz/4.0;
    texCoords = ParallaxMapping(texCoords, eye);
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = perturb_normal( normalize(Normal), camPos - FragPos, texCoords );
    // and the diffuse per-fragment color
    gColor = texture(texture_diffuse, texCoords);
    gSpecular = vec4(
        /*occlusion:  */ 0.0,
        /*roughness:  */ texture(roughness, texCoords).a,
        /*metallic:   */ 0.0,
        /*ambientocc: */ texture(ambient, texCoords).a
    );
    gEmissive = vec4(0.0);
    gDiffuseEnv = vec3(1.0);
    gSpecularEnv = vec3(1.0);
}
