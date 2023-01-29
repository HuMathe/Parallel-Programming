#include "heat_system.h"
#include "signature.h"
#include "gui.h"

#include <omp.h>
#include <iostream>

class omp_simulation
  : public grid_system
{
public:
  int num_threads;

public:
  omp_simulation(): grid_system(), num_threads(0) { }
  omp_simulation(int n_row, int n_col, int n_thds)
    : grid_system(n_row, n_col), num_threads(n_thds) { }

  void update_state() override
  {
    int fire_size = fire.size(), i, j, idx;
    std::pair<int, int> * fire_data = fire.data();
    
    #pragma omp parallel num_threads(this->num_threads) default(shared) private(i, j, idx)
    {
      for(idx = omp_get_thread_num(); idx < size; idx += num_threads)
      {
        i = idx / num_col, j = idx % num_col;
        if(i == 0 || j == 0 || i == num_row - 1 || j == num_col - 1)
          swp_heat[i][j] = 0.2;
        else
          swp_heat[i][j] = 0.25 * (heat[i][j - 1] + heat[i][j + 1] + heat[i - 1][j] + heat[i + 1][j]);
      }

      for(idx = omp_get_thread_num(); idx < fire_size; idx += num_threads)
      {
        swp_heat[fire_data[idx].first][fire_data[idx].second] = 1.0;
      }
    }
  }

  void switch_buffer() override
  {
    std::swap(this->heat, this->swp_heat);
    std::swap(this->data, this->swp_data);
  }
};

int n_iter, n_row, n_col, n_thds;

int main(int argc, char ** argv)
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

  gui_init(&argc, argv, "OpenMP", 800, 800);
  omp_simulation heat_state(n_row, n_col, n_thds);

  heat_state.init_temperature(FireType::single_fire_square);

  for(int i = 0; i < n_iter; i++)
  {
    heat_state.step();
    visualize(&heat_state, 800, 800);
  }
  
  print_info("OpenMP");

  return 0;
}