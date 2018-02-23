#include <limits.h>
#include "cache.h"

int main( int argc, char **argv ){
  cache_param_t parL1, parL2;
  char * trace_file = malloc(sizeof(*trace_file) * 100);
  // parse configurations and initialize parL1 and parL2
  memset(&parL1,0,sizeof(parL1));  
  memset(&parL2,0,sizeof(parL2));  
  readCommOpt(argc, argv, &parL1, &parL2, &trace_file);
  /* Initialize a cache */
  //allocate space for sets and lines
  cache_t L2cache, Dcache, Icache;
  L2cache.sets = malloc( parL2.S * sizeof ( cache_set ) );
  for ( int i = 0; i < parL2.S; i++ ){
    L2cache.sets[i].lines = malloc( sizeof ( line_st ) * parL2.E );
    for(int j = 0; j < parL2.E; j ++){
      memset(&(L2cache.sets[i].lines[j]), 0, sizeof(L2cache.sets[i].lines[j]));
    }
  }
  if(parL1.IDMode == 1){
    Dcache.sets = malloc( parL1.S * sizeof ( cache_set ) );
    Icache.sets = malloc( parL1.S * sizeof ( cache_set ) );
    for ( int i = 0; i < parL1.S; i++ ){
      Dcache.sets[i].lines = malloc( sizeof ( line_st ) * parL1.E );
      Icache.sets[i].lines = malloc( sizeof ( line_st ) * parL1.E );
      for(int j = 0; j < parL1.E; j ++){
	memset(&(Icache.sets[i].lines[j]), 0, sizeof(Icache.sets[i].lines[j]));
	memset(&(Dcache.sets[i].lines[j]), 0, sizeof(Dcache.sets[i].lines[j]));
      }
    }
  }
  // set counters and initialize cache counters
  total_t total = 0, L2Total = 0;
  cache_count Dcount, Icount, L2_count;
  memset(&Dcount, 0, sizeof(Dcount));
  memset(&Icount, 0, sizeof(Icount));
  memset(&L2_count, 0, sizeof(L2_count));
  
  /* Run the trace simulation */
  char act;                    //0: data read, 1: data write, 2: insn fetch
  mem_addr_t addr;
  
  // open the file and read it in
  FILE * traceFile = fopen(trace_file, "r" );
  if ( traceFile != NULL ){
    while ( fscanf( traceFile, " %c %lx", &act, &addr ) == 2 ){
      total++;
      // if unified and one level cache
      if(parL1.IDMode == 0){
	L2_cache(&parL2, &L2cache, &L2_count, act, addr, &L2Total);
      }
      // if 2 level caches: split for first level 1 and unified for level 2
      else{
	// enter level 1 Data cache
	if(act == '0' || act == '1'){
	  L1_cache(&parL1, &parL2, &Dcache, &L2cache, &Dcount, &L2_count, act, addr, &L2Total);
	}
	// enter level 1 Instruction cache
	else{
	  L1_cache(&parL1, &parL2, &Icache, &L2cache, &Icount, &L2_count, act, addr, &L2Total);
	}
      }
    }
  }
  else{
    printf("Failed to open the test trace file.\n");
  }
  /* Print out real results */
  if(parL1.IDMode == 0){
    printf("Only one level cache\n");
    printSummary(total,L2_count);
    double tavg = computeAveragetime(L2_count, L2Total, parL2.hitTime, parL2.hitTimeDM);
    printf("Average cache access time: %f cycles.\n\n", tavg);
  }
  else{
    cache_count L1_count = countadd(Dcount,Icount);
    printf("\nSummary of cache level 1:\n");
    printSummary(total,L1_count);
    printf("\nSummary of cache level 2:\n");
    printSummary(L2Total,L2_count);
    printf("There are %ld more accesses in cache level 2, which are caused by dirty read and write miss. We need to write back first and read again cache level 2.\n\n",L2Total-Dcount.rdmiss_count-Icount.insmiss_count);
    double tavg2 = computeAveragetime(L2_count, L2Total, parL2.hitTime, (double)parL2.hitTimeDM);
    double tavg1 = computeAveragetime(L1_count, total, parL1.hitTime, tavg2);
    printf("Average L1 cache access time: %f cycles.\n", tavg1);
    printf("Average L2 cache access time: %f cycles.\n\n", tavg2);
  }
  fclose(traceFile);
  /* Clean up cache resources */
  free(trace_file);
  for(int i = 0; i < parL2.S; i++){
    free(L2cache.sets[i].lines);
  }
  free(L2cache.sets);

  if(parL1.IDMode == 1){
    for(int i = 0; i < parL1.S; i++){
      free(Dcache.sets[i].lines);
      free(Icache.sets[i].lines);
    }
    free(Dcache.sets);
    free(Icache.sets);
  }
  return 0;
}
