#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// XRT includes
#include "xrt/xrt_kernel.h"
#include "xrt/xrt_bo.h"
#include "experimental/xrt_xclbin.h"

int main(int argc, char** argv) {
    // Check command line arguments
    if(argc != 2) {
        printf("Usage: %s <XCLBIN file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char* xclbin_file = argv[1];
    
    // Test parameters
    const unsigned int TEST_LENGTH = 1024;
    
    // Initialize XRT
    printf("Loading XCLBIN: %s\n", xclbin_file);
    xrtDeviceHandle device = xrtDeviceOpen(0);
    if(!device) {
        printf("ERROR: Failed to open device\n");
        return EXIT_FAILURE;
    }
    
    xuid_t uuid;
    if(xrtDeviceLoadXclbinFile(device, xclbin_file)) {
        printf("ERROR: Failed to load xclbin\n");
        xrtDeviceClose(device);
        return EXIT_FAILURE;
    }
    
    if(xrtDeviceGetXclbinUUID(device, uuid)) {
        printf("ERROR: Failed to get xclbin UUID\n");
        xrtDeviceClose(device);
        return EXIT_FAILURE;
    }
    
    // Get kernel
    xrtKernelHandle kernel = xrtPLKernelOpen(device, uuid, "deserialize");
    if(!kernel) {
        printf("ERROR: Failed to open kernel\n");
        xrtDeviceClose(device);
        return EXIT_FAILURE;
    }
    printf("Kernel loaded successfully\n\n");
    
    // Allocate host buffers
    uint8_t* input_host = (uint8_t*)malloc(TEST_LENGTH);
    uint8_t* output_host = (uint8_t*)malloc(TEST_LENGTH);
    uint8_t* expected_host = (uint8_t*)malloc(TEST_LENGTH);
    
    if(!input_host || !output_host || !expected_host) {
        printf("ERROR: Failed to allocate host memory\n");
        xrtKernelClose(kernel);
        xrtDeviceClose(device);
        return EXIT_FAILURE;
    }
    
    // Create device buffers
    xrtBufferHandle input_bo = xrtBOAlloc(device, TEST_LENGTH, 0, 0);
    xrtBufferHandle output_bo = xrtBOAlloc(device, TEST_LENGTH, 0, 1);
    
    if(!input_bo || !output_bo) {
        printf("ERROR: Failed to allocate device buffers\n");
        free(input_host);
        free(output_host);
        free(expected_host);
        xrtKernelClose(kernel);
        xrtDeviceClose(device);
        return EXIT_FAILURE;
    }
    
    // Sequential counting pattern test
    printf("Sequential Counting Pattern Test\n");
    printf("=================================\n");
    
    // Fill input buffer with counting pattern: input[i] = i
    for(unsigned int i = 0; i < TEST_LENGTH; i++) {
        input_host[i] = i % 256;
        expected_host[i] = i % 256;
    }
    
    // Write input data to device
    if(xrtBOWrite(input_bo, input_host, TEST_LENGTH, 0)) {
        printf("ERROR: Failed to write input buffer\n");
        goto cleanup;
    }
    if(xrtBOSync(input_bo, XCL_BO_SYNC_BO_TO_DEVICE, TEST_LENGTH, 0)) {
        printf("ERROR: Failed to sync input buffer to device\n");
        goto cleanup;
    }
    
    // Clear output buffer
    memset(output_host, 0, TEST_LENGTH);
    if(xrtBOWrite(output_bo, output_host, TEST_LENGTH, 0)) {
        printf("ERROR: Failed to write output buffer\n");
        goto cleanup;
    }
    if(xrtBOSync(output_bo, XCL_BO_SYNC_BO_TO_DEVICE, TEST_LENGTH, 0)) {
        printf("ERROR: Failed to sync output buffer to device\n");
        goto cleanup;
    }
    
    // Run kernel
    xrtRunHandle run = xrtKernelRun(kernel, input_bo, output_bo, TEST_LENGTH);
    if(!run) {
        printf("ERROR: Failed to run kernel\n");
        goto cleanup;
    }
    
    // Wait for kernel to complete
    xrtRunWait(run);
    xrtRunClose(run);
    
    // Read output data from device
    if(xrtBOSync(output_bo, XCL_BO_SYNC_BO_FROM_DEVICE, TEST_LENGTH, 0)) {
        printf("ERROR: Failed to sync output buffer from device\n");
        goto cleanup;
    }
    if(xrtBORead(output_bo, output_host, TEST_LENGTH, 0)) {
        printf("ERROR: Failed to read output buffer\n");
        goto cleanup;
    }
    
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
        printf("Pattern verified: output[i] == input[i] == i (mod 256)\n");
    } else {
        printf("TEST FAILED: %u errors found out of %u bytes\n", errors, TEST_LENGTH);
    }
    
    printf("\n");
    
cleanup:
    // Cleanup
    free(input_host);
    free(output_host);
    free(expected_host);
    xrtBOFree(input_bo);
    xrtBOFree(output_bo);
    xrtKernelClose(kernel);
    xrtDeviceClose(device);
    
    return test_passed ? EXIT_SUCCESS : EXIT_FAILURE;
}