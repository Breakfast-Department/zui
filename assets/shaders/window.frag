#version 330 core

in vec2 v_local_pos;
in vec2 v_rect_size;
in vec2 v_world_pos;

uniform vec4 u_bg_color;
uniform vec4 u_border_color;
uniform float u_border_width;
uniform float u_radius_tl;
uniform float u_radius_tr;
uniform float u_radius_br;
uniform float u_radius_bl;
uniform float u_titlebar_height;
uniform vec4 u_titlebar_color;
uniform float u_curve_intensity;
uniform bool u_active;

out vec4 frag_color;

float sdf_corner_tl(vec2 p, float r) {
    vec2 q = p - vec2(r, r);
    if (q.x < 0.0 && q.y < 0.0) {
        return length(q) - r;
    }
    return min(max(-p.x, -p.y), 0.0);
}

float sdf_corner_tr(vec2 p, vec2 size, float r, float curve) {
    vec2 corner = vec2(size.x - r, r);
    vec2 q = p - corner;

    if (q.x > 0.0 && q.y < 0.0) {
        float wave = sin((p.y / r) * 1.5) * curve * 0.15;
        q.x += wave;
        return length(q) - r;
    }
    return min(max(p.x - size.x, -p.y), 0.0);
}

float sdf_corner_br(vec2 p, vec2 size, float r, float curve) {
    vec2 corner = vec2(size.x - r, size.y - r);
    vec2 q = p - corner;

    if (q.x > 0.0 && q.y > 0.0) {
        float wave = sin((p.y / size.y) * 3.14159) * curve * 0.1;
        q.x += wave;
        return length(q) - r;
    }
    return min(max(p.x - size.x, p.y - size.y), 0.0);
}

float sdf_corner_bl(vec2 p, vec2 size, float r) {
    vec2 corner = vec2(r, size.y - r);
    vec2 q = p - corner;

    if (q.x < 0.0 && q.y > 0.0) {
        return length(q) - r;
    }
    return min(max(-p.x, p.y - size.y), 0.0);
}

float window_sdf(vec2 p, vec2 size, float rtl, float rtr, float rbr, float rbl, float curve) {
    rtl = min(rtl, min(size.x * 0.5, size.y * 0.5));
    rtr = min(rtr, min(size.x * 0.5, size.y * 0.5));
    rbr = min(rbr, min(size.x * 0.5, size.y * 0.5));
    rbl = min(rbl, min(size.x * 0.5, size.y * 0.5));

    float d_tl = sdf_corner_tl(p, rtl);
    float d_tr = sdf_corner_tr(p, size, rtr, curve);
    float d_br = sdf_corner_br(p, size, rbr, curve);
    float d_bl = sdf_corner_bl(p, size, rbl);

    float d_left = -p.x;
    float d_right = p.x - size.x;
    float d_top = -p.y;
    float d_bottom = p.y - size.y;

    bool in_tl = p.x < rtl && p.y < rtl;
    bool in_tr = p.x > size.x - rtr && p.y < rtr;
    bool in_br = p.x > size.x - rbr && p.y > size.y - rbr;
    bool in_bl = p.x < rbl && p.y > size.y - rbl;

    if (in_tl) return d_tl;
    if (in_tr) return d_tr;
    if (in_br) return d_br;
    if (in_bl) return d_bl;

    return max(max(d_left, d_right), max(d_top, d_bottom));
}

void main() {
    vec2 p = v_local_pos;
    vec2 size = v_rect_size;

    float curve = u_curve_intensity;
    float d = window_sdf(p, size, u_radius_tl, u_radius_tr, u_radius_br, u_radius_bl, curve);

    float aa = 1.0;
    float shape_alpha = 1.0 - smoothstep(-aa, aa, d);

    if (shape_alpha < 0.001) {
        discard;
    }

    bool in_titlebar = p.y < u_titlebar_height;

    vec4 base_color;
    if (in_titlebar) {
        base_color = u_titlebar_color;
    } else {
        base_color = u_bg_color;
    }

    if (!u_active) {
        base_color.rgb *= 0.85;
    }

    vec4 final_color = base_color;

    if (u_border_width > 0.0) {
        float inner_d = d + u_border_width;
        float border_blend = smoothstep(-aa, aa, inner_d);
        final_color = mix(u_border_color, base_color, border_blend);
    }

    float final_alpha = final_color.a * shape_alpha;
    frag_color = vec4(final_color.rgb * final_alpha, final_alpha);
}
