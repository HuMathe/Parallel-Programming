#ifndef SIGN_H_ 
#define SIGN_H_ 

#include <cstdio>


inline void print_info(const char *impl_name, int use_quad_tree) {
  printf("Student ID: 120090562\n");
  printf("Name: Derong Jin\n");
  printf("Project 3: N-body simulation %s Implementaion (%s)\n", 
  impl_name, use_quad_tree ? "Barnes-Hut Simulation" : "Direct Simulation");
}

#endif