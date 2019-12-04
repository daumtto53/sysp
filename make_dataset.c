#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]){
  int count;
  int i;
  FILE *file;
  count = atoi(argv[1]);
  file = fopen("dataset", "w+");
  srand(time(NULL));

  fprintf(file, "%d\n",count);
  for(i=0 ; i<count ; i++){  
    fprintf(file, "%4d\n", rand()%10000);
  }

  fclose(file);
  return 0;
}
