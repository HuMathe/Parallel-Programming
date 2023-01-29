#include "gui.h"
#include "physics.h"
#include "signature.h"

#include <cuda.h>
#include <iostream>
#include <cuda_runtime.h>

#define whereis(node, x, y) (       \
  (node.lx + node.rx > x + x)       \
    ? (node.uy + node.dy > y + y ? 0 : 2) \
    : (node.uy + node.dy > y + y ? 1 : 3))


// extern __shared__ particle2d_stat_t stars[];

// extern __shared__ double position[];

__device__ void get_g(double x0, double y0, double x1, double y1, double m1, 
  double &ansx, double &ansy, double min_r, double eps) {

  double dx, dy;
  double norm;
  dx = x1 - x0;
  dy = y1 - y0;
  norm = sqrt(dx * dx + dy * dy);
  if(norm > min_r) {
    ansx += dx * m1 / (norm * norm + eps) / norm;
    ansy += dy * m1 / (norm * norm + eps) / norm;
  }
}

__device__ void collision(double x0, double y0, double m0, double &vx0, double &vy0, 
  double x1, double y1, double m1, double vx1, double vy1,
  double min_r, double eps) {

  double dx, dy;
  double norm, coeff;
  dx = x0 - x1;
  dy = y0 - y1;
  norm = sqrt(dx * dx + dy * dy);
  if(vx0 * vx0 + vy0 * vy0 > 25e8) {
    vx0 = vx0 / (sqrt(vx0 * vx0 + vy0 * vy0)) * 3e4;
    vy0 = vy0 / (sqrt(vx0 * vx0 + vy0 * vy0)) * 3e4;
  }
  if(norm <= min_r) {
    coeff = 2.0 * m1 / (m0 + m1) / (norm + eps) / (norm + eps);
    coeff *= (vx0 - vx1) * (x0 - x1) + (vy0 - vy1) * (y0 - y1);
    vx0 -= coeff * (x0 - x1);
    vy0 -= coeff * (y0 - y1);
  }
}

__global__ void direct_simulation_step(particle2d_stat_t *star_stat, 
  vec2d *force, int N, const universal_const C,
  quad_tree_node_stat *ta, int use_qtree) {
  
  int i = blockDim.x * blockIdx.x + threadIdx.x;
  particle2d_stat_t *stars = star_stat;
  
  if(i >= N) return ;

  
  force[i].x = 0;
  force[i].y = 0;
  
  if(use_qtree) {
    int cur = 0, next_cur;
    while(cur != -1) {
      for(int c = 0; c < 4; c++) {
        if(c == whereis(ta[cur], stars[i].dx, stars[i].dy)) {
          next_cur = ta[cur].ch[c];
          if(next_cur == -1) {
            get_g(stars[i].dx, stars[i].dy, 
              ta[cur].mx, ta[cur].my, ta[cur].m, 
              force[i].x, force[i].y, C.min_r, C.eps);

            collision(stars[i].dx, stars[i].dy, stars[i].m, stars[i].vx, stars[i].vy,
              ta[cur].mx, ta[cur].my, ta[cur].m, ta[cur].vx, ta[cur].vy, C.min_r, C.eps);

          }
        } else if(ta[cur].ch[c] != -1) {
          get_g(stars[i].dx, stars[i].dy, 
            ta[ta[cur].ch[c]].mx, ta[ta[cur].ch[c]].my, ta[ta[cur].ch[c]].m,
            force[i].x, force[i].y, C.min_r, C.eps);

          collision(stars[i].dx, stars[i].dy, stars[i].m, stars[i].vx, stars[i].vy,
            ta[ta[cur].ch[c]].mx, ta[ta[cur].ch[c]].my, ta[ta[cur].ch[c]].m,
            ta[ta[cur].ch[c]].vx, ta[ta[cur].ch[c]].vy, C.min_r, C.eps);

        }
      }
      cur = next_cur;
    }
  } else {
    for(int j = 0; j < N; j++) {
      if(j == i) continue;
      get_g(stars[i].dx, stars[i].dy, 
        stars[j].dx, stars[j].dy, stars[j].m,
        force[i].x, force[i].y, C.min_r, C.eps);

      collision(stars[i].dx, stars[i].dy, stars[i].m, stars[i].vx, stars[i].vy,
        stars[j].dx, stars[j].dy, stars[j].m, stars[j].vx, stars[j].vy,
        C.min_r, C.eps);
    }
  }
  force[i].x *= stars[i].m * C.G;
  force[i].y *= stars[i].m * C.G;
}

__host__ void offload2GPU(std::vector<particle2d> &stars, const universal_const C, quad_tree &tree, int use_qtree, int threadsPerBlock = 128) {


    particle2d_stat_t   *particle_stat, *d_particle_stat;
    vec2d               *forces,        *d_forces;
    quad_tree_node_stat *tree_array,    *d_tree_array;
    int tree_size = 0;

    particle_stat = new particle2d_stat_t[stars.size()];
    forces        = new vec2d[stars.size()];
    if(use_qtree) {
      tree.serialize(&tree_array, tree_size);
    }

    size_t mem_size_p = sizeof(particle2d_stat_t)   * stars.size();
    size_t mem_size_f = sizeof(vec2d)               * stars.size();
    size_t mem_size_t = sizeof(quad_tree_node_stat) * tree_size;
    
    
    cudaMalloc(&d_particle_stat, mem_size_p);
    cudaMalloc(&d_forces, mem_size_f);
    if(mem_size_t) cudaMalloc(&d_tree_array, mem_size_t);

    for(int i = 0; i < stars.size(); i++) 
      particle_stat[i] = stars[i].stat();
    
    cudaMemcpy(d_particle_stat, particle_stat, mem_size_p,
      cudaMemcpyHostToDevice);
    cudaMemset(d_forces, 0, mem_size_f);
    if(use_qtree) cudaMemcpy(d_tree_array, tree_array, mem_size_t, 
      cudaMemcpyHostToDevice);
    
    
    // offload calculation to cuda
    direct_simulation_step<<<(stars.size() - 1) / threadsPerBlock + 1, threadsPerBlock>>>(
      d_particle_stat, d_forces, stars.size(), C, d_tree_array, use_qtree);
    cudaDeviceSynchronize();
    

    // load result from cuda
    cudaMemcpy(forces, d_forces, mem_size_f, 
      cudaMemcpyDeviceToHost);
    cudaMemcpy(particle_stat, d_particle_stat, mem_size_p,
      cudaMemcpyDeviceToHost);


    for(int i = 0; i < stars.size(); i++) {
      stars[i].velocity() = vec2d(particle_stat[i].vx, particle_stat[i].vy);
      stars[i].apply_constant_force(forces[i]);
    }

    // release cuda resources
    cudaFree(d_particle_stat);
    cudaFree(d_forces);

    // release memory
    delete[] particle_stat;
    delete[] forces;
    
}


class cuda_direct_simulation
  : public gravity_system2d {
public:
  int n_thrds;
  int blocksPerGrid, threadsPerBlock;

  cuda_direct_simulation(int l, int r, int u, int d, int use_qtree)
    : gravity_system2d(l, r, u, d, use_qtree) {
      blocksPerGrid = (bodys.size() - 1) / 128 + 1, threadsPerBlock = 128;
    }
  
  // custom parallelization
  void simulation_step() override {
    offload2GPU(bodys, this->export_env(), this->tree, use_quad_tree, 32);
  }
  
};

int main(int argc, char **argv) {
  srand(time(NULL));
  int N, n_iter;
  if(argc == 1) {
    N = 200, n_iter = 1000;
  } else {
    N = atoi(argv[1]), n_iter = atoi(argv[2]);
  }
  gui_init(&argc, argv, "cuda", 4000, 4000);
  cuda_direct_simulation s(0, 4000, 0, 4000, argc==4);

  // s.bodys.emplace_back(2000, 0, 0, 1e17);
  for(int i = 0; i < N; i++) {
    s.bodys.push_back(particle2d::random(0, 4000, 0, 4000, 1000));
  }

  for(int it = 0; it < n_iter; it++) {
    s.step();
    visualize(s.bodys);
  }

  print_info("CUDA", s.use_quad_tree);

  return 0;
}