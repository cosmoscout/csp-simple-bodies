#version 440 compatibility

uniform sampler2D uSurfaceTexture;
uniform float uAmbientBrightness;
uniform float uFarClip;

// inputs
in vec2 vTexCoords;
in vec3 vSunDirection;
in vec3 vPosition;
in vec3 vCenter;
in vec2 vLonLat;

// outputs
layout(location = 0) out vec3 oColor;

void main() {
    vec3 normal = normalize(vPosition - vCenter);
    float light = max(dot(normal, normalize(vSunDirection)), 0.0);

    oColor = texture(uSurfaceTexture, vTexCoords).rgb;
    oColor = mix(oColor * uAmbientBrightness, oColor, light);

    gl_FragDepth = length(vPosition) / uFarClip;
}