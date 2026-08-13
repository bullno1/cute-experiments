layout (location = 0) in vec3 in_pos;
layout (location = 2) in vec2 in_uv;

layout (location = 8)  in vec4 in_model0;          // model transform, affine row 0
layout (location = 9)  in vec4 in_model1;          // row 1
layout (location = 10) in vec4 in_model2;          // row 2
layout (location = 11) in vec4 in_uv_rect;         // cf_draw3d_push_texture's atlas sub-rect
layout (location = 15) in vec4 in_mesh_attributes; // cf_draw3d_push_mesh_attributes

layout (location = 0) out vec2 v_uv;
layout (location = 1) flat out vec4 v_uv_rect;

layout (set = 1, binding = 0) uniform uniform_block {
	mat4 u_view_projection;
};

void main() {
	vec4 p = vec4(in_pos, 1.0);
	vec3 world_pos = vec3(dot(in_model0, p), dot(in_model1, p), dot(in_model2, p));
	gl_Position = u_view_projection * vec4(world_pos, 1.0);

	v_uv = in_uv;
	v_uv_rect = in_uv_rect;
}
