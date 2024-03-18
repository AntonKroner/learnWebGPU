struct Matrices {
			projection: mat4x4f,
    	view: mat4x4f,
    	model: mat4x4f,
};
struct Uniforms {
    matrices: Matrices,
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
	out.position = uniforms.matrices.projection * uniforms.matrices.view * uniforms.matrices.model * vec4f(in.position, 1.0);	
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
