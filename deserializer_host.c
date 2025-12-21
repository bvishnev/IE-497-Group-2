#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>

// Utility function to check OpenCL errors
void check_error(cl_int err, const char* operation) {
    if (err != CL_SUCCESS) {
        printf("Error during operation '%s': %d\n", operation, err);
        exit(EXIT_FAILURE);
    }
}

// Utility function to load binary file
unsigned char* load_file(const char* filename, size_t* size) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("Error: Failed to open file %s\n", filename);
        return NULL;
    }
    
    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    rewind(fp);
    
    unsigned char* buffer = (unsigned char*)malloc(*size);
    if (!buffer) {
        printf("Error: Failed to allocate memory for file\n");
        fclose(fp);
        return NULL;
    }
    
    if (fread(buffer, 1, *size, fp) != *size) {
        printf("Error: Failed to read file\n");
        free(buffer);
        fclose(fp);
        return NULL;
    }
    
    fclose(fp);
    return buffer;
}

int main(int argc, char** argv) {
    // Check command line arguments
    if(argc != 2) {
        printf("Usage: %s <XCLBIN file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char* xclbin_file = argv[1];
    
    // Test parameters
    const unsigned int TEST_LENGTH = 1024;
    
    cl_int err;
    cl_platform_id platform_id;
    cl_device_id device_id;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem input_buffer, output_buffer;
    
    // Allocate host buffers
    uint8_t* input_host = (uint8_t*)malloc(TEST_LENGTH);
    uint8_t* output_host = (uint8_t*)malloc(TEST_LENGTH);
    uint8_t* expected_host = (uint8_t*)malloc(TEST_LENGTH);
    
    if(!input_host || !output_host || !expected_host) {
        printf("ERROR: Failed to allocate host memory\n");
        return EXIT_FAILURE;
    }
    
    // Get platform
    err = clGetPlatformIDs(1, &platform_id, NULL);
    check_error(err, "getting platform ID");
    
    // Get device
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ACCELERATOR, 1, &device_id, NULL);
    check_error(err, "getting device ID");
    
    // Create context
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    check_error(err, "creating context");
    
    // Create command queue
    queue = clCreateCommandQueue(context, device_id, 0, &err);
    check_error(err, "creating command queue");
    
    // Load xclbin
    printf("Loading XCLBIN: %s\n", xclbin_file);
    size_t xclbin_size;
    unsigned char* xclbin_data = load_file(xclbin_file, &xclbin_size);
    if (!xclbin_data) {
        printf("ERROR: Failed to load xclbin file\n");
        return EXIT_FAILURE;
    }
    
    // Create program from binary
    program = clCreateProgramWithBinary(context, 1, &device_id, &xclbin_size,
                                       (const unsigned char**)&xclbin_data, NULL, &err);
    check_error(err, "creating program with binary");
    free(xclbin_data);
    
    // Build program
    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    check_error(err, "building program");
    
    // Create kernel
    kernel = clCreateKernel(program, "deserialize", &err);
    check_error(err, "creating kernel");
    printf("Kernel loaded successfully\n\n");
    
    // Create device buffers
    input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, TEST_LENGTH, NULL, &err);
    check_error(err, "creating input buffer");
    
    output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, TEST_LENGTH, NULL, &err);
    check_error(err, "creating output buffer");
    
    // Sequential counting pattern test
    printf("Sequential Counting Pattern Test\n");
    printf("=================================\n");
    
    // Fill input buffer with counting pattern: input[i] = i
    for(unsigned int i = 0; i < TEST_LENGTH; i++) {
        input_host[i] = i % 256;
        expected_host[i] = i % 256;
    }
    
    // Write input data to device
    err = clEnqueueWriteBuffer(queue, input_buffer, CL_TRUE, 0, TEST_LENGTH, 
                               input_host, 0, NULL, NULL);
    check_error(err, "writing input buffer");
    
    // Clear output buffer
    memset(output_host, 0, TEST_LENGTH);
    err = clEnqueueWriteBuffer(queue, output_buffer, CL_TRUE, 0, TEST_LENGTH, 
                               output_host, 0, NULL, NULL);
    check_error(err, "writing output buffer");
    
    // Set kernel arguments
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
    check_error(err, "setting kernel arg 0");
    
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buffer);
    check_error(err, "setting kernel arg 1");
    
    err = clSetKernelArg(kernel, 2, sizeof(unsigned int), &TEST_LENGTH);
    check_error(err, "setting kernel arg 2");
    
    // Execute kernel
    err = clEnqueueTask(queue, kernel, 0, NULL, NULL);
    check_error(err, "executing kernel");
    
    // Wait for kernel to complete
    err = clFinish(queue);
    check_error(err, "waiting for kernel");
    
    // Read output data from device
    err = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0, TEST_LENGTH, 
                              output_host, 0, NULL, NULL);
    check_error(err, "reading output buffer");
    
    // Verify results
    int test_passed = 1;
    unsigned int errors = 0;
    for(unsigned int i = 0; i < TEST_LENGTH; i++) {
        if(output_host[i] != expected_host[i]) {
            if(errors < 10) {  // Only print first 10 errors
                printf("Error at index %u: Expected %u, Got %u\n", 
                       i, expected_host[i], output_host[i]);
            }
            errors++;
            test_passed = 0;
        }
    }
    
    printf("\n");
    if(test_passed) {
        printf("TEST PASSED: All %u bytes copied correctly\n", TEST_LENGTH);
        printf("  Pattern verified: output[i] == input[i] == i (mod 256)\n");
    } else {
        printf("TEST FAILED: %u errors found out of %u bytes\n", errors, TEST_LENGTH);
    }
    
    printf("\n");
    
    // Cleanup
    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    
    free(input_host);
    free(output_host);
    free(expected_host);
    
    return test_passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
