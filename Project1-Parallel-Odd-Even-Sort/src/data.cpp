#include <ctime>
#include <cstdio>
#include <cstring>
#include <cstdlib> 


int main(int argc, char** argv)
{
  if(argc != 3) 
  {
    fprintf(stderr, "Usage: ./data <n> <file>");
    return 1;
  }
  srand(time(NULL));
  int n = atoi(argv[1]);
  FILE* fp = fopen(argv[2], "w");
  fprintf(fp, "%d\n", n);
  for(int i = 0; i < n; i++)
  {
    int value = rand();
    fprintf(fp, "%d\n", value);
  }
  fclose(fp);
  return 0;
}