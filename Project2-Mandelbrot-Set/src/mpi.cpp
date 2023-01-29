#include "asg2.h"
#include <stdio.h>
#include <mpi.h>

int rank;
int world_size;
float **rgb_buffer_global;
float **rgb_buffer;

void master() {
	
	Point* p = data;
	int i, j, cnt = 0, index, block_size = (total_size - 1) / world_size + 1;

	MPI_Bcast(&Y_RESN, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&max_iteration, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// create local buffer 
	rgb_buffer = new float* [3];
	rgb_buffer[0] = new float[block_size];
	rgb_buffer[1] = new float[block_size];
	rgb_buffer[2] = new float[block_size];

	// create global buffer (for display)
	rgb_buffer_global = new float* [3];
	rgb_buffer_global[0] = new float[block_size * world_size];
	rgb_buffer_global[1] = new float[block_size * world_size];
	rgb_buffer_global[2] = new float[block_size * world_size];
	
	for(index = 0, p = data; index < block_size; ++index, ++p)
		compute(p);

	MPI_Gather(rgb_buffer[0], block_size, MPI_FLOAT, rgb_buffer_global[0], block_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
	MPI_Gather(rgb_buffer[1], block_size, MPI_FLOAT, rgb_buffer_global[1], block_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
	MPI_Gather(rgb_buffer[2], block_size, MPI_FLOAT, rgb_buffer_global[2], block_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
	

	p = data;
	for (i = 0; i < X_RESN; ++i) {
        for (j = 0; j < Y_RESN; ++j, ++cnt, ++p) {
			if(cnt >= block_size) {
				p->rgb = color3{
					.r=rgb_buffer_global[0][cnt], 
					.g=rgb_buffer_global[1][cnt],
					.b=rgb_buffer_global[2][cnt]};
			}
				
		}
	}

	// delete all heap variables
	delete[] rgb_buffer_global[0];
	delete[] rgb_buffer_global[1];
	delete[] rgb_buffer_global[2];
	delete[] rgb_buffer_global;
	delete[] rgb_buffer[0];
	delete[] rgb_buffer[1];
	delete[] rgb_buffer[2];
	delete[] rgb_buffer;
}


void slave() {
	
	Point p;
	int i, index, block_size;
	MPI_Bcast(&Y_RESN, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&max_iteration, 1, MPI_INT, 0, MPI_COMM_WORLD);

	
	block_size = (total_size - 1) / world_size + 1;

	// create local buffer (carries the calculation)
	rgb_buffer = new float* [3];
	rgb_buffer_global = new float* [3]; // to avoid NULL pointer
	rgb_buffer[0] = new float[block_size];
	rgb_buffer[1] = new float[block_size];
	rgb_buffer[2] = new float[block_size];
	for(i = 0, index = rank * block_size; i < block_size; ++i, ++index)
	{
		p = (Point) { .x=index / Y_RESN, .y=index % Y_RESN };
		compute(&p);
		rgb_buffer[0][i] = p.rgb.r;
		rgb_buffer[1][i] = p.rgb.g;
		rgb_buffer[2][i] = p.rgb.b;
	}

	// gather the results
	MPI_Gather(rgb_buffer[0], block_size, MPI_FLOAT, rgb_buffer_global[0], block_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
	MPI_Gather(rgb_buffer[1], block_size, MPI_FLOAT, rgb_buffer_global[1], block_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
	MPI_Gather(rgb_buffer[2], block_size, MPI_FLOAT, rgb_buffer_global[2], block_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

	// destroy heap variables
	delete[] rgb_buffer[0];
	delete[] rgb_buffer[1];
	delete[] rgb_buffer[2];
	delete[] rgb_buffer;
	delete[] rgb_buffer_global;
}


int main(int argc, char *argv[]) {
	if ( argc == 4 ) {
		X_RESN = atoi(argv[1]);
		Y_RESN = atoi(argv[2]);
		max_iteration = atoi(argv[3]);
	} else {
		X_RESN = 1000;
		Y_RESN = 1000;
		max_iteration = 100;
	}

	

	/* computation part begin */
	MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if (rank == 0) {
		t1 = std::chrono::high_resolution_clock::now();

		initData();
		master();

		t2 = std::chrono::high_resolution_clock::now();  
		time_span = t2 - t1;

		printf("Student ID: 120090562\n"); // replace it with your student id
		printf("Name: Derong Jin\n"); // replace it with your name
		printf("Assignment 2 MPI\n");
		printf("Run Time: %f seconds\n", time_span.count());
		printf("Problem Size: %d * %d, %d\n", X_RESN, Y_RESN, max_iteration);
		printf("Process Number: %d\n", world_size);
		
	} else {
		slave();
	}
	
	MPI_Finalize();
	/* computation part end */
	if (rank == 0) {
		#ifdef GUI
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
		glutInitWindowSize(800, 800); 
		glutInitWindowPosition(0, 0);
		glutCreateWindow("MPI");
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glMatrixMode(GL_PROJECTION);
		gluOrtho2D(0, X_RESN, 0, Y_RESN);
		glutDisplayFunc(plot);
		glutMainLoop();
		#endif
		freeData();
	}
	return 0;
}

