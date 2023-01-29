#include "gui.h"
#include "vec2d.h"
#include "physics.h"
#include "signature.h"

#include <ctime>
#include <cstdlib>
#include <iostream>
#include <pthread.h>

class pthread_simulation
  : public gravity_system2d {
  public:
    int n_thds;
    pthread_t *workers;
    struct thread_arg {
      pthread_simulation * sys_ptr;
      int tid;
    } *work_args;

    // init:
    pthread_simulation(int l, int r, int u, int d, int n_thrds, int use_qtree)
      : gravity_system2d(l, r, u, d, use_qtree), n_thds(n_thrds) { 
      workers = new pthread_t [n_thds];
      work_args = new thread_arg [n_thds];
    }

    ~pthread_simulation() {
      delete[] workers;
      delete[] work_args;
    }

    static void *thread_work(void * targs) {
      thread_arg *args = (thread_arg *) targs;
      
      int num = args->sys_ptr->bodys.size();
      int n_workers = args->sys_ptr->n_thds;
      for(int i = args->tid; i < num; i += n_workers) {
        vec2d force = args->sys_ptr->gravitation(args->sys_ptr->bodys[i]);
        args->sys_ptr->bodys[i].apply_constant_force(force);
      }
      
      
      pthread_exit(NULL);
    }

    void simulation_step() override {
      for(int i = 0; i < n_thds; i++) {
        work_args[i] = thread_arg{ this, i };
        pthread_create(workers + i, NULL, 
          pthread_simulation::thread_work, work_args + i);
      }
      for(int i = 0; i < n_thds; i++) {
        pthread_join(workers[i], NULL);
      }
    }
};

int main(int argc, char **argv) {
  srand(time(NULL));
  int N(200), n_iter(1000), nthd(1);
  if(argc != 1) {
    N = atoi(argv[1]), n_iter = atoi(argv[2]), nthd = atoi(argv[3]);
  }
  
  gui_init(&argc, argv, "pthread", 4000, 4000);
  
  pthread_simulation s(0, 4000, 0, 4000, nthd, argc == 5);
  
  for(int i = 0; i < N; i++) {
    s.bodys.push_back(particle2d::random(0, 4000, 0, 4000, 1000));
  }
  
  for(int it = 0; it < n_iter; it++) {
    s.step();
    visualize(s.bodys, s.tree.root);
  }

  print_info("Pthread", s.use_quad_tree);
  
  return 0;
}