#version 330 core

in vec2 v_local_pos;
in vec2 v_rect_size;
in vec4 v_color;
in vec4 v_radii;
in float v_outline;
in vec2 v_world_pos;

uniform vec4 u_clip_rect;
uniform float u_clip_radius;
uniform bool u_clip_enabled;

out vec4 frag_color;

float rounded_box_sdf(vec2 p, vec2 b, vec4 r) {
    float rx = (p.x > 0.0) ? ((p.y > 0.0) ? r.z : r.y) : ((p.y > 0.0) ? r.w : r.x);
    rx = min(rx, min(b.x, b.y));
    vec2 q = abs(p) - b + rx;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - rx;
}

void main() {
    vec2 half_size = v_rect_size * 0.5;
    vec2 p = v_local_pos - half_size;
    float d = rounded_box_sdf(p, half_size - 0.5, v_radii);

    float alpha;
    if (v_outline > 0.0) {
        float aa = 1.0;
        float inner = d + v_outline;
        alpha = smoothstep(aa, -aa, d) * smoothstep(-aa, aa, inner);
    } else {
        alpha = 1.0 - smoothstep(-0.5, 0.5, d);
    }

    if (u_clip_enabled) {
        vec2 clip_half = u_clip_rect.zw * 0.5;
        vec2 clip_center = u_clip_rect.xy + clip_half;
        vec2 clip_p = v_world_pos - clip_center;
        float clip_d = rounded_box_sdf(clip_p, clip_half - 0.5,
            vec4(u_clip_radius, u_clip_radius, u_clip_radius, u_clip_radius));
        float clip_alpha = 1.0 - smoothstep(-0.5, 0.5, clip_d);
        alpha *= clip_alpha;
    }

    alpha *= v_color.a;
    frag_color = vec4(v_color.rgb * alpha, alpha);
}
