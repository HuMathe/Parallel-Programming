/**
 * @file odd_even_parallel.cpp
 * @author Derong Jin (120090562@link.cuhk.edu.cn)
 * @brief Parallel implementation of Odd-Even Sort using MPI
 *      compile:  mpic++ odd_even_parallel.cpp -o p_sort -std=c++11 -w
 *        usage:  mpiexec -np <N core> ./p_sort <data-file-path>
 * 
 * @date 2022-10-11
 * 
 */

#include <mpi.h>
#include <cmath>
#include <cstdio>
#include <chrono>
#include <cstring>
#include <algorithm>
#define ODD_TAG 1
#define ODD_TAG_ 2
#define EVEN_TAG 4
#define EVEN_TAG_ 8

/**
 * @brief Sort the element of a partial sequence, 
 *        processor are co-working in this function
 * 
 * @tparam RandomIt: Random Access Iterator 
 * @param first An iterator
 * @param last Another iterator
 * @param rank the processor's rank
 * @param numprocs the total number of processor
 * @param datatype use to indicate the datatype, 
 *                 should be kept compatible with 
 *                 typeof(*RandomIt)
 * 
 * This is an MPI implementaion of parallel Odd-Even Sort
 * algo., every processor is corresponding to its first and
 * last iterator, and is responsible to sort elements in 
 * [first, last) in acsending order. When executing, 
 * processors are conducting odd-even sort in its own range, 
 * as well as in extended range by comparing the boundary element
 * sent by its adjacant processors. The algorithm is equivalent
 * to sequential version, however is more efficiency in some 
 * (large) case.
 */
template<typename RandomIt>
void inplace_odd_even_sort_parallel(
  RandomIt first, RandomIt last, int rank, int numprocs, 
  MPI_Datatype datatype)
{
  // Nothing to do when there is no elements.
  if(first == last) return ;
  
  // MPI variables
  MPI_Status status;
  MPI_Request send_request, recv_request;

  // status
  int done = 0, sorted = 0;

  // information related to the partial sequence
  // fill_buffer indicates sending information to privious processor
  // check_buffer indicates receiving information
  // odd and even means the duty is on odd/even round
  int length = std::distance(first, last);
  int odd_fill_buffer = 1 - rank * length % 2;
  int even_fill_buffer = rank * length % 2;
  int odd_check_buffer = 1 - (rank + 1) * length % 2;
  int even_check_buffer = (rank + 1) * length % 2;
  if( rank == numprocs - 1 ) odd_check_buffer = even_check_buffer = 0;
  if( rank == 0 ) odd_fill_buffer = even_fill_buffer = 0;
  
  // the first/last element that needs compare in odd/even round 
  RandomIt even_first = first + rank * length % 2;
  RandomIt even_last = last - (rank + 1) * length % 2;
  RandomIt odd_first = first + (1 - rank * length % 2);
  RandomIt odd_last = last - (1 - (rank + 1) * length % 2);
  
  // allocating buffer, into which storing the message
  RandomIt recv_buffer = (RandomIt) malloc(sizeof(*first));
  RandomIt send_buffer = (RandomIt) malloc(sizeof(*first));
  RandomIt maxv_buffer = (RandomIt) malloc(sizeof(*first));
  while( ! done )
  {
    sorted = 1;

    // odd sort
    for(RandomIt i = odd_first; i != odd_last; i += 2)
    {
      if(*(i) > *(i + 1))
      {
        std::iter_swap(i, i + 1);
        sorted = 0;
      }
    }

    // on odd boundary, Isend/Irecv to avoid deadlock
    *send_buffer = *first;
    if( odd_fill_buffer ) MPI_Isend(send_buffer, 1, datatype, rank - 1, ODD_TAG, MPI_COMM_WORLD, &send_request);
    if( odd_check_buffer ) MPI_Irecv(recv_buffer, 1, datatype, rank + 1, ODD_TAG, MPI_COMM_WORLD, &recv_request);
    if( odd_fill_buffer ) MPI_Wait(&send_request, &status);
    if( odd_check_buffer ) 
    {
      MPI_Wait(&recv_request, &status);
      if( *recv_buffer < *(last - 1) )
      {
        std::iter_swap(recv_buffer, last - 1);
        sorted = 0;
      }
      MPI_Isend(recv_buffer, 1, datatype, rank + 1, ODD_TAG_, MPI_COMM_WORLD, &send_request);
    }
    if( odd_fill_buffer ) 
    {
      MPI_Recv(maxv_buffer, 1, datatype, rank - 1, ODD_TAG_, MPI_COMM_WORLD, &status);
      if( *first < *maxv_buffer ) 
      {
        *first = *maxv_buffer;
        sorted = 0;
      }
    }

    
    // even sort
    for(RandomIt i = even_first; i != even_last; i += 2)
    {
      if(*(i) > *(i + 1))
      {
        std::iter_swap(i, i + 1);
        sorted = 0;
      }
    }

    // on even boundary
    *send_buffer = *first;
    if( even_fill_buffer ) MPI_Isend(send_buffer, 1, datatype, rank - 1, EVEN_TAG, MPI_COMM_WORLD, &send_request); 
    if( even_check_buffer ) MPI_Irecv(recv_buffer, 1, datatype, rank + 1, EVEN_TAG, MPI_COMM_WORLD, &recv_request);
    if( even_fill_buffer ) MPI_Wait(&send_request, &status);
    if( even_check_buffer )
    {
      MPI_Wait(&recv_request, &status);
      if( *recv_buffer < *(last - 1) )
      {
        std::iter_swap(recv_buffer, last - 1);
        sorted = 0;
      }
      MPI_Isend(recv_buffer, 1, datatype, rank + 1, EVEN_TAG_, MPI_COMM_WORLD, &send_request);
    }
    if( even_fill_buffer )
    {
      MPI_Recv(maxv_buffer, 1, datatype, rank - 1, EVEN_TAG_, MPI_COMM_WORLD, &status);
      if( *first < *maxv_buffer ) 
      {
        *first = *maxv_buffer;
        sorted = 0;
      }
    }
    
    // the array is sorted if no change is detected in each processor
    MPI_Allreduce(&sorted, &done, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);  
  }

  // free the memory 
  free(send_buffer);
  free(recv_buffer);
  free(maxv_buffer);
  return ;
}

// print my information
void my_info()
{
  fputs(
    "Name: Derong Jin\nStudent ID: 120090562\nAssignment 1, Odd-Even Transposition Sort, parallel version, MPI implementation\n", 
    stdout
  );
}

// main function
int main(int argc, char **argv)
{
  // declaration
  int *a, *arr;
  int numprocs, rank, real_n, n;

  // MPI initialization
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  
  // load data
  if(rank == 0 && argc == 2)
  {
    n = -1; // to detect error.
    FILE* fp = fopen(argv[1], "r");
    if(fp != NULL)
    {
      fscanf(fp, "%d", &real_n);
      n = real_n % numprocs == 0 ? real_n : (real_n / numprocs + 1) * numprocs;
      arr = new int[n];
      int inf = 0x7fffffff; // INT_MAX
      for(int i = 0; i < real_n; i++) fscanf(fp, "%d", arr + i);
      for(int i = real_n; i < n; i++) arr[i] = inf;
      fclose(fp);
    }
  }

  // sorting & timing
  std::chrono::high_resolution_clock::time_point time1, time2;
  time1 = std::chrono::high_resolution_clock::now();
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if(n == -1) {
    if(rank == 0)
      fprintf(stderr, "Usage: ./psort <input-file-name>\n");
    return 1;
  }
  a = new int[n / numprocs + 1];
  MPI_Scatter(arr, n / numprocs, MPI_INT, a, n / numprocs, MPI_INT, 0, MPI_COMM_WORLD);
  inplace_odd_even_sort_parallel(a, a + n / numprocs, rank, numprocs, MPI_INT);
  MPI_Gather(a, n / numprocs, MPI_INT, arr, n / numprocs, MPI_INT, 0, MPI_COMM_WORLD);
  time2 = std::chrono::high_resolution_clock::now();

  // print out and terminate
  if(rank == 0)
  {
    char* output_path = new char[strlen(argv[1]) + 30];
    sprintf(output_path, "%s.parallel.out", argv[1]);
    my_info();
    fprintf(stdout, "runTime is %.6lf sec\n",
      std::chrono::duration<double, std::ratio<1, 1>>(time2 - time1).count()
    );
    fprintf(stdout, "Input Size: %d\nProc Num: %d\n", real_n, numprocs);
    
    // if n is small, also print to stdout
    if( real_n <= 20 )
    { 
      for(int i = 0; i < real_n; i++) 
        fprintf(stdout, "%d ", *(arr + i));
      fprintf(stdout, "\n");
    }
    
    // print out to result file
    FILE* fp = fopen(output_path, "w");
    for(int i = 0; i < real_n; i++) fprintf(fp, "%d\n", *(arr + i));
    fclose(fp);
    delete[] arr;
    delete[] output_path;
  }

  delete[] a;

  MPI_Finalize();
  return 0;
}