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

void main()
{
    vec4 eye_position = worldToCameraMatrix * modelToWorldMatrix * vec4(position.xyz, 1.0);

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

