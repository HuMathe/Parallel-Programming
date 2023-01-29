GUI_OPT=-I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -DGUI
COMP_OPT=-O2 -std=c++11 -g
STD_VER=-std=c++11
PROG=cuda openmp seq mpi pthread bonus
PROG_GUI=cudag openmpg seqg mpig pthreadg bonusg
ALL_PROGRAM=*.o $(PROG) $(PROG_GUI)


no_gui: $(PROG)

gui: $(PROG_GUI)

cudag : cudag.o physics.o 
	nvcc -o cudag cudag.o physics.o $(GUI_OPT) $(COMP_OPT) --relocatable-device-code=true

cuda : cuda.o physics.o 
	nvcc -o cuda cuda.o physics.o $(COMP_OPT) --relocatable-device-code=true

openmpg : openmpg.o physics.o 
	g++ -o openmpg openmpg.o physics.o $(COMP_OPT) $(GUI_OPT) -fopenmp

openmp : openmp.o physics.o 
	g++ -o openmp openmp.o physics.o $(COMP_OPT) -fopenmp

seqg : sequentialg.o physics.o 
	g++ -o seqg sequentialg.o physics.o $(COMP_OPT) $(GUI_OPT)

seq : sequential.o physics.o 
	g++ -o seq sequential.o physics.o $(COMP_OPT)

mpig : mpig.o physics.o 
	mpic++ -o mpig mpig.o physics.o $(GUI_OPT) $(STD_VER)

mpi : mpi.o physics.o 
	mpic++ -o mpi mpi.o physics.o $(STD_VER)

bonusg : bonusg.o physics.o 
	mpic++ -o bonusg bonusg.o physics.o $(GUI_OPT) $(STD_VER) -fopenmp

bonus : bonus.o physics.o 
	mpic++ -o bonus bonus.o physics.o $(STD_VER) -fopenmp

pthreadg : pthreadg.o physics.o
	g++ -o pthreadg pthreadg.o physics.o $(COMP_OPT) $(GUI_OPT) -lpthread

pthread : pthread.o physics.o
	g++ -o pthread pthread.o physics.o $(COMP_OPT) -lpthread

sequentialg.o : sequential.cpp vec2d.h gui.h physics.h signature.h
	g++ -c $(GUI_OPT) sequential.cpp $(COMP_OPT) -o sequentialg.o

sequential.o : sequential.cpp vec2d.h gui.h physics.h signature.h
	g++ -c sequential.cpp $(COMP_OPT)

mpig.o : mpi.cpp vec2d.h gui.h physics.h signature.h
	mpic++ -c $(GUI_OPT) mpi.cpp $(STD_VER) -o mpig.o

mpi.o : mpi.cpp vec2d.h gui.h physics.h signature.h
	mpic++ -c mpi.cpp $(STD_VER)

bonusg.o : bonus.cpp vec2d.h gui.h physics.h signature.h
	mpic++ -c $(GUI_OPT) bonus.cpp $(STD_VER) -o bonusg.o -fopenmp

bonus.o : bonus.cpp vec2d.h gui.h physics.h signature.h
	mpic++ -c bonus.cpp $(STD_VER) -fopenmp

cudag.o : cuda.cu vec2d.h gui.h physics.h signature.h
	nvcc -c $(GUI_OPT) cuda.cu $(COMP_OPT) -g -G --relocatable-device-code=true -o cudag.o

cuda.o : cuda.cu vec2d.h gui.h physics.h signature.h
	nvcc -c cuda.cu $(COMP_OPT) -g -G --relocatable-device-code=true


openmpg.o : openmp.cpp gui.h physics.h vec2d.h signature.h
	g++ -c $(GUI_OPT) openmp.cpp $(COMP_OPT) -fopenmp -o openmpg.o

openmp.o : openmp.cpp gui.h physics.h vec2d.h signature.h
	g++ -c openmp.cpp $(COMP_OPT) -fopenmp

pthreadg.o : gui.h vec2d.h physics.h pthread.cpp signature.h 
	g++ -c pthread.cpp $(COMP_OPT) $(GUI_OPT) -o pthreadg.o

pthread.o : gui.h vec2d.h physics.h pthread.cpp signature.h
	g++ -c pthread.cpp $(COMP_OPT)

physics.o : vec2d.h physics.h physics.cpp
	g++ -c physics.cpp $(COMP_OPT)

clean:
	rm -f $(ALL_PROGRAM)

