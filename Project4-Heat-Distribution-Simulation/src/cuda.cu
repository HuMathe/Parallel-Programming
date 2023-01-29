#include "heat_system.h"
#include "signature.h"
#include "gui.h"

#include <cuda.h>
#include <iostream>

__global__ void kernel_jacobi_step(double *current_state, int num_row, int num_col, double *result_buffer)
{
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  int i = idx / num_col, j = idx % num_col;
  if(idx >= num_col * num_row) return ;
  if(i == 0 || i == num_row - 1 || j == 0 || j == num_col - 1)
    result_buffer[idx] = 0.2;
  else
    result_buffer[idx] = 0.25 * (current_state[idx + 1] + current_state[idx - 1]
      + current_state[idx + num_col] + current_state[idx - num_col]);
}

__global__ void set_value(double *buffer, int *indices, int size, double value)
{
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if(idx < size)
  {
    buffer[indices[idx]] = value;
  }
}

class cuda_simulation
  : public grid_system
{
public:
  int ThreadPerBlock;
  double * device_state, * device_buffer;
  int * device_fire_indices, fire_size;

public:
  void init()
  {
    device_fire_indices = nullptr; // there is no fire orignally
    fire_size = 0;

    if(size == 0)
    {
      device_state = nullptr;
      device_buffer = nullptr;
      return ;
    }

    size_t buffer_size = sizeof(double) * size;
    cudaMalloc(&device_state, buffer_size);
    cudaMalloc(&device_buffer, buffer_size);
  }

  cuda_simulation(): grid_system() { init(); }
  cuda_simulation(int num_row, int num_col, int thread_per_block)
    : grid_system(num_row, num_col), ThreadPerBlock(thread_per_block)
  {  init(); }

  ~cuda_simulation()
  {
    if(size == 0) 
      return ;

    cudaFree(device_fire_indices);
    cudaFree(device_state);
    cudaFree(device_buffer);
  }

  void update_state() override
  {
    kernel_jacobi_step<<<(size - 1) / ThreadPerBlock + 1, ThreadPerBlock>>>(
      device_state, num_row, num_col, device_buffer
    );
    cudaDeviceSynchronize(); 
  }

  void sync_state() override
  {
    cudaMemcpy(data, device_state, sizeof(double) * size, cudaMemcpyDeviceToHost); 
  }

  void init_shared_memory() override
  {
    cudaMemcpy(device_state, data, sizeof(double) * size, cudaMemcpyHostToDevice);

    int *indices = new int[this->fire.size()];
    for(index_iterator it = this->fire.begin(); it != this->fire.end(); it++)
    {
      indices[fire_size++] = it->first * num_col + it->second;
    }
    cudaMalloc(&device_fire_indices, sizeof(int) * fire_size);
    cudaMemcpy(device_fire_indices, indices, sizeof(int) * fire_size, cudaMemcpyHostToDevice);
    delete[] indices;
  }

  void switch_buffer() override
  {
    set_value<<<(fire_size - 1) / ThreadPerBlock + 1, ThreadPerBlock>>>(
      device_buffer, device_fire_indices, fire_size, 1.0
    );
    cudaDeviceSynchronize();

    std::swap(device_state, device_buffer);
  }

};

int num_row, num_col, thread_per_block, num_iter;

int main(int argc, char **argv)
{
  if(argc == 4)
  {
    num_row = num_col = atoi(argv[1]);
    num_iter = atoi(argv[2]);
    thread_per_block = atoi(argv[3]);
  }else if(argc == 5)
  {
    num_row = atoi(argv[1]);
    num_col = atoi(argv[2]);
    num_iter = atoi(argv[3]);
    thread_per_block = atoi(argv[4]);
  }else{
    num_row = num_col = 800;
    num_iter = 100;
    thread_per_block = 256;
  }

  gui_init(&argc, argv, "CUDA", 800, 800);
  cuda_simulation heat_state(num_row, num_col, thread_per_block);

  heat_state.init_temperature(FireType::single_fire_circular);

  for(int i = 0; i < num_iter; i++)
  {
    heat_state.step();
    visualize(&heat_state, 800, 800);
  }

  print_info("CUDA");
  return 0;
}