#version 330 core

in vec2 v_uv;
in vec2 v_world_pos;

uniform sampler2D u_texture;
uniform vec4 u_color;
uniform vec4 u_clip_rect;
uniform float u_clip_radius;
uniform bool u_clip_enabled;

out vec4 frag_color;

float rounded_box_sdf(vec2 p, vec2 b, float r) {
    r = min(r, min(b.x, b.y));
    vec2 q = abs(p) - b + r;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

void main() {
    float glyph_alpha = texture(u_texture, v_uv).a;
    float alpha = glyph_alpha * u_color.a;

    if (u_clip_enabled) {
        vec2 clip_half = u_clip_rect.zw * 0.5;
        vec2 clip_center = u_clip_rect.xy + clip_half;
        vec2 clip_p = v_world_pos - clip_center;
        float clip_d = rounded_box_sdf(clip_p, clip_half - 0.5, u_clip_radius);
        float clip_alpha = 1.0 - smoothstep(-0.5, 0.5, clip_d);
        alpha *= clip_alpha;
    }

    frag_color = vec4(u_color.rgb * alpha, alpha);
}
