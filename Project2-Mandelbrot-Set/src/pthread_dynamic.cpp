#include "asg2.h"
#include <cmath>
#include <stdio.h>
#include <pthread.h>
#include <algorithm>

int n_thd; // number of threads
typedef struct {
    int tid;
    int num_threads;
} Args;
int now, max_once, min_once;
pthread_mutex_t busy;

// use locks and global variables to implement job allocation
int job_assign(int *first, int *last) {
	
	int num, len;
	// lock: to keep thread safety
	pthread_mutex_lock(&busy); 
		// critical section
		num = total_size - now;
		*first = now;
		len = std::max(std::min(num / n_thd, max_once), min_once);
		if(len > num) len = num;
		*last = now + len;
		now += len;
	pthread_mutex_unlock(&busy); // unlock
	return num > 0;
}

void* worker(void* arg) {
    Args *args = (Args *) arg;
	int first = 0, last = 0;
	// 1. request for a new job
	while(job_assign(&first, &last)) {
		// 2. finish the job
		while(first != last) {
			compute(data + (first++));
		}
	}
    pthread_exit(NULL);
}

void preprocess() {
	now = 0;
	// to avoid massive job request, 
	// define the maximum and minimum workload
	// if there is less than minimum remaining jobs
	// assign all of them
	max_once = sqrt(total_size);
	min_once = std::max(max_once / n_thd, 1);
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
	preprocess();
    //TODO: assign jobs
    pthread_t thds[n_thd]; // thread pool
    Args args[n_thd]; // arguments for all threads
    for (int thd = 0; thd < n_thd; thd++){
        args[thd].tid = thd;
        args[thd].num_threads = n_thd;
    }
	
	// create pthreads
	pthread_mutex_init(&busy, NULL);
	for (int thd = 0; thd < n_thd; thd++) pthread_create(&thds[thd], NULL, worker, &args[thd]);
    
	// wait for pthreads exit
	for (int thd = 0; thd < n_thd; thd++) pthread_join(thds[thd], NULL);
	pthread_mutex_destroy(&busy);
    //TODO END

    t2 = std::chrono::high_resolution_clock::now();  
	time_span = t2 - t1;
	/* computation part end */
    printf("Student ID: 120090562\n"); // replace it with your student id
	printf("Name: Derong Jin\n"); // replace it with your name
	printf("Assignment 2 Pthread (Dynamic)\n");
	printf("Run Time: %f seconds\n", time_span.count());
	printf("Problem Size: %d * %d, %d\n", X_RESN, Y_RESN, max_iteration);
	printf("Thread Number: %d\n", n_thd);

    #ifdef GUI
	glutMainLoop();
    #endif

	return 0;
}

