#include "smooth_uv.shd"

layout (location = 0) in vec2 v_uv;
layout (location = 1) flat in vec4 v_uv_rect;
layout (location = 2) in float v_depth;
layout (location = 0) out vec4 result;

layout (set = 2, binding = 0) uniform sampler2D u_image;

void main() {
	vec4 color = texture(u_image, smooth_uv(v_uv, v_uv_rect, textureSize(u_image, 0)));
	if (color.a < 0.5) discard;

	float fog = 1.0 - exp(-v_depth * 0.065);
	vec3 out_color = mix(color.rgb, vec3(0.05, 0.06, 0.12), fog);
	result = vec4(out_color, color.a);
}
