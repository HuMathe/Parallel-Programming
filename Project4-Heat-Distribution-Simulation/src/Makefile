GUI_OPT=-I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -DGUI
COMP_OPT=-O2 -std=c++11
STD_VER=-std=c++11
PROG=cuda openmp seq mpi pthread bonus
PROG_GUI=cudag openmpg seqg mpig pthreadg bonusg

no_gui: $(PROG)

gui: $(PROG_GUI)

seq: seq.o heat_system.o 
	g++ $(COMP_OPT) seq.o heat_system.o -o seq 

seqg: seqg.o heat_system.o 
	g++ $(COMP_OPT) $(GUI_OPT) seqg.o heat_system.o -o seqg

pthread: pthread.o heat_system.o 
	g++ $(COMP_OPT) pthread.o heat_system.o -o pthread -lpthread

pthreadg: pthreadg.o heat_system.o 
	g++ $(COMP_OPT) $(GUI_OPT) pthreadg.o heat_system.o -o pthreadg -lpthread

openmp: openmp.o heat_system.o 
	g++ $(COMP_OPT) openmp.o heat_system.o -o openmp -fopenmp

openmpg: openmpg.o heat_system.o 
	g++ $(COMP_OPT) $(GUI_OPT) openmpg.o heat_system.o -o openmpg -fopenmp

mpi: mpi.o heat_system.o 
	mpic++ $(STD_VER) mpi.o heat_system.o -o mpi 

mpig: mpig.o heat_system.o 
	mpic++ $(STD_VER) $(GUI_OPT) mpig.o heat_system.o -o mpig

cuda: cuda.o heat_system.o 
	nvcc cuda.o heat_system.o $(COMP_OPT) -o cuda --relocatable-device-code=true

cudag: cudag.o heat_system.o 
	nvcc cudag.o heat_system.o $(COMP_OPT) $(GUI_OPT) -o cudag --relocatable-device-code=true

bonus: bonus.o heat_system.o 
	mpic++ $(STD_VER) bonus.o heat_system.o -o bonus -fopenmp

bonusg: bonusg.o heat_system.o 
	mpic++ $(STD_VER) $(GUI_OPT) bonusg.o heat_system.o -o bonusg -fopenmp

seq.o: seq.cpp signature.h heat_system.h gui.h
	g++ $(COMP_OPT) -c seq.cpp -o seq.o

seqg.o: seq.cpp signature.h heat_system.h gui.h 
	g++ $(COMP_OPT) $(GUI_OPT) -c seq.cpp -o seqg.o

pthread.o: pthread.cpp signature.h heat_system.h gui.h 
	g++ $(COMP_OPT) -c pthread.cpp -o pthread.o -lpthread

pthreadg.o: pthread.cpp signature.h heat_system.h gui.h 
	g++ $(COMP_OPT) $(GUI_OPT) -c pthread.cpp -o pthreadg.o -lpthread

openmp.o: openmp.cpp signature.h heat_system.h gui.h 
	g++ $(COMP_OPT) -c openmp.cpp -o openmp.o -fopenmp

openmpg.o: openmp.cpp signature.h heat_system.h gui.h 
	g++ $(COMP_OPT) $(GUI_OPT) -c openmp.cpp -o openmpg.o -fopenmp

mpi.o: mpi.cpp signature.h heat_system.h gui.h 
	mpic++ $(STD_VER) -c mpi.cpp -o mpi.o 

mpig.o: mpi.cpp signature.h heat_system.h gui.h 
	mpic++ $(STD_VER) $(GUI_OPT) -c mpi.cpp -o mpig.o 

cuda.o: cuda.cu signature.h heat_system.h gui.h 
	nvcc -c cuda.cu -o cuda.o --relocatable-device-code=true $(COMP_OPT)

cudag.o: cuda.cu signature.h heat_system.h gui.h
	nvcc -c cuda.cu -o cudag.o --relocatable-device-code=true $(COMP_OPT) $(GUI_OPT)

bonus.o: bonus.cpp signature.h heat_system.h gui.h 
	mpic++ $(STD_VER) -c bonus.cpp -o bonus.o -fopenmp

bonusg.o: bonus.cpp signature.h heat_system.h gui.h 
	mpic++ $(STD_VER) $(GUI_OPT) -c bonus.cpp -o bonusg.o -fopenmp

heat_system.o: heat_system.cpp heat_system.h 
	g++ $(COMP_OPT) -c heat_system.cpp -o heat_system.o


.PHONY: clean
clean: 
	rm -f $(PROG) $(PROG_GUI) *.o