#version 330 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;

uniform vec2 u_resolution;
uniform vec4 u_rect;
uniform vec4 u_uv_rect;

out vec2 v_uv;
out vec2 v_world_pos;

void main() {
    vec2 pos = u_rect.xy + a_pos * u_rect.zw;
    vec2 ndc = (pos / u_resolution) * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);
    v_uv = u_uv_rect.xy + a_uv * u_uv_rect.zw;
    v_world_pos = pos;
}
