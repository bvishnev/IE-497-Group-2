// parser_host.c
#include <CL/opencl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Make sure struct layout matches kernel: avoid padding */
typedef struct __attribute__((packed)) {
    uint8_t data;
    uint8_t valid;
    uint8_t start_msg;
    uint8_t end_msg;
} ByteData;

typedef struct __attribute__((packed)) {
    uint8_t valid_msg;
    uint8_t msg_type;
    uint16_t stock_locate;
    uint16_t tracking_no;
    uint64_t timestamp;
    uint64_t order_ref_no;
    uint32_t shares;
    uint8_t buy_sell;
    uint64_t stock;
    uint32_t price;
    uint64_t match_no;
    uint64_t new_order_ref_no;
    uint32_t attribution;
} ParserOutput;

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <xclbin>\n", argv[0]);
        return 1;
    }

    const char *xclbinPath = argv[1];

    // --- Build the input buffer according to the Verilog testbench sequence ---
    const int MAX_BYTES = 1024;
    ByteData *input = (ByteData*)calloc(MAX_BYTES, sizeof(ByteData));
    if (!input) { perror("calloc input"); return 1; }
    int in_pos = 0;
    #define PUSH_BYTE(d, v, s, e) do { \
        input[in_pos].data = (uint8_t)(d); \
        input[in_pos].valid = (uint8_t)(v); \
        input[in_pos].start_msg = (uint8_t)(s); \
        input[in_pos].end_msg = (uint8_t)(e); \
        in_pos++; \
    } while(0)

    /* Recreate testbench sequence (copy of sends in parser_tb) */
    // Valid "A" type message
    PUSH_BYTE(0x41,1,1,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x05,1,0,0);
    PUSH_BYTE(0xAA,1,0,0);
    PUSH_BYTE(0xBB,1,0,0);
    PUSH_BYTE(0xCC,1,0,0);
    PUSH_BYTE(0xDD,1,0,0);
    PUSH_BYTE(0xEE,1,0,0);
    PUSH_BYTE(0xFF,1,0,0);
    PUSH_BYTE(0x11,1,0,0);
    PUSH_BYTE(0x22,1,0,0);
    PUSH_BYTE(0x42,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x10,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x53,1,0,0);
    PUSH_BYTE(0x54,1,0,0);
    PUSH_BYTE(0x4F,1,0,0);
    PUSH_BYTE(0x43,1,0,0);
    PUSH_BYTE(0x4B,1,0,0);
    PUSH_BYTE(0x20,1,0,0);
    PUSH_BYTE(0x20,1,0,0);
    PUSH_BYTE(0x20,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,1);

    // In-between message garbage
    PUSH_BYTE(0xF8,1,0,0);
    PUSH_BYTE(0x32,0,0,0);

    // Invalid "A" type message (one invalid byte)
    PUSH_BYTE(0x41,1,1,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x05,1,0,0);
    PUSH_BYTE(0xAA,1,0,0);
    PUSH_BYTE(0xBB,1,0,0);
    PUSH_BYTE(0xCC,1,0,0);
    PUSH_BYTE(0xDD,1,0,0);
    PUSH_BYTE(0xEE,0,0,0); // invalid byte (valid==0)
    PUSH_BYTE(0xFF,1,0,0);
    PUSH_BYTE(0x11,1,0,0);
    PUSH_BYTE(0x22,1,0,0);
    PUSH_BYTE(0x42,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x10,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x53,1,0,0);
    PUSH_BYTE(0x54,1,0,0);
    PUSH_BYTE(0x4F,1,0,0);
    PUSH_BYTE(0x43,1,0,0);
    PUSH_BYTE(0x4B,1,0,0);
    PUSH_BYTE(0x20,1,0,0);
    PUSH_BYTE(0x20,1,0,0);
    PUSH_BYTE(0x20,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,1);

    // Valid "F" type message
    PUSH_BYTE(0x46,1,1,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x05,1,0,0);
    PUSH_BYTE(0xAA,1,0,0);
    PUSH_BYTE(0xBB,1,0,0);
    PUSH_BYTE(0xCC,1,0,0);
    PUSH_BYTE(0xDD,1,0,0);
    PUSH_BYTE(0xEE,1,0,0);
    PUSH_BYTE(0xFF,1,0,0);
    PUSH_BYTE(0x11,1,0,0);
    PUSH_BYTE(0x22,1,0,0);
    PUSH_BYTE(0x53,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x10,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x53,1,0,0);
    PUSH_BYTE(0x54,1,0,0);
    PUSH_BYTE(0x4F,1,0,0);
    PUSH_BYTE(0x43,1,0,0);
    PUSH_BYTE(0x4B,1,0,0);
    PUSH_BYTE(0x20,1,0,0);
    PUSH_BYTE(0x20,1,0,0);
    PUSH_BYTE(0x20,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x41,1,0,0);
    PUSH_BYTE(0x44,1,0,0);
    PUSH_BYTE(0x41,1,0,0);
    PUSH_BYTE(0x4D,1,0,1);

    // Valid "E" Type Message
    PUSH_BYTE(0x45,1,1,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x05,1,0,0);
    PUSH_BYTE(0xAA,1,0,0);
    PUSH_BYTE(0xBB,1,0,0);
    PUSH_BYTE(0xCC,1,0,0);
    PUSH_BYTE(0xDD,1,0,0);
    PUSH_BYTE(0xEE,1,0,0);
    PUSH_BYTE(0xFF,1,0,0);
    PUSH_BYTE(0x11,1,0,0);
    PUSH_BYTE(0x22,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x10,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0xAA,1,0,0);
    PUSH_BYTE(0xBB,1,0,0);
    PUSH_BYTE(0xCC,1,0,0);
    PUSH_BYTE(0xDD,1,0,0);
    PUSH_BYTE(0xEE,1,0,0);
    PUSH_BYTE(0xFF,1,0,0);
    PUSH_BYTE(0x67,1,0,0);
    PUSH_BYTE(0x69,1,0,1);

    // Valid "X" Type Message
    PUSH_BYTE(0x58,1,1,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x05,1,0,0);
    PUSH_BYTE(0xAA,1,0,0);
    PUSH_BYTE(0xBB,1,0,0);
    PUSH_BYTE(0xCC,1,0,0);
    PUSH_BYTE(0xDD,1,0,0);
    PUSH_BYTE(0xEE,1,0,0);
    PUSH_BYTE(0xFF,1,0,0);
    PUSH_BYTE(0x11,1,0,0);
    PUSH_BYTE(0x22,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x10,1,0,1);

    // Valid "D" Type Message
    PUSH_BYTE(0x44,1,1,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x05,1,0,0);
    PUSH_BYTE(0xAA,1,0,0);
    PUSH_BYTE(0xBB,1,0,0);
    PUSH_BYTE(0xCC,1,0,0);
    PUSH_BYTE(0xDD,1,0,0);
    PUSH_BYTE(0xEE,1,0,0);
    PUSH_BYTE(0xFF,1,0,0);
    PUSH_BYTE(0x11,1,0,1);

    // Valid "U" Type Message
    PUSH_BYTE(0x55,1,1,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x05,1,0,0);
    PUSH_BYTE(0xAA,1,0,0);
    PUSH_BYTE(0xBB,1,0,0);
    PUSH_BYTE(0xCC,1,0,0);
    PUSH_BYTE(0xDD,1,0,0);
    PUSH_BYTE(0xEE,1,0,0);
    PUSH_BYTE(0xFF,1,0,0);
    PUSH_BYTE(0x11,1,0,0);
    PUSH_BYTE(0x22,1,0,0);
    PUSH_BYTE(0x67,1,0,0);
    PUSH_BYTE(0x67,1,0,0);
    PUSH_BYTE(0x67,1,0,0);
    PUSH_BYTE(0x67,1,0,0);
    PUSH_BYTE(0x67,1,0,0);
    PUSH_BYTE(0x67,1,0,0);
    PUSH_BYTE(0x67,1,0,0);
    PUSH_BYTE(0x67,1,0,1);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x10,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x00,1,0,0);
    PUSH_BYTE(0x01,1,0,0);
    PUSH_BYTE(0x02,1,0,0);
    PUSH_BYTE(0x03,1,0,1);

    // Invalid message type
    PUSH_BYTE(0x5A,1,1,0);
    PUSH_BYTE(0x04,1,0,0);
    PUSH_BYTE(0x05,1,0,0);
    PUSH_BYTE(0xAA,1,0,0);
    PUSH_BYTE(0xBB,1,0,0);
    PUSH_BYTE(0xCC,1,0,0);
    PUSH_BYTE(0xDD,1,0,0);
    PUSH_BYTE(0xEE,1,0,0);
    PUSH_BYTE(0xFF,1,0,1);

    #undef PUSH_BYTE

    int num_bytes = in_pos;
    printf("Prepared %d bytes for parser TB sequence\n", num_bytes);

    // --- prepare output buffers ---
    const int MAX_OUT = 32;
    ParserOutput *output = (ParserOutput*)calloc(MAX_OUT, sizeof(ParserOutput));
    if (!output) { perror("calloc output"); free(input); return 1; }
    int num_outputs = 0;

    // --- OpenCL / Xilinx flow (mirrors your double_vector host) ---
    // Platform
    cl_platform_id platform;
    cl_uint num_platforms = 0;
    clGetPlatformIDs(1, &platform, &num_platforms);

    // Device
    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 1, &device, NULL);

    // Context
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);

    // Queue
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

    // Load xclbin
    FILE* fp = fopen(xclbinPath, "rb");
    if (!fp) { printf("Error: could not open %s\n", xclbinPath); free(input); free(output); return 1; }
    fseek(fp, 0, SEEK_END);
    size_t binary_size = ftell(fp);
    rewind(fp);
    unsigned char *binary = (unsigned char*)malloc(binary_size);
    fread(binary, 1, binary_size, fp);
    fclose(fp);

    // Program
    cl_int err;
    cl_program program = clCreateProgramWithBinary(context, 1, &device,
                                                   &binary_size, (const unsigned char**)&binary,
                                                   NULL, &err);
    if (err != CL_SUCCESS) { printf("clCreateProgramWithBinary failed: %d\n", err); return 1; }
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    // Kernel
    cl_kernel kernel = clCreateKernel(program, "parser", &err);
    if (err != CL_SUCCESS) { printf("clCreateKernel failed: %d\n", err); return 1; }

    // Buffers (use host ptr so we can read/write directly)
    cl_mem buffer_input = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                         num_bytes * sizeof(ByteData), input, &err);
    if (err != CL_SUCCESS) { printf("buffer_input create failed: %d\n", err); return 1; }

    cl_mem buffer_output = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                          MAX_OUT * sizeof(ParserOutput), output, &err);
    if (err != CL_SUCCESS) { printf("buffer_output create failed: %d\n", err); return 1; }

    // num_outputs is a device-writable integer; create a buffer for it
    int num_outputs_host = 0;
    cl_mem buffer_num_outputs = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                                               sizeof(int), &num_outputs_host, &err);
    if (err != CL_SUCCESS) { printf("buffer_num_outputs create failed: %d\n", err); return 1; }

    // Set kernel args
    // Signature: parser(const ByteData* input_stream, int num_bytes, ParserOutput* output_stream, int* num_outputs)
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_input);
    clSetKernelArg(kernel, 1, sizeof(int), &num_bytes);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer_output);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &buffer_num_outputs);

    // Migrate input buffer to device
    clEnqueueMigrateMemObjects(queue, 1, &buffer_input, 0, 0, NULL, NULL);

    // Also migrate the zero-initialized num_outputs (not strictly necessary but safe)
    clEnqueueMigrateMemObjects(queue, 1, &buffer_num_outputs, 0, 0, NULL, NULL);

    // Execute kernel
    printf("Executing parser kernel on FPGA...\n");
    clEnqueueTask(queue, kernel, 0, NULL, NULL);

    // Migrate output buffer and num_outputs back
    clEnqueueMigrateMemObjects(queue, 1, &buffer_output, CL_MIGRATE_MEM_OBJECT_HOST, 0, NULL, NULL);
    clEnqueueMigrateMemObjects(queue, 1, &buffer_num_outputs, CL_MIGRATE_MEM_OBJECT_HOST, 0, NULL, NULL);

    clFinish(queue);

    // After migration, num_outputs_host should contain the number of valid messages
    num_outputs = num_outputs_host;
    if (num_outputs > MAX_OUT) num_outputs = MAX_OUT;
    printf("Parser produced %d valid messages\n", num_outputs);

    // Print monitor-like output for each parsed message (similar fields to TB monitor)
    for (int i = 0; i < num_outputs; ++i) {
        ParserOutput *m = &output[i];
        printf("MSG[%d]: valid=%u type=0x%02x stock_locate=0x%04x tracking_no=0x%04x timestamp=0x%012llx order_ref=0x%016llx shares=%u buy_sell=0x%02x stock=0x%016llx price=%u match_no=0x%016llx new_order_ref=0x%016llx attrib=%u\n",
               i,
               (unsigned)m->valid_msg,
               (unsigned)m->msg_type,
               (unsigned)m->stock_locate,
               (unsigned)m->tracking_no,
               (unsigned long long)m->timestamp,
               (unsigned long long)m->order_ref_no,
               (unsigned)m->shares,
               (unsigned)m->buy_sell,
               (unsigned long long)m->stock,
               (unsigned)m->price,
               (unsigned long long)m->match_no,
               (unsigned long long)m->new_order_ref_no,
               (unsigned)m->attribution
               );
    }

    // cleanup
    clReleaseMemObject(buffer_input);
    clReleaseMemObject(buffer_output);
    clReleaseMemObject(buffer_num_outputs);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    free(input);
    free(output);
    free(binary);

    return 0;
}
