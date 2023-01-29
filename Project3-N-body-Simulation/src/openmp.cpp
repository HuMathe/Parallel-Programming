#include <omp.h>
#include <cstdlib>
#include <iostream>

#include "signature.h"
#include "physics.h"
#include "vec2d.h"
#include "gui.h"

class omp_simulation
  : public gravity_system2d {
public:
  int nthds;
  omp_simulation(int l, int r, int u, int d, int n_thds, int use_qtree=0)
    : gravity_system2d(l, r, u, d, use_qtree), nthds(n_thds) {
    
  }
  void simulation_step() override {
    int i, nt, j;
    vec2d force;
    #pragma omp parallel num_threads(nthds) default(shared) private(i, j, force)
    {
      nt = omp_get_num_threads();
      for(i = omp_get_thread_num(); i < bodys.size(); i += nt) {
        force = gravitation(bodys[i]);
        bodys[i].apply_constant_force(force);
      }
    }
  }
};


int main(int argc, char **argv) {
  srand(time(NULL));
  int N(200), n_iter(1000), n_thds(1);
  if(argc != 1) {
    N = atoi(argv[1]), n_iter = atoi(argv[2]), n_thds = atoi(argv[3]);
  }
  
  gui_init(&argc, argv, "OpenMP", 4000, 4000);
  omp_simulation s(0, 4000, 0, 4000, n_thds, argc==5);

  for(int i = 0; i < N; i++)
    s.bodys.push_back(particle2d::random(0, 4000, 0, 4000, 1000));

  for(int it = 0; it < n_iter; it++) {

    s.step();
    visualize(s.bodys, s.tree.root);

  }

  print_info("OpenMP", s.use_quad_tree);
  
  return 0;
}