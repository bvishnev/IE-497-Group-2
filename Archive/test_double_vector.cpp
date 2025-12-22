#include <CL/cl2.hpp>
#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <xclbin>" << std::endl;
        return 1;
    }

    const int size = 1024;
    int* input = new int[size];
    int* output = new int[size];
    
    for (int i = 0; i < size; i++) {
        input[i] = i;
        output[i] = 0;
    }
    
    std::cout << "Running test with " << size << " elements..." << std::endl;

    try {
        // Get platform
        cl::Platform platform;
        cl::Platform::get(&platform);
        
        // Get FPGA device
        cl::Device device;
        platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &device);
        
        // Create context and queue
        cl::Context context(device);
        cl::CommandQueue queue(context, device);
        
        // Load xclbin file
        std::ifstream bin_file(argv[1], std::ifstream::binary);
        bin_file.seekg(0, bin_file.end);
        unsigned nb = bin_file.tellg();
        bin_file.seekg(0, bin_file.beg);
        char* buf = new char[nb];
        bin_file.read(buf, nb);
        
        cl::Program::Binaries bins;
        bins.push_back({buf, nb});
        
        cl::Program program(context, {device}, bins);
        cl::Kernel kernel(program, "double_vector");
        
        delete[] buf;
        
        // Create buffers
        cl::Buffer buffer_input(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, 
                               size * sizeof(int), input);
        cl::Buffer buffer_output(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, 
                                size * sizeof(int), output);
        
        // Set kernel arguments
        kernel.setArg(0, buffer_input);
        kernel.setArg(1, buffer_output);
        kernel.setArg(2, size);
        
        // Migrate data to device
        queue.enqueueMigrateMemObjects({buffer_input}, 0);
        
        // Execute kernel
        std::cout << "Executing kernel on FPGA..." << std::endl;
        queue.enqueueTask(kernel);
        
        // Migrate data back
        queue.enqueueMigrateMemObjects({buffer_output}, CL_MIGRATE_MEM_OBJECT_HOST);
        queue.finish();
        
        // Verify results
        std::cout << "Verifying results..." << std::endl;
        bool success = true;
        int error_count = 0;
        
        for (int i = 0; i < size; i++) {
            int expected = input[i] * 2;
            if (output[i] != expected) {
                if (error_count < 10) {
                    std::cout << "Error at index " << i << ": input=" << input[i] 
                             << ", expected=" << expected 
                             << ", got=" << output[i] << std::endl;
                }
                error_count++;
                success = false;
            }
        }
        
        // Print sample results
        std::cout << "\nSample results (first 10 elements):" << std::endl;
        for (int i = 0; i < 10; i++) {
            std::cout << "  input[" << i << "]=" << input[i] 
                     << " -> output[" << i << "]=" << output[i] 
                     << " (expected " << (input[i] * 2) << ")" << std::endl;
        }
        
        if (success) {
            std::cout << "\n✓ TEST PASSED! All " << size << " elements correct." << std::endl;
        } else {
            std::cout << "\n✗ TEST FAILED! " << error_count << " errors found." << std::endl;
        }
        
        delete[] input;
        delete[] output;
        
        return success ? 0 : 1;
        
    } catch (cl::Error& e) {
        std::cout << "OpenCL Error: " << e.what() << " (" << e.err() << ")" << std::endl;
        delete[] input;
        delete[] output;
        return 1;
    }
}
