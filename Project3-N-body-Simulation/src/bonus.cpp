#include "gui.h"
#include "vec2d.h"
#include "physics.h"
#include "signature.h"

#include "mpi.h"
#include <omp.h>
#include <ctime>
#include <cstddef>
#include <cstdlib>
#include <iostream>

class mpi_direction_simulation
  : public gravity_system2d {
public:
  int rank, world_size, block_size;
  double *forcesx = NULL, *forcesy = NULL, *recv_fx = NULL, *recv_fy = NULL;
  particle2d_stat_t * buffer = NULL, * recv_buffer = NULL;
  MPI_Datatype MPI_PARTICLE2D_STAT;
  int nthds = 4;

  // initialization with boundary sizes
  mpi_direction_simulation(int l, int r, int u, int d, int use_qtree=0) 
    : gravity_system2d(l, r, u, d, use_qtree) {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    block_size = 0;


    int stat_blocklen[5] = {1, 1, 1, 1, 1};
    MPI_Datatype stat_types[5] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE};
    MPI_Aint stat_offsets[5] = {
      offsetof(particle2d_stat_t, m), 
      offsetof(particle2d_stat_t, dx), offsetof(particle2d_stat_t, dy), 
      offsetof(particle2d_stat_t, vx), offsetof(particle2d_stat_t, vy) };
    MPI_Type_create_struct(5, stat_blocklen, stat_offsets, stat_types, &MPI_PARTICLE2D_STAT);
    MPI_Type_commit(&MPI_PARTICLE2D_STAT);
    buffer = NULL;
    recv_buffer = NULL;
  }

  ~mpi_direction_simulation() {
    if(buffer != NULL)  
      delete[] buffer;
    if(recv_buffer != NULL)
      delete[] recv_buffer;
    if(forcesx != NULL) delete[] forcesx;
    if(forcesy != NULL) delete[] forcesy;
    if(recv_fx != NULL) delete[] recv_fx;
    if(recv_fy != NULL) delete[] recv_fy;
  }

  void start() {
    uint block_size = (bodys.size() - 1) / world_size + 1;
    forcesx = new double[block_size * world_size];
    forcesy = new double[block_size * world_size];
    
    buffer = new particle2d_stat_t[block_size * world_size];
    if(rank == 0) {
      recv_fx = new double[block_size * world_size];
      recv_fy = new double[block_size * world_size];
      recv_buffer = new particle2d_stat_t[block_size * world_size];
    }
  }

  void simulation_step() override {
    

    uint block_size = (bodys.size() - 1) / world_size + 1;
    if(rank == 0) {
      for(uint i = 0; i < bodys.size(); i++)
        buffer[i] = bodys[i].stat();
    }
    
    MPI_Bcast(buffer, bodys.size(), MPI_PARTICLE2D_STAT, 0, MPI_COMM_WORLD);
    
    for(uint i = 0; i < bodys.size(); i++)
      bodys[i].emplace_from(buffer[i]);
    
    uint end_pos = bodys.size() < (rank + 1) * block_size 
      ? bodys.size() : (rank + 1) * block_size;
    
    uint i, nt;
    vec2d force;
    
    #pragma omp parallel num_threads(this->nthds) default(shared) private(i, force)
    {
      nt = omp_get_num_threads();
      for(i = rank * block_size + omp_get_thread_num(); i < end_pos; i+=nt) {
        vec2d force = gravitation(bodys[i]);
        forcesx[i] = force.x;
        forcesy[i] = force.y;
        buffer[i] = bodys[i].stat();
      }
    }
    

    MPI_Gather(buffer + rank * block_size, block_size, MPI_PARTICLE2D_STAT, 
      recv_buffer, block_size, MPI_PARTICLE2D_STAT, 0, MPI_COMM_WORLD);
    MPI_Gather(forcesx + rank * block_size, block_size, MPI_DOUBLE, 
      recv_fx, block_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(forcesy + rank * block_size, block_size, MPI_DOUBLE, 
      recv_fy, block_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    if(rank == 0) {
      for(uint i = 0; i < bodys.size(); i++) {
        bodys[i].emplace_from(recv_buffer[i]);
        bodys[i].apply_constant_force(vec2d(recv_fx[i], recv_fy[i]));
      }
    }
  }
};

int main(int argc, char **argv) {
  srand(time(NULL));
  int N(200), n_iter(1000);
  if(argc != 1) {
    N = atoi(argv[1]), n_iter = atoi(argv[2]);
  }
  
  MPI_Init(&argc , &argv);
  mpi_direction_simulation s(0, 4000, 0, 4000, argc == 4);
  if(s.rank == 0) 
    gui_init(&argc, argv, "MPI + OpenMP", 4000, 4000);
  else
    s.set_slave();
  
  // s.bodys.emplace_back(2000, 0, 0, 1e9);
  for(int i = 0; i < N; i++) 
    s.bodys.push_back(particle2d::random(0, 4000, 0, 4000, 1000));

  s.start();
  for(int it = 0; it < n_iter; it++) {
    s.step();
    
    if(s.rank == 0)
      visualize(s.bodys);

  }

  if(s.rank == 0) print_info("MPI + OpenMP", s.use_quad_tree);

  MPI_Finalize();
  return 0;
}