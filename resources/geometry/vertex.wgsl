@vertex
  fn vs_main(@location(0) position: vec2f) -> @builtin(position) vec4<f32> {
  return vec4f(position, 0.0, 1.0);
}
@fragment
fn fs_main() -> @location(0) vec4f {
  return vec4f(0.0, 0.4, 1.0, 1.0);
	};
