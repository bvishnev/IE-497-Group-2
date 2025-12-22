#include <CL/opencl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <xclbin>\n", argv[0]);
        return 1;
    }

    const int SIZE = 1024;
    int *input = (int*)malloc(SIZE * sizeof(int));
    int *output = (int*)malloc(SIZE * sizeof(int));
    
    // Initialize test data
    for (int i = 0; i < SIZE; i++) {
        input[i] = i;
        output[i] = 0;
    }
    
    printf("Running test with %d elements...\n", SIZE);
    
    // Get platform
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);
    
    // Get device
    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 1, &device, NULL);
    
    // Create context
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    
    // Create command queue
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);
    
    // Load xclbin
    FILE* fp = fopen(argv[1], "rb");
    if (!fp) {
        printf("Error: Could not open %s\n", argv[1]);
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    size_t binary_size = ftell(fp);
    rewind(fp);
    
    unsigned char* binary = (unsigned char*)malloc(binary_size);
    fread(binary, 1, binary_size, fp);
    fclose(fp);
    
    // Create program
    cl_program program = clCreateProgramWithBinary(context, 1, &device, 
                                                   &binary_size, 
                                                   (const unsigned char**)&binary, 
                                                   NULL, NULL);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    
    // Create kernel
    cl_kernel kernel = clCreateKernel(program, "double_vector", NULL);
    
    // Create buffers
    cl_mem buffer_input = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                         SIZE * sizeof(int), input, NULL);
    cl_mem buffer_output = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                          SIZE * sizeof(int), output, NULL);
    
    // Set kernel arguments
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_input);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_output);
    clSetKernelArg(kernel, 2, sizeof(int), &SIZE);
    
    // Migrate data to device
    clEnqueueMigrateMemObjects(queue, 1, &buffer_input, 0, 0, NULL, NULL);
    
    // Execute kernel
    printf("Executing kernel on FPGA...\n");
    clEnqueueTask(queue, kernel, 0, NULL, NULL);
    
    // Migrate data back
    clEnqueueMigrateMemObjects(queue, 1, &buffer_output, CL_MIGRATE_MEM_OBJECT_HOST, 0, NULL, NULL);
    clFinish(queue);
    
    // Verify results
    printf("Verifying results...\n");
    int success = 1;
    int error_count = 0;
    
    for (int i = 0; i < SIZE; i++) {
        int expected = input[i] * 2;
        if (output[i] != expected) {
            if (error_count < 10) {
                printf("Error at index %d: input=%d, expected=%d, got=%d\n", 
                       i, input[i], expected, output[i]);
            }
            error_count++;
            success = 0;
        }
    }
    
    // Print sample results
    printf("\nSample results (first 10 elements):\n");
    for (int i = 0; i < 10; i++) {
        printf("  input[%d]=%d -> output[%d]=%d (expected %d)\n", 
               i, input[i], i, output[i], input[i] * 2);
    }
    
    if (success) {
        printf("\n✓ TEST PASSED! All %d elements correct.\n", SIZE);
    } else {
        printf("\n✗ TEST FAILED! %d errors found.\n", error_count);
    }
    
    // Cleanup
    clReleaseMemObject(buffer_input);
    clReleaseMemObject(buffer_output);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    free(input);
    free(output);
    free(binary);
    
    return success ? 0 : 1;
}
