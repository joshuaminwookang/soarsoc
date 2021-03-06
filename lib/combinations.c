
// Testbench C code for ranged comginations
// (c) Josh Kang

#include "rocc.h"
#include "encoding.h"
#include "dispatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/sysinfo.h>


/* Global loop count variable*/
unsigned long long num_loops = 0;
struct timeval start_tv,end_tv;
long start_time, end_time;

/* Global combinations function variable*/
int funct;

void print_stats(){
  /* Conversion constants. */
  const long minute = 60;
  const long hour = minute * 60;
  const long day = hour * 24;
  const double megabyte = 1024 * 1024;
  const long load_fshift = 1 << 16;
  /* Obtain system statistics. */
  struct sysinfo si;
  sysinfo (&si);
  /* Summarize interesting values. */
  printf ("\nsystem uptime : %ld days, %ld:%02ld:%02ld\n",
	  si.uptime / day, (si.uptime % day) / hour,
	  (si.uptime % hour) / minute, si.uptime % minute);
  printf ("total RAM   : %5.1f MB\n", si.totalram / megabyte);
  printf ("free RAM   : %5.1f MB\n", si.freeram / megabyte);
  printf ("process count : %d\n", si.procs);
  printf("load average: %2.2f  %2.2f %2.2f\n", (double) si.loads[0]/load_fshift,(double)  si.loads[1]/load_fshift, (double)  si.loads[2]/load_fshift);

}
// print number of loops, elapsed time 
void sigintHandler(int dummy)
{
  /* Record End Time */
  gettimeofday(&end_tv,NULL);
  end_time = end_tv.tv_sec%(24*3600);

  print_stats();

  /* Reset handler to catch SIGINT next time. */
  printf("\n Results for function: %d with ACCEL: %d \n Elapsed Loops count: %lld\n Elapsed Time: %ld seconds\n",
	   funct, ShellWantsHW, num_loops, end_time-start_time);
  fflush(stdout);
  //  initDNA();
  //  gettimeofday(&start_tv,NULL);
  //  start_time = start_tv.tv_sec%(24*3600);
  exit(0);
} 

/* FUNCT declarations*/
extern int initDNA();
extern int generate0(unsigned int, int, long);
extern int generate1(unsigned int, int, long);
extern int generate2(unsigned int, int, long);
extern int ShellWantsHW;


int main(int argc, char **argv) {
  funct = 0;
  if (argc > 1) funct = atoi(argv[1]); 
  if(initDNA() < 0) exit(1);
  printf("DNA init completed\n");
  
  /* set test env */
  unsigned long inputString;
  long answer;
  printf("FUNCT: %d\n", funct);

  switch(funct) {
    case 0:
      inputString = (1L << WIDTH/2) - 1;
      long lookups1[] = {0,0, 2, 0, 6, 0, 20, 0, 70, 0,0,0,0, 1716, 3432,0, 12870,0,0,0, 184756,0,0,0, 2704156,0,0,0,0,0,0,0,601080390};
      answer = lookups1[WIDTH];
      break;
    case 1:
      inputString = (1L << WIDTH) - 1;
      answer = 1L << WIDTH;
      break;
    case 2:
      inputString = 0;
      long lookups2[] = {0,0, 3, 0, 11, 0, 42, 0, 163, 0,0,0,0, 4096, 9908,0, 39203,0,0,0, 616666,0,0,0, 9740686,0,0,0,0,0,0,0, 2448023843};
      answer = lookups2[WIDTH];
      break;
    default:
      break;
  }

  // Read in new ACCEL environemnt variable and reset HW or SW
  signal(SIGINT, sigintHandler);
  // int limit = 10;
  // if(ShellWantsHW) limit = 100;
  int testResult = 0;
  gettimeofday(&start_tv,NULL);
  start_time = start_tv.tv_sec%(24*3600);
  
  switch (funct){
    case 0:
      while (1) {
        asm volatile ("fence");
        for (int i = 0; i < 100; i++) {
	        testResult = generate0(inputString, WIDTH, answer);
		num_loops++;
        }
        asm volatile ("fence");
      } 
      break;
    case 1:
      while (1) {
        asm volatile ("fence");
        for (int i = 0; i < 100; i++) {
	        testResult = generate1(inputString, WIDTH, answer);
		num_loops++;
        }
        asm volatile ("fence");
      } 
      break;
    case 2: 
      while (1) {
        asm volatile ("fence");
        for (int i = 0; i < 100; i++) {
	        testResult = generate2(inputString, WIDTH, answer);
		num_loops++;
        }
        asm volatile ("fence");
      } 
      break;
  }
  

  gettimeofday(&end_tv,NULL);
  end_time = end_tv.tv_sec%(24*3600);
  printf("\n Results for FUNCT: %d with ACCEL: %d \n Elapsed ops count: %lld\n Elapsed Time: %ld seconds\n",
	 funct, ShellWantsHW,num_loops, end_time-start_time);
  fflush(stdout);
  
  testResult -= answer;
  printf("correctness of result: %d\n", testResult);
  print_stats();
  return testResult;
}
