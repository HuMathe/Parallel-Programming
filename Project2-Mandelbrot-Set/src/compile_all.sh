mpic++ mpi.cpp -o mpi -std=c++11
g++ sequential.cpp -o seq -O2 -std=c++11
g++ pthread.cpp -lpthread -o pthread -O2 -std=c++11
g++ pthread_dynamic.cpp -lpthread -o dpthread -O2 -std=c++11