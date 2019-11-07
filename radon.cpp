#include <torch/extension.h>

#include <iostream>
#include <vector>

// CUDA forward declarations
void radon_forward_cuda(const float* x, const float* rays, const float* angles, float* y, const int batch_size, const int img_size, const int n_rays, const int n_angles);


#define CHECK_CUDA(x) TORCH_CHECK(x.type().is_cuda(), #x " must be a CUDA tensor")
#define CHECK_CONTIGUOUS(x) TORCH_CHECK(x.is_contiguous(), #x " must be contiguous")
#define CHECK_INPUT(x) CHECK_CUDA(x); CHECK_CONTIGUOUS(x)


torch::Tensor radon_forward(torch::Tensor x, torch::Tensor rays, torch::Tensor angles) {
    CHECK_INPUT(x);
    CHECK_INPUT(rays);
    CHECK_INPUT(angles);

    const int batch_size = x.size(0);
    const int img_size = x.size(1);
    TORCH_CHECK(x.size(2) == img_size, "Images in x must be squared")

    const int n_rays = rays.size(0);
    const int n_angles = angles.size(0);

    // create output sinogram tensor
    auto options = torch::TensorOptions().dtype(torch::kFloat32).device(x.device());
    auto y = torch::empty({batch_size, n_angles, n_rays}, options);

    radon_forward_cuda(x.data<float>(), rays.data<float>(), angles.data<float>(), y.data<float>(),
                       batch_size, img_size, n_rays, n_angles);

    return y;
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
  m.def("forward", &radon_forward, "Radon forward");
}
