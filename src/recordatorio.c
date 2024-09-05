#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void recordatorio(int time, const char* msg){
    sleep(time);
    printf("Recordatorio: %s\n", msg);
  }