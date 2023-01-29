#include "heat_system.h"
#include "signature.h"
#include "gui.h"

#include <iostream>

class seq_simulation
  : public grid_system
{
public:
  
  seq_simulation(): grid_system() { }
  seq_simulation(int n_row, int n_col): grid_system(n_row, n_col) { }

  void update_state() override
  {
    for(int i = 1; i < num_row - 1; i++)
    {
      for(int j = 1; j < num_col - 1; j++)
      {
        swp_heat[i][j] = 0.25 * (heat[i][j - 1] + heat[i][j + 1] + heat[i - 1][j] + heat[i + 1][j]);
      }
    }
  }
};

int num_iter, num_row, num_col;

int main(int argc, char **argv) 
{
  if(argc == 3)
  {
    num_row = num_col = atoi(argv[1]);
    num_iter = atoi(argv[2]);
  }else if(argc == 4)
  {
    num_row = atoi(argv[1]);
    num_col = atoi(argv[2]);
    num_iter = atoi(argv[3]);
  }else{
    num_col = num_row = 800;
    num_iter = 100;
  }

  gui_init(&argc, argv, "Sequential", 800, 800);
  seq_simulation heat_state(num_row, num_col);

  heat_state.init_temperature(FireType::single_fire_square);

  for(int i = 0; i < num_iter; i++)
  {
    heat_state.step();
    visualize(&heat_state, 800, 800);
  }

  print_info("Sequential");
  return 0;
}