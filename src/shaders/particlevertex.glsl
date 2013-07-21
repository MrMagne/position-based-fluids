#version 120

uniform mat4 cameraToClipMatrix;
uniform mat4 worldToCameraMatrix;
uniform mat4 modelToWorldMatrix;

attribute vec4 position;

varying float frag_velocity;

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
    frag_velocity = position.w;
    
    mat4 center = translate(-0.3, -0.3-0.02, -0.3);

    vec4 tmpEye = vec4(position.xyz, 1.0);
    vec4 eye_position = worldToCameraMatrix * center * tmpEye;
    gl_Position = cameraToClipMatrix * eye_position;

    float a = 1.0;
    float b = 0.0;
    float c = 10.0;
    float d = length(position.xyz);
    float size = 10.0;
    float derived_size = size * sqrt(1.0/(a + b * d + c * d * d));

    gl_PointSize = 2.0 / pow(gl_Position.w,2.0);
}

