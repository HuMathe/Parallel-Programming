#include "asg2.h"
#include <stdio.h>
#include <pthread.h>

int n_thd; // number of threads
typedef struct {
    int tid;
    int num_threads;
} Args;


void* worker(void* arg) {
    Args *args = (Args *) arg;
	// work (step=num_thread to ensure fairness)
    for(int i = args->tid; i < total_size; i += args->num_threads){
        compute(data + i);
	}
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {


	if ( argc == 5 ) {
		X_RESN = atoi(argv[1]);
		Y_RESN = atoi(argv[2]);
		max_iteration = atoi(argv[3]);
        n_thd = atoi(argv[4]);
	} else {
		X_RESN = 1000;
		Y_RESN = 1000;
		max_iteration = 100;
        n_thd = 4;
	}

    #ifdef GUI
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Pthread");
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, X_RESN, 0, Y_RESN);
	glutDisplayFunc(plot);
    #endif
    

	/* computation part begin */
    t1 = std::chrono::high_resolution_clock::now();

    initData();

    //TODO: assign jobs
    pthread_t thds[n_thd]; // thread pool
    Args args[n_thd]; // arguments for all threads
    for (int thd = 0; thd < n_thd; thd++){
        args[thd].tid = thd;
        args[thd].num_threads = n_thd;
    }
    for (int thd = 0; thd < n_thd; thd++) pthread_create(&thds[thd], NULL, worker, &args[thd]);
    for (int thd = 0; thd < n_thd; thd++) pthread_join(thds[thd], NULL);
    //TODO END

    t2 = std::chrono::high_resolution_clock::now();  
	time_span = t2 - t1;
	/* computation part end */
    printf("Student ID: 120090562\n"); // replace it with your student id
	printf("Name: Derong Jin\n"); // replace it with your name
	printf("Assignment 2 Pthread (Static)\n");
	printf("Run Time: %f seconds\n", time_span.count());
	printf("Problem Size: %d * %d, %d\n", X_RESN, Y_RESN, max_iteration);
	printf("Thread Number: %d\n", n_thd);

    #ifdef GUI
	glutMainLoop();
    #endif

	return 0;
}

