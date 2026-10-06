/**
 * CUDA performance teaching examples; not a standalone application.
 * Not compiled, executed or benchmarked. See the companion study guide.
 *
 * Before using these examples, reason through empty/null buffers, dimensions
 * not divisible by a block size, byte-count overflow and insufficient memory.
 * Validate positive dimensions, finite c/s, allocation capacity and RGB format.
 * Check every CUDA result, including stream creation and synchronisation.
 * References: NVIDIA CUDA Best Practices and API synchronisation behaviour.
 */
#include <cuda_runtime.h>
#include <stddef.h>

// Implemented in Task1_CUDA_Kernel_Examples.cu; fixed three-channel RGB.
__global__ void rotate_rgb(const unsigned char*, unsigned char*,
                          int, int, float, float);

// Compare launch shapes while keeping 256 threads (eight warps) per block.
// x is the fastest-changing thread dimension. A 32-wide block maps a warp
// to one output row. The winner depends on row alignment, source-read locality,
// registers and occupancy. No universal speed-up is implied.
dim3 rotation_grid(int width, int height, dim3 block) {
    // Preconditions: positive dimensions and valid, nonzero block dimensions.
    // This form avoids overflowing width + block.x - 1 in signed arithmetic.
    return dim3(1u + (static_cast<unsigned>(width) - 1u) / block.x,
                1u + (static_cast<unsigned>(height) - 1u) / block.y);
}

// Enqueue one image's ordered H2D -> kernel -> D2H work in a supplied stream.
// Preconditions: pinned host buffers (e.g. cudaMallocHost), valid separate
// device buffers, checked bytes == width * height * 3, and a live stream.
// Callers must keep all buffers alive and synchronise before consuming output.
// On failure, previously enqueued work may still be pending: the caller must
// wait/clean up safely. A returned success means submission, not completion.
cudaError_t enqueue_rotation(const unsigned char* h_input,
                             unsigned char* h_output,
                             unsigned char* d_input,
                             unsigned char* d_output,
                             size_t bytes, int width, int height,
                             float c, float s, cudaStream_t stream) {
    const dim3 block(32, 8);  // Compare against dim3(16, 16) when profiling.
    const dim3 grid = rotation_grid(width, height, block);
    cudaError_t status = cudaMemcpyAsync(d_input, h_input, bytes,
                                        cudaMemcpyHostToDevice, stream);
    if (status != cudaSuccess) return status;
    rotate_rgb<<<grid, block, 0, stream>>>(d_input, d_output,
                                         width, height, c, s);
    status = cudaGetLastError();
    if (status != cudaSuccess) return status;
    return cudaMemcpyAsync(h_output, d_output, bytes,
                           cudaMemcpyDeviceToHost, stream);
}

/*
For independent images, use separate non-default streams and separate pinned
and device buffers. Overlap is possible only with supporting hardware and
available resources; submission alone does not guarantee it. Synchronise each
stream before reading/reusing/freeing its output. More buffers cost memory.

For a fair comparison, measure the same sampling rule and canvas on CPU/GPU.
CUDA events around the kernel measure device elapsed time (after completion).
A host timer around setup, copies, launch and final completion measures total
latency. Exclude file I/O from both paths or include it in both.

Total pixel work is O(width * height). A fixed-size batch of k images uses
O(k * width * height) buffer space. Overlap changes scheduling, not total work.
*/
