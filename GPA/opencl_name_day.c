#include <stdio.h>
#include <CL/cl.h>

const char *kernelSource =
"__kernel void printNameDay() {                       \n"
"   int id = get_global_id(0);                        \n"
"   printf(\"Global ID %d: Neeti - Tuesday\\n\", id); \n"
"}                                                    \n";

int main() {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;

    // 1. Get platform and device
    clGetPlatformIDs(1, &platform, NULL);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);

    // 2. Create context and command queue
    context = clCreateContext(0, 1, &device, NULL, NULL, NULL);
    queue = clCreateCommandQueue(context, device, 0, NULL);

    // 3. Build program
    program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, NULL);
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    // 4. Create kernel
    kernel = clCreateKernel(program, "printNameDay", NULL);

    // 5. Run kernel (5 work-items)
    size_t globalSize = 5;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    clFinish(queue);

    // 6. Cleanup
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}