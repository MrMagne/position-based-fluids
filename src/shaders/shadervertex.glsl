#version 120

uniform mat4 cameraToClipMatrix;
uniform mat4 worldToCameraMatrix;
uniform mat4 modelToWorldMatrix;

uniform sampler2D texture;

attribute vec4 position;
attribute vec4 normal;

varying vec3 frag_position;
varying vec3 frag_normal;
varying vec2 frag_texcoord;

mat4 translate(float x, float y, float z)
{
    return mat4(
               vec4(1.0, 0.0, 0.0, 0.0),
               vec4(0.0, 1.0, 0.0, 0.0),
               vec4(0.0, 0.0, 1.0, 0.0),
               vec4(x,   y,   z,   1.0)
           );
}

void main()
{
    mat4 center = translate(-0.3, -0.3-0.02, -0.3-0.02);

    vec4 tmpEye = vec4(position.xyz, 1.0);
    vec4 eye_position = worldToCameraMatrix * center * tmpEye;

    gl_Position = cameraToClipMatrix * eye_position;
    frag_position = eye_position.xyz;
    frag_normal = (worldToCameraMatrix * vec4(normal.xyz, 0.0)).xyz;

    // TODO: avoid conditional in shaders
    if(normal.y > 0.0) {
        frag_texcoord = (position.xz + vec2(0.0, -0.15)) * vec2(4.0);
    } else {
        frag_texcoord = (position.xy + vec2(0.5, -0.15)) * vec2(4.0);
    }
}

