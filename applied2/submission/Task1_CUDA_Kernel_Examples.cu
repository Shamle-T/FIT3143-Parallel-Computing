/**
 * Illustrative CUDA C++ kernels for FIT3143 Applied 2, Task 1.
 * Savin Vindiv De Alwis (35221631) - sdea0018@student.monash.edu
 * Shamle Thilaksiri (35512075) - wthi0003@student.monash.edu
 *
 * These are teaching examples, not a standalone or benchmarked application.
 * They have not been compiled or executed. A complete program must load the
 * image, allocate separate input/output device buffers, copy data, check CUDA
 * errors, and save the output. Do not infer a measured speed-up from this file.
 *
 * Assumptions: valid contiguous RGB uint8 buffers; positive width and height;
 * allocations large enough for width * height * 3 bytes; x increases rightward
 * and y increases downward. Grid dimensions cover the complete output image.
 * The brightness example assumes a value in [-255, 255]. Rotation assumes
 * finite sine/cosine values and dimensions representable by the chosen types.
 */

#include <cuda_runtime.h>
#include <math.h>
#include <stddef.h>

// One thread owns one RGB pixel. Addition uses an int to avoid uint8 wrap.
__global__ void brightness_rgb(const unsigned char* input,
                               unsigned char* output,
                               int width,
                               int height,
                               int brightness) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= width || y >= height) {
        return;
    }
    const size_t offset =
        (size_t(y) * size_t(width) + size_t(x)) * 3;
    for (int channel = 0; channel < 3; ++channel) {
        const int value = int(input[offset + channel]) + brightness;
        output[offset + channel] =
            value < 0 ? 0 : (value > 255 ? 255 : value);
    }
}

// Positive theta is visually counterclockwise in y-down image coordinates.
// The host supplies c = cos(theta), s = sin(theta), with theta in radians.
// Every thread maps one output pixel back to its nearest source pixel.
__global__ void rotate_rgb(const unsigned char* input,
                           unsigned char* output,
                           int width,
                           int height,
                           float c,
                           float s) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= width || y >= height) {
        return;
    }

    const float cx = (width - 1) * 0.5f;
    const float cy = (height - 1) * 0.5f;
    const float sx = c * (x - cx) - s * (y - cy) + cx;
    const float sy = s * (x - cx) + c * (y - cy) + cy;
    const int ix = int(floorf(sx + 0.5f));
    const int iy = int(floorf(sy + 0.5f));
    const size_t destination =
        (size_t(y) * size_t(width) + size_t(x)) * 3;

    if (ix >= 0 && ix < width && iy >= 0 && iy < height) {
        const size_t source =
            (size_t(iy) * size_t(width) + size_t(ix)) * 3;
        for (int channel = 0; channel < 3; ++channel) {
            output[destination + channel] = input[source + channel];
        }
    } else {
        for (int channel = 0; channel < 3; ++channel) {
            output[destination + channel] = 0;
        }
    }
}

/* Example host configuration after allocating and validating all buffers:

    dim3 block(16, 16);
    dim3 grid((width + 15) / 16, (height + 15) / 16);
    cudaMemcpy(d_input, h_input, bytes, cudaMemcpyHostToDevice);
    rotate_rgb<<<grid, block>>>(d_input, d_output, width, height, c, s);
    cudaMemcpy(h_output, d_output, bytes, cudaMemcpyDeviceToHost);

   In production, check each call and the launch using cudaGetLastError().
   If a dimension can approach INT_MAX, avoid overflow in width + 15 by using
   a checked wider calculation. Check byte counts and available GPU memory.

   Both kernels perform O(width * height) total work for three channels.
   Separate input/output buffers require O(width * height) memory each.
*/
