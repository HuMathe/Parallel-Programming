#include "heat_system.h"
#include "signature.h"
#include "gui.h"

#include <mpi.h>
#include <omp.h>
#include <cmath>
#include <cstring>
#include <iostream>

class hybrid_simulation
  : public grid_system
{
public:
  int rank, num_proc;
  int block_size;
  int first_idx, last_idx;
  int *fire_indices, fire_size;
  int threads_per_proc;


public:
  void init()
  {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    
    fire_indices = nullptr;

    if(size == 0)
      return ;
    
    block_size = (size - 1) / num_proc + 1;

    first_idx = rank * block_size;
    last_idx = std::min(size, first_idx + block_size);

    delete[] data;
    delete[] swp_data;

    data = new double[block_size * num_proc];
    swp_data = new double[block_size * num_proc];

    for(int i = 0; i < num_row; i++)
    {
      this->heat[i] = this->data + i * this->num_col;
      this->swp_heat[i] = this->swp_data + i * this->num_col;
    }

    if(rank != 0)
      mute = 1;
  }

  hybrid_simulation(): grid_system(), threads_per_proc(0) { init(); }
  hybrid_simulation(int num_row, int num_col, int thds_per_proc)
    : grid_system(num_row, num_col), threads_per_proc(thds_per_proc) { init(); }

  ~hybrid_simulation()
  {
    if(fire_indices != nullptr)
      delete[] fire_indices;
  }

  void update_state() override
  {
    MPI_Status recv[2];
    MPI_Request send[2];
    if(rank != num_proc - 1)
      MPI_Isend(data + last_idx - num_col, num_col, MPI_DOUBLE, rank + 1, rank, MPI_COMM_WORLD, send);
      
    if(rank != 0)
      MPI_Isend(data + first_idx, num_col, MPI_DOUBLE, rank - 1, rank * 2, MPI_COMM_WORLD, send + 1);

    if(rank != num_proc - 1)
      MPI_Recv(data + last_idx, num_col, MPI_DOUBLE, rank + 1, rank * 2 + 2, MPI_COMM_WORLD, recv);
    
    if(rank != 0)
      MPI_Recv(data + first_idx - num_col, num_col, MPI_DOUBLE, rank - 1, rank - 1, MPI_COMM_WORLD, recv + 1);
    
    int row_idx, col_idx, i;

    #pragma omp parallel num_threads(this->threads_per_proc) default(shared) private(i, row_idx, col_idx)
    {
      for(i = first_idx + omp_get_thread_num(); i < last_idx; i += threads_per_proc)
      {
        row_idx = i / num_col, col_idx = i % num_col;
        if(row_idx == 0 || row_idx == num_row - 1 || col_idx == 0 || col_idx == num_col - 1)
          swp_data[i] = 0.2;
        else
          swp_data[i] = 0.25 * (data[i - num_col] + data[i + num_col] + data[i + 1] + data[i - 1]);
      }
    }
    
  }


  void init_shared_memory() override 
  {
    fire_size = this->fire.size();
    MPI_Bcast(&fire_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(data, block_size * num_proc, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    fire_indices = new int[fire_size];
    if(rank == 0)
      for(int i = 0; i < fire_size; i++)
        fire_indices[i] = fire[i].first * num_col + fire[i].second;
    MPI_Bcast(fire_indices, fire_size, MPI_INT, 0, MPI_COMM_WORLD);

    int local_fire_size = 0;
    
    for(int i = 0; i < fire_size; i++)
      if(first_idx <= fire_indices[i] && fire_indices[i] < last_idx)
        fire_indices[local_fire_size++] = fire_indices[i];
    fire_size = local_fire_size;
  }

  void switch_buffer() override
  {
    int i;
    #pragma omp parallel num_threads(threads_per_proc) default(shared) private(i)
    {
      for(i = omp_get_thread_num(); i < fire_size; i += threads_per_proc)
      {
        swp_data[fire_indices[i]] = 1.0;
      }
    }
    

    std::swap(swp_data, data);
    std::swap(swp_heat, heat);
  }
};

int rank, num_row, num_col, num_iter, thds_per_proc;
int main(int argc, char **argv)
{
  if(argc == 4)
  {
    num_row = num_col = atoi(argv[1]);
    num_iter = atoi(argv[2]);
    thds_per_proc = atoi(argv[3]);
  }else if(argc == 5)
  {
    num_row = atoi(argv[1]);
    num_col = atoi(argv[2]);
    num_iter = atoi(argv[3]);
    thds_per_proc = atoi(argv[4]);
  }else{
    num_col = num_row = 800;
    num_iter = 100;
    thds_per_proc = 1;
  }

  MPI_Init(&argc, &argv);
  
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank == 0)
    gui_init(&argc, argv, "MPI + OpenMP", 800, 800);

  hybrid_simulation heat_state(num_row, num_col, thds_per_proc);

  heat_state.init_temperature(FireType::single_fire_circular);

  for(int i = 0; i < num_iter; i++)
  {
    heat_state.step();

    MPI_Gather(heat_state.data + heat_state.first_idx, heat_state.block_size,\
      MPI_DOUBLE, heat_state.swp_data, heat_state.block_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if(rank == 0)
    {
      memcpy(heat_state.data, heat_state.swp_data, sizeof(double) * heat_state.size);
      visualize(&heat_state, 800, 800);
    }
      
  }

  MPI_Finalize();

  if(rank == 0)
    print_info("MPI + OpenMP");
  return 0;
}
