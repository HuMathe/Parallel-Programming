/**
 * @file odd_even_serial.cpp
 * @author Derong Jin (120090562@link.cuhk.edu.cn)
 * @brief Sequential implementation of Odd-Even Transportation Sort
 *      compile:  g++ odd_even_serial.cpp -o s_sort -std=c++11 -w
  *       usage:  ./s_sort <data-file-path>
 * @date 2022-10-11
 * 
 */
#include <cstdio>
#include <chrono>
#include <cstring>
#include <algorithm>


/**
 * @brief Sort the elements of a sequence, using sequential 
 *        odd-even sort
 *  
 * @param first   An iterator
 * @param last    Another iterator
 * @return Nothing.
 * 
 * This is an implementation of sequential Odd-Even Sort algo. 
 * that sorts elements in [first, last) in acsending order, 
 * i.e., *(i) <= *(i + 1) holds for all i in the given range. 
 * When executing, this algo. checks if elements and their 
 * posterior neighbours are in ascending order, and do 
 * correction if they are not. The iterative checking operation 
 * is conducted twice, on every element with odd index and 
 * with even index. And the order of the check-and-swap 
 * operation of elements in each checking operation has no 
 * effect on efficiency and correctness.
*/
template<typename RandomAccessIt>
void inplace_odd_even_sort_serial(RandomAccessIt first, 
  RandomAccessIt last)
{
  int length = std::distance(first, last);
  int sorted = (length == 0), __even = length & 1,\
    __odd = 1 ^ (length & 1);
  while( ! sorted ) // not sorted
  {
    sorted = 1;
    // Odd sort
    for(RandomAccessIt __i = first + 1; __i + __odd != last; __i += 2)
    {
      if( *(__i) > *(__i + 1) )
      {
        std::iter_swap(__i, __i + 1);
        sorted = 0;
      }
    }

    // Even sort, the same process with different start point
    for(RandomAccessIt __i = first; __i + __even != last; __i += 2)
    {
      if( *(__i) > *(__i + 1) )
      {
        std::iter_swap(__i, __i + 1);
        sorted = 0;
      }
    }
  }
  return ;
}

// print my information
void my_info()
{
  fputs(
    "Name: Derong Jin\nStudent ID: 120090562\n",
    stdout);
  fputs(
    "Assignment 1, Odd-Even Transposition Sort, sequenential version\n", 
    stdout
  );
  return ;
}


// main function
int main(int argc, char **argv)
{
  // read
  int n, *a;
  FILE* fp = fopen(argv[1], "r");
  fscanf(fp, "%d", &n);
  a = new int[n];
  for(int i = 0; i < n; i++) fscanf(fp, "%d", a + i);
  fclose(fp);

  // sort
  auto time1 = std::chrono::high_resolution_clock::now();
  inplace_odd_even_sort_serial(a, a + n);
  auto time2 = std::chrono::high_resolution_clock::now();
  my_info();
  fprintf(stdout, "runTime is %.6lf sec\n",
   std::chrono::duration<double, std::ratio<1, 1>>(time2 - time1).count()
  );

  // if n is small, also print to stdout
  if( n <= 20 )
  {
    for(int i = 0; i < n; i++) 
      fprintf(stdout, "%d ", *(a + i));
    fprintf(stdout, "\n");
  }
    

  // write and terminate
  char* output_path = new char[strlen(argv[1]) + 30];
  sprintf(output_path, "%s.sequential.out", argv[1]);
  fp = fopen(output_path, "w");
  for(int i = 0; i < n; i++) fprintf(fp, "%d\n", *(a + i));
  fclose(fp);
  delete[] output_path;
  delete[] a;
  return 0;
}