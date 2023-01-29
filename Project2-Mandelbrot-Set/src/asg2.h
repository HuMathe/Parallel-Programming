#ifdef GUI
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <cstdlib>
#include <chrono>
#include <cmath>

typedef struct {
	float r, g, b;
} color3;
color3 black = {0, 0, 0};


/* define a struct called Compl to store information of a complex number*/
typedef struct complextype { float real, imag; } Compl;

/* define a struct called Point to store information of each point */
typedef struct pointtype { int x, y; float color; color3 rgb; } Point;

/*
X_RESN = resolution of x axis
Y_RESN = resolution of y axis
total_size = X_RESN * Y_RESN
max_iteration = a parameter of Mandelbrot computation
*/
int X_RESN, Y_RESN, total_size, max_iteration;

/* to store all the points, it will be initialized later */
Point* data;

/* to keep track of time */
std::chrono::high_resolution_clock::time_point t1;
std::chrono::high_resolution_clock::time_point t2;
std::chrono::duration<double> time_span;


void initData() {
	/*
	Intialize data storage.

	data = 
	|   x1   |   x2   |  ...  |   xn   |
	|   y1   |   y2   |  ...  |   yn   |
	| color1 | color2 |  ...  | colorn |

	it represents a 2D array where each entry has a color attribute.
	
	x_i is in {0, 1, ..., X_RESN)}
	y_i is in {0, 1, ..., Y_RESN)}
	color_i is in {0, 1}
	
	*/

	total_size = X_RESN * Y_RESN;
	data = new Point[total_size];
	int i, j;
	Point* p = data;
	for (i = 0; i < X_RESN; i++) {
        for (j = 0; j < Y_RESN; j++) {
			p->x = i;
			p->y = j;
			p ++;
		}
	}
}

void freeData()
{
	delete[] data;
}

// color function credit: [https://iquilezles.org/articles/palettes/]
// 		c(t) = a + b * \cos(2\pi(c*t + d))
color3 colormap_find(int iter_num)
{
	if(iter_num == max_iteration) {
		return black;
	}
	double t = 1.0 * iter_num / max_iteration + 0.5;
	t -= floor(t);
	return (color3) {
		(float)(0.5 + 0.5 * cos(2.0 * M_PI * (1.0 * t + 0.00))),
		(float)(0.5 + 0.5 * cos(2.0 * M_PI * (1.0 * t + 0.10))),
		(float)(0.5 + 0.5 * cos(2.0 * M_PI * (1.0 * t + 0.20)))
	};
}

void compute(Point* p) {
	/* 
	Give a Point p, compute its color.
	Mandelbrot Set Computation.
	No need to modify this 'atom' function. 
	*/

	Compl z, c;
	float lengthsq, temp;
	int k;

	/* scale [0, X_RESN] x [0, Y_RESN] to [-1, 1] x [-1, 1] */
	c.real = ((float) p->x - X_RESN / 2) / (X_RESN / 2);
	c.imag = ((float) p->y - Y_RESN / 2) / (Y_RESN / 2);
	// c.real = 2 * (c.real) - 0.7;
	// c.imag = 2 * (c.imag);
	/* the following block is about math. */ 

	z.real = z.imag = 0.0;
    k = 0;

	do { 
		temp = z.real*z.real - z.imag*z.imag + c.real;
		z.imag = 2.0*z.real*z.imag + c.imag;
		z.real = temp;
		lengthsq = z.real*z.real+z.imag*z.imag;
		k++;
	} while (lengthsq < 4.0 && k < max_iteration);

	/* math block end */ 

	p->color = (float) k / max_iteration;
	p->rgb = colormap_find(k);
}


#ifdef GUI
void plot() {
	/* Plot all the points to screen. */

	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0f, 0.0f, 0.0f);
	
	GLfloat pointSize = 1.0f;
	glPointSize(pointSize);
	glBegin(GL_POINTS);
		glClear(GL_COLOR_BUFFER_BIT);
		
		int count;
		Point* p = data;
		for (count = 0; count < total_size; count++){
			// glColor3f(1-p->color, 1-p->color, 1-p->color); // control the color
			glColor3f(p->rgb.r, p->rgb.g, p->rgb.b);
			glVertex2f(p->x, p->y); // plot a point
			p ++;
		}
	// glColor3f(1, 0, 0);
	// glVertex2f(X_RESN / 2, Y_RESN / 2);
	glEnd();
	glFlush();
	
}

#endif