#ifndef GUI_H
#define GUI_H


#include "physics.h"


#ifdef GUI

#include "GL/glut.h"
#include <string>
#include <iostream>


void draw_recurrsive(qtree_node const *node) {
  if(node == nullptr) return ;
  glVertex2d(node->bb.lu.x, node->bb.lu.y);
  glVertex2d(node->bb.lu.x, node->bb.rd.y);

  glVertex2d(node->bb.lu.x, node->bb.rd.y);
  glVertex2d(node->bb.rd.x, node->bb.rd.y);
  
  glVertex2d(node->bb.rd.x, node->bb.rd.y);
  glVertex2d(node->bb.rd.x, node->bb.lu.y);
  
  glVertex2d(node->bb.rd.x, node->bb.lu.y);
  glVertex2d(node->bb.lu.x, node->bb.lu.y);
  draw_recurrsive(node->ch[0]);
  draw_recurrsive(node->ch[1]);
  draw_recurrsive(node->ch[2]);
  draw_recurrsive(node->ch[3]);

}


void gui_init(int *argc, char **argv, const char *name, int bound_x, int bound_y) {
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(name);
	glClearColor(0, 0, 0, 1);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, bound_x, 0, bound_y);
}

void visualize(const std::vector<particle2d> &bodys, const qtree_node *node = nullptr) {
  std::vector<particle2d>::const_iterator i;
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(1.0f, 1.0f, 0.0f);
  glPointSize(2.0f);
  glBegin(GL_POINTS);
    for( i= bodys.begin(); i != bodys.end(); i++) {
      glVertex2d(i->x.x, i->x.y);
    }
  glEnd();
  glFlush();
  if(false)
  {
    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(0.1f);
    glBegin(GL_LINES);
      draw_recurrsive(node);
    glEnd();
    glFlush();
  }
  
  glutSwapBuffers();
  
}




#else

void gui_init(int *argc, char **argv, const char *name, int bound_x, int bound_y) 
{
  // nothing
}

void visualize(const std::vector<particle2d> &bodys, const qtree_node *node = nullptr) 
{
  // nothing
}

#endif



#endif