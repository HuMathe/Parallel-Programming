#include "gui.h"
#include "vec2d.h"
#include "physics.h"
#include "signature.h"

#include <ctime>
#include <chrono>
#include <cstdlib>
#include <iostream>


class seq_simulation 
  : public gravity_system2d {
public:
  
  // initialization with boundary sizes
  seq_simulation(int l ,int r, int u, int d, int use_qtree=0)
    : gravity_system2d(l, r, u, d, use_qtree) { }
  

  void simulation_step() override {
    for(int i = 0; i < this->bodys.size(); i++) {
      vec2d force = gravitation(bodys[i]);
      bodys[i].apply_constant_force(force);
    }
  }
};


int main(int argc, char **argv) {
  
  // initialization
  srand(time(NULL));
  int N(200), n_iter(1000);
  if(argc != 1) {
    N = atoi(argv[1]), n_iter = atoi(argv[2]);
  }
  gui_init(&argc, argv, "sequential", 4000, 4000);
  seq_simulation s(0, 4000, 0, 4000, argc==4);
  
  
  for(int i = 0; i < N; i++) {
    s.bodys.push_back(particle2d::random(0, 4000, 0, 4000, 1000));
  }

  // simulate for `n_iter` steps
  for(int it = 0; it < n_iter; it++) {
    s.step();
    visualize(s.bodys);
  }

  print_info("Sequential", s.use_quad_tree);
  return 0;
}