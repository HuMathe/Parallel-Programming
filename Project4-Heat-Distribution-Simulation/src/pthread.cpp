#include "heat_system.h"
#include "signature.h"
#include "gui.h"

#include <pthread.h>
#include <iostream>

class pthread_simulation
  : public grid_system
{
public:
  int num_threads;
  pthread_t * workers;
  struct pthread_arg
  {
    int thread_idx;
    pthread_simulation * system_ptr;
  } *args;
  

public:
  pthread_simulation() : grid_system(), num_threads(0) 
  {
    workers = nullptr;
    args = nullptr;
  }

  pthread_simulation(int num_row, int num_col, int num_thds)
    : grid_system(num_row, num_col), num_threads(num_thds) 
  {
    workers = new pthread_t[num_thds];
    args = new pthread_arg[num_thds];

    for(int i = 0; i < num_thds; i++)
    {
      args[i].thread_idx = i;
      args[i].system_ptr = this;
    }
  }

  ~pthread_simulation()
  {
    if(workers != nullptr)
      delete[] workers;
    if(args != nullptr)
      delete[] args;
    workers = nullptr;
    args = nullptr;
  }

  // static scheduler
  static void* pthread_helper(void * void_args)
  {
    pthread_arg * args = (pthread_arg *) void_args;
    pthread_simulation * sys_this = args->system_ptr;
    int thread_idx = args->thread_idx, num_threads = sys_this->num_threads;
    int fire_size = sys_this->fire.size(), size = sys_this->size;
    int num_col = sys_this->num_col, num_row = sys_this->num_row;
    
    std::pair<int, int> *fire = sys_this->fire.data();
    double **h = sys_this->heat, **new_h = sys_this->swp_heat;

    int idx, i, j;

    int block_size = (size - 1) / num_threads + 1, end = std::min(block_size * (thread_idx + 1), size);
    for(idx = thread_idx * block_size; idx < end; idx ++)
    {
      i = idx / num_col, j = idx % num_col;
      if(i == 0 || i == num_row - 1 || j == 0 || j == num_col - 1)
        new_h[i][j] = 0.2;
      else
        new_h[i][j] = 0.25 * (h[i - 1][j] + h[i + 1][j] + h[i][j - 1] + h[i][j + 1]);
    }

    for(idx = thread_idx; idx < fire_size; idx += num_threads)
    {
      new_h[fire[idx].first][fire[idx].second] = 1.0;
    }

    pthread_exit(NULL);
  }
  
  // override the update function
  void update_state() override
  {
    for(int i = 0; i < num_threads; i++)
      pthread_create(workers + i, NULL, pthread_helper, args + i);

    for(int i = 0; i < num_threads; i++)
      pthread_join(workers[i], NULL);
  }

  void switch_buffer() override
  {
    std::swap(this->heat, this->swp_heat);
    std::swap(this->data, this->swp_data);
  }
};

int n_iter, n_row, n_col, n_thds;

int main(int argc, char **argv)
{
  if(argc == 4)
  {
    n_row = n_col = atoi(argv[1]);
    n_iter = atoi(argv[2]);
    n_thds = atoi(argv[3]);
  }else if(argc == 5)
  {
    n_row = atoi(argv[1]);
    n_col = atoi(argv[2]);
    n_iter = atoi(argv[3]);
    n_thds = atoi(argv[4]);
  }else{
    n_col = n_row = 800;
    n_iter = 100;
    n_thds = 4;
  }

  gui_init(&argc, argv, "Pthread", 800, 800);
  pthread_simulation heat_state(n_row, n_col, n_thds);
  
  heat_state.init_temperature(FireType::single_fire_square);

  for(int i = 0; i < n_iter; i++)
  {
    heat_state.step();
    visualize(&heat_state, 800, 800);
  }

  print_info("Pthread");
  
  return 0;
}
