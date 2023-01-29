#include "heat_system.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

grid_system::grid_system()
{
  num_col = num_row = size = mute = 0;
  heat = nullptr;
  swp_heat = nullptr;
  color_buffer = nullptr;
  data = nullptr;
  swp_data = nullptr;
}

grid_system::grid_system(int num_row, int num_col)
{
  this->mute = 0;
  this->num_col = num_col;
  this->num_row = num_row;
  this->size = this->num_col * this->num_row;
  this->color_buffer = nullptr;

  this->heat = new double* [this->num_row];
  this->swp_heat = new double* [this->num_row];
  this->data = new double[this->size];
  this->swp_data = new double[this->size];

  for(int i = 0; i < num_row; i++)
  {
    this->heat[i] = this->data + i * this->num_col;
    this->swp_heat[i] = this->swp_data + i * this->num_col;
  }
}

grid_system::~grid_system()
{
  if(this->color_buffer != nullptr)
    delete[] this->color_buffer;
  
  if(this->heat != nullptr)
    delete[] this->heat;
    
  if(this->swp_heat != nullptr)
    delete[] this->swp_heat;

  if(this->data != nullptr)
    delete[] this->data;
  
  if(this->swp_data != nullptr)
    delete[] this->swp_data;
    
}

void grid_system::step()
{
  time_point t_start, t_end;
  t_start = std::chrono::system_clock::now();

  this->update_state();
  this->switch_buffer();


  t_end = std::chrono::system_clock::now();

  std::chrono::duration<double> e_sec = t_end - t_start;
  
  if(! this->mute)
    std::cout << "[step runtime]: " << e_sec.count() << " sec" << std::endl;
}

float *grid_system::export_glpixels(int display_w, int display_h)
{
  if(color_buffer != nullptr)
    delete[] color_buffer;
  
  color_buffer = new float [display_w * display_h * 3];

  sync_state(); // synchronize the heat value

  int n_pixels = display_w * display_h;
  float rw = 1.0 * num_col / display_w, rh = 1.0 * num_row / display_h, tmp;
  for(int i = 0; i < n_pixels; i++)
  {
    tmp = (float)(heat[int(rh*(i / display_h))][int(rw*(i % display_w))]);
    
    // update the rgb value of the buffer
    evaluate_rgb(color_buffer + i * 3, (tmp - 0.2) / 0.8);
  }

  return color_buffer;
}

void grid_system::sample_fire(FireType type)
{
  int i = std::rand() % (num_row - 1) + 1;
  int j = std::rand() % (num_col - 1) + 1;
  int len = std::min(num_col, num_row);

  int r = std::rand() % int(0.2 * len) + 0.05 * len;
  r = std::min(r, std::min(std::min(j, num_col - j - 1) - 1, std::min(i, num_row - i - 1) - 1));

  for(int _i = i - r; _i <= i + r; _i++)
  {
    for(int _j = j - r; _j <= j + r; _j++)
    {
      if(type == FireType::single_fire_circular 
        && (_j - j) * (_j - j) + (_i  - i) * (_i - i) > r * r)
          continue;
      fire.emplace_back(_i, _j);
      heat[_i][_j] = 1.0;
    }
  }
}

void grid_system::init_temperature(FireType type)
{
  for(int i = 0; i < num_row; i++)
  {
    for(int j = 0; j < num_col; j++)
    {
      heat[i][j] = 0.2;
    }
  }
  
  sample_fire(type);

  wall_is_cold();
  fire_is_hot();

  init_shared_memory();
}

void grid_system::wall_is_cold()
{
  for(int i = 0; i < num_row; i++)
    swp_heat[i][0] = swp_heat[i][num_col - 1] = .2;
  
  for(int j = 0; j < num_col; j++)
    swp_heat[0][j] = swp_heat[num_row - 1][j] = .2;
}

void grid_system::fire_is_hot()
{
  for(index_iterator it = fire.begin(); it != fire.end(); it++)
  {
    swp_heat[it->first][it->second] = 1.0;
  }
}

// this palette is credited to: https://stackoverflow.com/questions/12875486/what-is-the-algorithm-to-create-colors-for-a-heatmap
void evaluate_rgb(float *rgb, float x)
{
  x = std::round((1 - x) * 12) / 12;
  float h = x * 4;
  x = (1 - std::abs(std::fmod(h, 2) - 1));
  if(h <= 1) 
    rgb[0] = 1, rgb[1] = x, rgb[2] = 0;
  else if(h <= 2)
    rgb[0] = x, rgb[1] = 1, rgb[2] = 0;
  else if(h <= 3)
    rgb[0] = 0, rgb[1] = 1, rgb[2] = x;
  else if(h <= 4)
    rgb[0] = 0, rgb[1] = x, rgb[2] = 1;
  else if(h <= 5)
    rgb[0] = x, rgb[1] = 0, rgb[2] = 1;
  else
    rgb[0] = 1, rgb[1] = 0, rgb[2] = x;
}


void evaluate_rgb_1(float *rgb, float x)
{
  int r[] = {0x99, 0x9c, 0xa2, 0xaa, 0xb8, 0xcf, 0xfa, 0xfe, 0xfd, 0xfb, 0xf9, 0xf9, 0xf8};
  int g[] = {0xb3, 0xb5, 0xb9, 0xbf, 0xca, 0xd9, 0xf6, 0xec, 0xe1, 0xc3, 0x7f, 0x5e, 0x20};
  int b[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xc4, 0x84, 0x0e, 0x0f, 0x22};
  int i = std::round(x * 12);
  rgb[0] = 1.0 * r[i] / 0xff;
  rgb[1] = 1.0 * g[i] / 0xff;
  rgb[2] = 1.0 * b[i] / 0xff;
}

void grid_system::sync_state()
{
  // do nothing. (override by child class if needed)
}

void grid_system::switch_buffer()
{
  fire_is_hot();
  wall_is_cold();

  std::swap(this->heat, this->swp_heat);
  std::swap(this->data, this->swp_data);
}

void grid_system::init_shared_memory()
{
  // do nothing (override by child class if needed)
}