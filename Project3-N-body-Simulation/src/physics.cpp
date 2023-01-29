#include "physics.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

void particle2d::apply_constant_force(force2d f) {
  this->f = f;
  return ;
}

gravity_system2d::~gravity_system2d() {
  
}

void gravity_system2d::__step(particle2d &p) {
  if(p.pinned()) return ;
  double t = this->delta_t;
  p.acceleration() = p.f / p.m;
  p.dv = 0.5 * (p.a0 + p.a);
  p.v += p.dv * t;
  // if(p.v.norm() > velocity_damp) p.v *= 0.9;
  p.x += p.v * t + 0.5 * p.a * t * t;
  p.a0 = p.a;
  p.a = 0;

  if(bounded == 0)
    return ;

  while(p.x.x < bleft || p.x.x > bright) {
    if(p.x.x < bleft) {
      p.v.x = -p.v.x;
      p.x.x = 2.0 * bleft - p.x.x;
    } else {
      p.v.x = -p.v.x;
      p.x.x = 2.0 * bright - p.x.x;
    }
  }

  while(p.x.y < bup || p.x.y > bdown) {
    if(p.x.y < bup) {
      p.v.y *= -1;
      p.x.y = 2.0 * bup - p.x.y;
    } else {
      p.v.y *= -1.0;
      p.x.y = 2.0 * bdown - p.x.y;
    }
  }
  return ;
}

vec2d gravity_system2d::gravitation(
  particle2d &p0, particle2d &p1) {
  vec2d d = p1.x - p0.x;
  vec2d force = d.unit() * this->G * p0.m * p1.m / (d.norm2() + this->eps);
  if(p0.v.norm() > 5e4) p0.v = p0.v.unit() * 3e4;
  if(d.norm() < min_r && this->collision_chk) {
    p0.v -= 2.0 * p1.m / (p1.m + p0.m) * dot(p0.v - p1.v, p0.x - p1.x) / (d.norm2() + this->eps) * (p0.x - p1.x);
    return vec2d(0);
  }
  return force;
}

vec2d gravity_system2d::gravitation(particle2d &p0) {
  vec2d force = 0;
  if(use_quad_tree) {
    qtree_node *node = tree.root, *next_node;
    
    while(node != nullptr) {
      for(int c = 0; c < 4; c++) {
        if(c == node->whereis(p0.x)) {
          next_node = node->ch[c];
          if(next_node == nullptr)
            force += gravitation(p0, node->summary);
        } else if(node->ch[c] != nullptr){
          force += gravitation(p0, node->ch[c]->summary);
        } 
      }
      node = next_node;
    }
  } else {
    for(int i = 0; i < bodys.size(); i++) {
      if(&bodys[i] == &p0) continue;
      force += gravitation(p0, bodys[i]);
    }
  }
  return force;
}

particle2d particle2d::random(int l, int r, int u, int d, int max_mass) {

  // return particle2d(vec2d(1500.0 + 1.0 * (rand() % 500000) / 50000, 
  //   2000 + 5.0 * (rand() % 50000) / 5000), vec2d(0, 8e4), 0, 10);

  // particle2d ans;
  // if(rand() % 5000 < 50) {
  //   ans.x = vec2d(rand()%((r-l) / 27) + l + (r-l) * 13 /27, rand()%((d-u) / 27) + u + (d - u) / 27 * 13);
  //   ans.m = 100;
  //   ans.v =  vec2d(ans.x.y - u - (d - u) / 2, -ans.x.x + l + (r - l) / 2) * 1000;
  //   ans.a = 0;
  // } else {
  //   ans.x = vec2d(rand()%((r-l) / 19) + l + (r-l)/19 * 9, rand()%((d-u) / 5) + u + (d - u) / 5 * 2);
  //   ans.m = 30;
  //   ans.v =  vec2d(ans.x.y - u - (d - u) / 2, -ans.x.x + l + (r - l) / 2);
  //   // ans.v /= ans.v.norm();
  //   ans.v *= 250;
  // }
  // return ans;
  
  double x = rand()%((r-l) / 3) + l + (r-l)/3, y = rand()%((d-u) / 3) + u + (d - u) / 3;
  return particle2d(
    vec2d(x, y),
    //vec2d(rand()%((r-l)) + l , rand()%((d-u)) + u),
    // 100 * vec2d(y - u - (d - u) / 2, -x + l + (r - l) / 2),
    0,
    /*vec2d(0),*/ vec2d(0), rand()%(max_mass / 2) + max_mass / 2);
}

void gravity_system2d::update_qtree() {
  particle2d *temp = new particle2d[bodys.size()];
  memcpy(temp, bodys.data(), sizeof(particle2d) * bodys.size());
  tree.build_tree(temp, bodys.size());
  delete[] temp;
}


quad_tree::~quad_tree() {
  destroy_tree(root);
}

void quad_tree::build_tree(particle2d *data, int N) {
  destroy_tree(root);
  root = build_range(data, data + N);
}


void quad_tree::destroy_tree(qtree_node *node) {
  if(node == nullptr) return ;
  for(int i = 0; i < 4; i++)
    destroy_tree(node->ch[i]);
  cnt -- ;
  delete node;
}

qtree_node*quad_tree::build_range(
  particle2d *first, particle2d *last, double eps) {
  if(first >= last) return nullptr;

  cnt ++;
  qtree_node *node = new qtree_node();
  for(particle2d *it = first; it != last; it++) {
    node->bb.add(it->x);
    node->summary = node->summary + *it;
  }
  vec2d median = node->bb.median();
  vec2d range = node->bb.range();
  
  if(node->bb.size == 1 || std::max(range.x, range.y) < eps) 
    return node;
  
  particle2d *uplast = first, *leftlast = first;
  for(particle2d *it = first; it != last; it++) {
    if(it->x.y < median.y) {
      std::iter_swap(it, uplast);
      uplast++;
    }
  }
  
  for(particle2d *it = first; it != uplast; it++) {
    if(it->x.x < median.x) {
      std::iter_swap(it, leftlast);
      leftlast++;
    }
  }
  
  // build quadtree for upper half
  node->lu_child() = build_range(first, leftlast);
  node->ru_child() = build_range(leftlast, uplast);
  
  leftlast = uplast;
  for(particle2d *it = uplast; it != last; it++) {
    if(it->x.x < median.x) {
      std::iter_swap(it, leftlast);
      leftlast++;
    }
  }

  // build qtree for lower half
  node->ld_child() = build_range(uplast, leftlast);
  node->rd_child() = build_range(leftlast, last);

  return node;
}

void rec_dump(qtree_node *node, quad_tree_node_stat *tarr, int &now) {
  int i = now++;
  tarr[i] = node->dump();
  for(int c = 0; c < 4; c++) {
    if(node->ch[c] == nullptr) tarr[i].ch[c] = -1;
    else rec_dump(node->ch[c], tarr, now);
  }
}

void quad_tree::serialize(quad_tree_node_stat **tree_array, int &len) {
  int now = 0;
  len = this->size();
  *tree_array = new quad_tree_node_stat[len];
  rec_dump(root, *tree_array, now);
}


quad_tree_node_stat qtree_node::dump() { 
  quad_tree_node_stat ans;
  ans.ch[0] = ans.ch[1] = ans.ch[2] = ans.ch[3] = -1;
  ans.lx = this->bb.lu.x; ans.rx = this->bb.rd.x;
  ans.uy = this->bb.lu.y; ans.dy = this->bb.rd.y;
  ans.vx = summary.v.x; 
  ans.vy = summary.v.y;
  ans.m = this->summary.m;
  ans.mx = summary.x.x; ans.my = summary.x.y;
  return  ans;
}


void gravity_system2d::step() {
  std::chrono::time_point<std::chrono::system_clock> t_start, t_end;
  t_start = std::chrono::system_clock::now();
  if(use_quad_tree) update_qtree();
  this->simulation_step();

  if(silent) return ;

  for(int i = 0; i < this->bodys.size(); i++) {
    __step(bodys[i]);
  }
  
  t_end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_sec = t_end - t_start;
  std::cout << "[step runtime]: " << elapsed_sec.count() << " sec" << std::endl; 
  this->runtime += elapsed_sec.count();
}