#version 120

varying float frag_velocity;
varying float frag_pointsize;

const vec3 light_direction = vec3(1.0, 1.0, 1.0);
const vec4 light_intensity = vec4(1.0, 1.0, 1.0, 1.0);

// adapted from: http://www.arcsynthesis.org/gltut/Illumination/Tutorial%2013.html
void fakeSphere(out vec3 cameraNormal)
{
    vec2 mapping = gl_PointCoord*vec2(2.0) - vec2(1.0);
    float lensqr = dot(mapping, mapping);
    if(lensqr > 1.0) {
        discard;
    }

    cameraNormal = vec3(mapping, sqrt(1.0 - lensqr));
}

void main()
{
    vec3 cameraPos;
    vec3 cameraNormal;
    fakeSphere(cameraNormal);

    vec3 normCamSpace = normalize(cameraNormal);
    float cosAngIncidence = dot(normCamSpace, light_direction);
    cosAngIncidence = clamp(cosAngIncidence, 0.0, 1.0);

    vec4 frag_diffuse = vec4(frag_velocity, frag_velocity, 1.0, 1.0);
    gl_FragColor = light_intensity * frag_diffuse * cosAngIncidence;

    // gl_FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
}
