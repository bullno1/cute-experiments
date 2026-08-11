layout (location = 0) in vec2 v_uv;
layout (location = 0) out vec4 result;

layout (set = 2, binding = 0) uniform sampler2D u_image;

void main() {
    vec4 color = texture(u_image, v_uv);
	if (color.a < 0.5) discard;
	result = color;
}
