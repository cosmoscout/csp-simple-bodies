#version 330

uniform vec3 uRadii;
uniform mat4 uMatModelView;
uniform mat4 uMatProjection;

// inputs
layout(location = 0) in vec2 iGridPos;

// outputs
out vec2 vTexCoords;
out vec3 vPosition;
out vec3 vCenter;
out vec2 vLonLat;

const float PI = 3.141592654;

void main() {
    vTexCoords = vec2(iGridPos.x, 1 - iGridPos.y);
    vLonLat.x = iGridPos.x * 2.0 * PI;
    vLonLat.y = (iGridPos.y - 0.5) * PI;
    vPosition = uRadii * vec3(
        -sin(vLonLat.x) * cos(vLonLat.y),
        -cos(vLonLat.y + PI * 0.5),
        -cos(vLonLat.x) * cos(vLonLat.y)
    );
    vPosition   = (uMatModelView * vec4(vPosition, 1.0)).xyz;
    vCenter     = (uMatModelView * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    gl_Position =  uMatProjection * vec4(vPosition, 1);
}