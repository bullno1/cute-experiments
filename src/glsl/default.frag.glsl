#include "smooth_uv.shd"

layout (location = 0) in vec2 v_uv;
layout (location = 1) flat in vec4 v_uv_rect;
layout (location = 0) out vec4 result;

layout (set = 2, binding = 0) uniform sampler2D u_image;

void main() {
	result = texture(u_image, smooth_uv(v_uv, v_uv_rect, textureSize(u_image, 0)));
}
