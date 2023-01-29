#ifndef GUI_H
#define GUI_H


#include "heat_system.h"


#ifdef GUI

#include "GL/glut.h"
#include <iostream>



inline void gui_init(int *argc, char **argv, const char *name, int height, int width) {
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(name);
	glClearColor(0, 0, 0, 1);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, width, 0, height);
}

inline void visualize(grid_system *gs, int height, int width) {
  float *pixel_buffer = gs->export_glpixels(width, height);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawPixels(width, height, GL_RGB, GL_FLOAT, pixel_buffer);
  glFlush();
  glutSwapBuffers();
}




#else

inline void gui_init(int *argc, char **argv, const char *name, int height, int width) {
  // do nothing...
}

inline void visualize(grid_system *gs, int height, int width) {
 // do nothing...
}

#endif



#endif