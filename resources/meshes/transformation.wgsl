struct Uniforms {
    color: vec4f,
    time: f32,
};
struct VertexInput {
	@location(0) position: vec3f,
	@location(1) normal: vec3f,
	@location(2) color: vec3f,
};
struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) color: vec3f,
	@location(1) normal: vec3f,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	let ratio = 640.0 / 480.0;
	var offset = vec2f(0.0);
	let S = transpose(mat4x4f(
		0.3,  0.0, 0.0, 0.0,
		0.0,  0.3, 0.0, 0.0,
		0.0,  0.0, 0.3, 0.0,
		0.0,  0.0, 0.0, 1.0,
	));
	let T = transpose(mat4x4f(
		1.0,  0.0, 0.0, 0.5,
		0.0,  1.0, 0.0, 0.0,
		0.0,  0.0, 1.0, 0.0,
		0.0,  0.0, 0.0, 1.0,
	));
	let angle1 = uniforms.time;
	let c1 = cos(angle1);
	let s1 = sin(angle1);
	let R1 = transpose(mat4x4f(
		 c1,  s1, 0.0, 0.0,
		-s1,  c1, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0,  0.0, 0.0, 1.0,
	));
	let angle2 = 3.0 * 3.14 / 4.0;
	let c2 = cos(angle2);
	let s2 = sin(angle2);
	let R2 = transpose(mat4x4f(
		1.0, 0.0, 0.0, 0.0,
		0.0,  c2,  s2, 0.0,
		0.0, -s2,  c2, 0.0,
		0.0,  0.0, 0.0, 1.0,
	));
	let homogeneous_position = vec4f(in.position, 1.0);
	let position = (R2 * R1 * T * S * homogeneous_position).xyz;
	out.position = vec4f(position.x, position.y * ratio, position.z * 0.5 + 0.5, 1.0);
	out.color = in.color;
	out.normal = in.normal;
	return out;
}
@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	let lightColor1 = vec3f(1.0, 0.9, 0.6);
  let lightColor2 = vec3f(0.6, 0.9, 1.0);
	let lightDirection1 = vec3f(0.5, -0.9, 0.1);
  let lightDirection2 = vec3f(0.2, 0.4, 0.3);
  let shading1 = max(0.0, dot(lightDirection1, in.normal));
  let shading2 = max(0.0, dot(lightDirection2, in.normal));
  let shading = shading1 * lightColor1 + shading2 * lightColor2;
  let color = in.color * shading;
	let corrected_color = pow(color, vec3f(2.2));
	return vec4f(corrected_color, 1.0);
}
