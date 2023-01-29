#include <cstdio>
#include <cstring>
#include <algorithm>

int main(int argc, char **argv)
{
  if(argc != 3)
  {
    fprintf(stderr, "Usage: ./check_sorted <file-unsorted> <file-sorted>");
    return 1;
  }
  int n, *a, *b;
  FILE *raw_fp, *new_fp;
  raw_fp = fopen(argv[1], "r");
  new_fp = fopen(argv[2], "r");
  fscanf(raw_fp, "%d", &n);
  a = new int[n];
  b = new int[n];
  for(int i = 0; i < n; i++)
  {
    fscanf(raw_fp, "%d", a + i);
    fscanf(new_fp, "%d", b + i);
  }
  fclose(raw_fp);
  fclose(new_fp);
  std::sort(a, a + n);
  for(int i = 0; i < n; i++)
    if(a[i] != b[i])
    {
      fprintf(stderr, "Error deteced, the sort result is incorrect!\n");
      raw_fp = fopen("result_std.txt", "w");
      for(int i = 0; i < n; i++)
        fprintf(raw_fp, "%d\n", a[i]);
      fclose(raw_fp);
      return 1;
    }
  delete[] a;
  delete[] b;
  fprintf(stderr, "Sort result checked, no problem found\n");
  return 0;
}