#ifndef HEAT_SYS_H_
#define HEAT_SYS_H_

#define MAX_TEMP 1.0
#define MIN_TEMP 0.0

#include <chrono>
#include <vector>

typedef std::vector<std::pair<int, int>>::iterator index_iterator;
typedef std::chrono::time_point<std::chrono::system_clock> time_point;

enum class FireType
{
  no_fire,
  single_fire_square,
  single_fire_circular, // should be specified with radius (rand number)
  mutiple_fire_square,
  mutiple_fire_circular
};

/**
 * abstract class to support heat simulation via different methods
*/
class grid_system {
public:
  int num_row, num_col; // number of rows and columns samples
  int size;             // number of total samples
  
  double **heat;        // heat value on each grid vertex
  double **swp_heat;    // store the next state of simulation
  double *data, *swp_data;

  int mute;             // no msg printing

  float *color_buffer;  // pixel buffer (used to support gui)

  std::vector<std::pair<int, int>> fire;

// public methods
public:
  grid_system();
  grid_system(int num_row, int num_col);
  virtual ~grid_system();

  void sample_fire(FireType type);

  void init_temperature(FireType type);

  // conduct 1 step of simulation, and record the runtime...
  void step();

  // helper function that supports gui
  float *export_glpixels(int display_w, int display_h);

  // calculate the physical state of the next moment, store the ans in swp_heat (to be implemented by child class)
  virtual void update_state() = 0;

  virtual void sync_state();

  virtual void switch_buffer();

  virtual void init_shared_memory();

private:
  // maintain the temperature of the fire and the wall
  void wall_is_cold();
  void fire_is_hot();
};

void evaluate_rgb(float *rgb, float x);


#endif