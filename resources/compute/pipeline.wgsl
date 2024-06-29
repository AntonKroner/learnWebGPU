@group(0) @binding(0) var<storage, read> inputBuffer: array<f32, 64>;
@group(0) @binding(1) var<storage, read_write> outputBuffer: array<f32, 64>;
@compute @workgroup_size(32)
fn main() {
    // Compute stuff
}
