struct Matrices {
			projection: mat4x4f,
    	view: mat4x4f,
    	model: mat4x4f,
};
struct Uniforms {
    matrices: Matrices,
    color: vec4f,
		cameraPosition: vec3f,
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
	@location(2) uv: vec2f,
	@location(3) viewDirection: vec3f,
};
@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	let worldPosition = uniforms.matrices.model * vec4f(in.position, 1.0);
	out.position = uniforms.matrices.projection * uniforms.matrices.view * worldPosition;	
	out.normal = (uniforms.matrices.model * vec4f(in.normal, 0.0)).xyz;
	out.color = in.color;
	out.uv = in.uv;
	out.viewDirection = uniforms.cameraPosition - worldPosition.xyz;
	return out;
}
struct LightingUniforms {
    directions: array<vec4f, 2>,
    colors: array<vec4f, 2>,
		hardness: f32,
		diffusivity: f32,
		specularity: f32,
}
@group(0) @binding(1) var colorbaseTexture: texture_2d<f32>;
@group(0) @binding(2) var textureSampler: sampler;
@group(0) @binding(3) var<uniform> lightning: LightingUniforms;
@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
  let colorbase =  textureSample(colorbaseTexture, textureSampler, in.uv).rgb;
	let normal = normalize(in.normal);
	let viewDirection = normalize(in.viewDirection);
	var color = vec3f(0.0);
  	for (var i: i32 = 0; 2 > i; i++) {
			let direction = normalize(lightning.directions[i].xyz);
			let diffuse = max(0.0, dot(direction, normal)) * lightning.colors[i].rgb;
			let reflection = reflect(-direction, normal);
			let specular = pow(max(0.0, dot(reflection, viewDirection)), lightning.hardness);
			color += colorbase * lightning.diffusivity * diffuse + lightning.specularity * specular;
  	}
	let colorCorrected = pow(color, vec3f(2.2));
	return vec4f(colorCorrected, uniforms.color.a);
}
