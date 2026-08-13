layout (location = 0) in vec2 v_uv;
layout (location = 1) flat in vec4 v_uv_rect;
layout (location = 0) out vec4 result;

layout (set = 2, binding = 0) uniform sampler2D u_image;

vec2 smooth_uv(vec2 local, vec4 rect, vec2 texture_size) {
	vec2 tile_texels = (rect.zw - rect.xy) * texture_size;

	vec2 pixel = local * tile_texels;
	vec2 seam  = floor(pixel + 0.5);
	pixel = seam + clamp((pixel - seam) / fwidth(pixel), -0.5, 0.5);

	vec2 uv = rect.xy + (pixel / tile_texels) * (rect.zw - rect.xy);
	vec2 half_texel = 0.5 / texture_size;
	return clamp(uv, rect.xy + half_texel, rect.zw - half_texel);
}

void main() {
	result = texture(u_image, smooth_uv(v_uv, v_uv_rect, textureSize(u_image, 0)));
}
