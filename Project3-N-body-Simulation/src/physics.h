#ifndef N_BODY_PHYSICS_H
#define N_BODY_PHYSICS_H

#include "vec2d.h"

#include <vector>
#include <chrono>
#include <iostream>

typedef vec2d force2d;

struct particle2d_stat_t {
  double m, dx, dy, vx, vy;
};

class particle2d {
  public:
    double m;
    vec2d x, v, a, a0 = 0, dv, f;

    particle2d() {}
    particle2d(double _x):
      x(_x), v(_x), a(_x), m(_x) {}
    particle2d(
      const vec2d &x, const vec2d &v, const vec2d &a, double m):
      x(x), v(v), a(a), m(m) { }
     
    double &mass() { return m; }
    vec2d &position() { return x; }
    vec2d &velocity() { return v; }
    vec2d &acceleration() { return a; }

    void apply_constant_force(vec2d force);
    static particle2d random(int l, int r, int u, int d, int max_mass = 100);
    void pin() { fixed = true; }
    void unpin() { fixed = false; }
    bool pinned() { return fixed; }
    void emplace_from(const particle2d_stat_t &stat) {
      x = vec2d(stat.dx, stat.dy);
      v = vec2d(stat.vx, stat.vy);
      m = stat.m;
    }
    particle2d_stat_t stat() const { 
      return particle2d_stat_t{ m, x.x, x.y, v.x, v.y}; 
    }

    inline particle2d operator + (const particle2d & rhs) const {
      double tot_mass = m + rhs.m;
      return particle2d(
        (x * m + rhs.x * rhs.m) / tot_mass,
        (v * m + rhs.v * rhs.m) / tot_mass,
        (a * m + rhs.a * rhs.m) / tot_mass,
        tot_mass
      );
    }
  private:
    bool fixed = false;
};

class bbox {
public:
  vec2d lu, rd;
  int size;
  inline vec2d range() const { return rd - lu; }
  inline vec2d median() const { return (lu + rd) / 2; }
  void add(vec2d x) { 
    if(size++ == 0) lu = rd = x; 
    else { 
      lu.x = std::min(x.x, lu.x);
      lu.y = std::min(x.y, lu.y);
      rd.x = std::max(x.x, rd.x);
      rd.y = std::max(x.y, rd.y);
    } 
  }
};


struct quad_tree_node_stat {
  int ch[4];
  double vx, vy, mx, my, m, lx, rx, uy, dy;
};

class qtree_node {
typedef qtree_node* child_ptr;
public:
  child_ptr ch[4] = {nullptr, nullptr, nullptr, nullptr};
  particle2d summary;
  bbox bb;

  qtree_node() { summary = 0; bb.size = 0;}
  quad_tree_node_stat dump();
  int whereis(const vec2d &p) const { 
    vec2d dif = p - bb.median();
    return dif.x < 0 
      ? (dif.y < 0 ? 0 : 2) 
      : (dif.y < 0 ? 1 : 3);
  }
  child_ptr &lu_child() { return ch[0]; }
  child_ptr &ru_child() { return ch[1]; }
  child_ptr &ld_child() { return ch[2]; }
  child_ptr &rd_child() { return ch[3]; }
};

class quad_tree {
public:
  qtree_node* root;
  quad_tree() : root(nullptr) { }
  quad_tree(particle2d *data, int N) { build_tree(data, N); }
  ~quad_tree();
  void build_tree(particle2d *data, int N);
  void destroy_tree(qtree_node *node);
  int size() { return cnt; }
  void serialize(quad_tree_node_stat **tree_array, int &len);
private:
  int cnt = 0;
  qtree_node* build_range(
    particle2d *first, particle2d *last, double eps = 1e-7
    ); // recursion function
};

struct universal_const {
  double delta_t, eps, G, min_r;
};

class gravity_system2d {
  public:
    
    double delta_t = 1e-4; // 1e-3
    double eps = 1e-7;
    double G = 2e4; // 2e5
    double min_r = 10;
    int bounded = 1;
    int collision_chk = 1;
       
    int use_quad_tree;
    int bleft, bright, bup, bdown;

    quad_tree tree;
    std::vector<particle2d> bodys;

    double runtime=0.0;
    

    gravity_system2d()
      : bleft(0), bright(1000), bup(0), bdown(1000), use_quad_tree(0), tree() { }
    gravity_system2d(int l, int r, int u, int d, int uqt)
      : bleft(l), bright(r), bup(u), bdown(d), use_quad_tree(uqt), tree() { }
    ~gravity_system2d();
    void init(int N);
    void __step(particle2d &p);
    universal_const export_env() const { return  universal_const{ delta_t, eps, G, min_r };}
    virtual void simulation_step() = 0;
    vec2d gravitation(particle2d &, particle2d &);
    vec2d gravitation(particle2d &);
    void update_qtree();
    void step();
    void set_slave() { silent = 1; }
    void set_master() { silent = 0; }
  private:
    int silent=0;
};


#endif