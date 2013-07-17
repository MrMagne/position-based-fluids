#version 120

uniform mat4 p_matrix, mv_matrix;

attribute vec4 position;

varying float frag_velocity;
varying float frag_pointsize;

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
    vec4 eye_position = mv_matrix * center * tmpEye;
    gl_Position = p_matrix * eye_position;

    float a = 1.0;
    float b = 0.0;
    float c = 10.0;
    float d = length(position.xyz);
    float size = 10.0;
    float derived_size = size * sqrt(1.0/(a + b * d + c * d * d));

    frag_pointsize = 2.0 / pow(gl_Position.w,2.0);
    gl_PointSize = frag_pointsize;
}

