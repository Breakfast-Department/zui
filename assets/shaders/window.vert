#version 330 core

layout(location = 0) in vec2 a_pos;

uniform vec2 u_resolution;
uniform vec2 u_rect_pos;
uniform vec2 u_rect_size;

out vec2 v_local_pos;
out vec2 v_rect_size;
out vec2 v_world_pos;

void main() {
    vec2 pos = u_rect_pos + a_pos * u_rect_size;
    vec2 ndc = (pos / u_resolution) * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);
    v_local_pos = a_pos * u_rect_size;
    v_rect_size = u_rect_size;
    v_world_pos = pos;
}
