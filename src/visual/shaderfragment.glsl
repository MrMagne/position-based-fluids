#version 120

uniform mat4 mv_matrix;
uniform sampler2D texture;

varying vec3 frag_position;
varying vec3 frag_normal;
varying vec2 frag_texcoord;

const vec3 light_direction = vec3(1.0,1.0,1.0);
const vec4 light_diffuse = vec4(0.8, 0.8, 0.8, 0.0);
const vec4 light_ambient = vec4(0.2, 0.2, 0.2, 1.0);
const vec4 light_specular = vec4(1.0, 1.0, 1.0, 1.0);

const float frag_shininess = 1.0;
const vec4 frag_specular = vec4(0.5, 0.5, 0.5, 1.0);

const vec4 light_intensity = vec4(0.8, 0.8, 0.8, 0.8);

void main()
{
    vec3 normCamSpace = normalize(frag_normal);
    float cosAngIncidence = dot(normCamSpace, light_direction);
    cosAngIncidence = clamp(cosAngIncidence, 0.0, 1.0);

    vec4 frag_diffuse = texture2D(texture, frag_texcoord);
    gl_FragColor = light_intensity * frag_diffuse * cosAngIncidence;

    // vec3 mv_light_direction = (mv_matrix * vec4(light_direction, 0.0)).xyz;
    // vec3 normal = normalize(frag_normal);
    // vec3 eye = normalize(frag_position);
    // vec3 reflection = reflect(mv_light_direction, normal);

    // vec4 frag_diffuse = texture2D(texture, frag_texcoord);
    // vec4 diffuse_factor = max(-dot(normal, mv_light_direction), 0.0) * light_diffuse;
    // vec4 ambient_diffuse_factor = diffuse_factor + light_ambient;
    // vec4 specular_factor = max(pow(-dot(reflection, eye), frag_shininess), 0.0) * light_specular;

    // gl_FragColor = specular_factor * frag_specular
    //     + ambient_diffuse_factor * frag_diffuse;
}
