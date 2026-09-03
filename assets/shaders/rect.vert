#version 330 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_rect_pos;
layout(location = 2) in vec2 a_rect_size;
layout(location = 3) in vec4 a_color;
layout(location = 4) in vec4 a_radii;
layout(location = 5) in float a_outline;

uniform vec2 u_resolution;

out vec2 v_local_pos;
out vec2 v_rect_size;
out vec4 v_color;
out vec4 v_radii;
out float v_outline;
out vec2 v_world_pos;

void main() {
    vec2 pos = a_rect_pos + a_pos * a_rect_size;
    vec2 ndc = (pos / u_resolution) * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);
    v_local_pos = a_pos * a_rect_size;
    v_rect_size = a_rect_size;
    v_color = a_color;
    v_radii = a_radii;
    v_outline = a_outline;
    v_world_pos = pos;
}
