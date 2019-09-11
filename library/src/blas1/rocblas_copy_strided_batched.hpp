/* ************************************************************************
 * Copyright 2016-2019 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#include "handle.h"
#include "logging.h"
#include "rocblas.h"
#include "utility.h"

constexpr int NB = 256;

template <typename T>
__global__ void copy_strided_batched_kernel(rocblas_int n,
                                            const T*    xa,
                                            rocblas_int    shiftx,
                                            rocblas_int incx,
                                            rocblas_int stridex,
                                            T*          ya,
                                            rocblas_int    shifty,
                                            rocblas_int incy,
                                            rocblas_int stridey)
{
    ptrdiff_t tid = hipBlockIdx_x * hipBlockDim_x + hipThreadIdx_x;
    // bound
    if(tid < n)
    {
        const T* x;
        T*       y;
        x = xa + hipBlockIdx_y * stridex + shiftx;
        y = ya + hipBlockIdx_y * stridey + shifty;

        if(incx < 0)
            x -= ptrdiff_t(incx) * (n - 1);
        if(incy < 0)
            y -= ptrdiff_t(incy) * (n - 1);

        y[tid * incy] = x[tid * incx];
    }
}

template <class T>
rocblas_status rocblas_copy_strided_batched_template(rocblas_handle handle,
                                                     rocblas_int    n,
                                                     const T*       x,
                                                     rocblas_int    shiftx,
                                                     rocblas_int    incx,
                                                     rocblas_int    stridex,
                                                     T*             y,
                                                     rocblas_int    shifty,
                                                     rocblas_int    incy,
                                                     rocblas_int    stridey,
                                                     rocblas_int    batched_count)
{
    // Quick return if possible.
    if(!n || !batch_count)
        return rocblas_status_success;
        
    int  blocks = (n - 1) / NB + 1;
    dim3 grid(blocks, batched_count);
    dim3 threads(NB);

    hipStream_t rocblas_stream = handle->rocblas_stream;

    hipLaunchKernelGGL(copy_strided_batched_kernel,
                       grid,
                       threads,
                       0,
                       rocblas_stream,
                       n,
                       x,
                       shiftx,
                       incx,
                       stridex,
                       y,
                       shiftx,
                       incy,
                       stridey);

    return rocblas_status_success;
}