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
	@location(3) uv: vec2f,
};
struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) color: vec3f,
	@location(1) normal: vec3f,
	@location(2) uv: vec2f
};
@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	out.position = uniforms.matrices.projection * uniforms.matrices.view * uniforms.matrices.model * vec4f(in.position, 1.0);	
	out.color = in.color;
	out.normal = (uniforms.matrices.model * vec4f(in.normal, 0.0)).xyz;
	out.uv = in.uv;
	return out;
}
@group(0) @binding(1) var gradientTexture: texture_2d<f32>;
@group(0) @binding(2) var textureSampler: sampler;
@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
  let color =  textureSample(gradientTexture, textureSampler, in.uv).rgb;
	let corrected_color = pow(color, vec3f(2.2));
	return vec4f(corrected_color, 1.0);
}
