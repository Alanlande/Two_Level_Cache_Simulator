#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <time.h> 
#define LRU 0
#define RND 1

/* Always use a 32-bit variable to hold memory addresses*/
typedef unsigned long int mem_addr_t;
typedef unsigned long int total_t;

/* a struct that groups cache parameters together */
typedef struct{
  int s;                       /* 2**s cache sets */
  int b;                       /* cacheline block size 2**b bytes */
  int A;                       /* associativity */
  int C;                       /* capacity */
  int IDMode;                  /* instn data mode */
  int allocWriMiss;            /* if miss, write allocate or not*/
  int hitTime;
  int hitTimeDM;
  int replaceAG;               /* replacement algorithm, 0 for LRU, 1 for RND */
  int E;                       /* number of cachelines per set */
  int S;                       /* number of sets, derived from S = 2**s */
  int B;                       /* cacheline block size (bytes), derived from B = 2**b */
} cache_param_t;

//Structure for a line
typedef struct{
  int valid;
  int dirty;
  mem_addr_t tag;
  mem_addr_t addr;
  int timestamp; // the lines with smallest timestamp will be evict
} line_st;

//Structure for a set; a pointer to an array of lines
typedef struct{
  line_st * lines;
} cache_set;

//Structure for a cache; a pointer to an array of sets
typedef struct{
  cache_set * sets;
} cache_t;
// Structure for a cache counter
typedef struct{
  total_t hit_count, rdmiss_count, insmiss_count, wrmiss_count, rd_count, ins_count, write_count, TSTAMP; //value for LRU, register for most recently used line
} cache_count;
// to add counter elements and return a cache counter
cache_count countadd(cache_count Dcount, cache_count Icount){
  cache_count add;
  add.hit_count = Dcount.hit_count + Icount.hit_count;
  add.rdmiss_count = Dcount.rdmiss_count + Icount.rdmiss_count;
  add.insmiss_count = Dcount.insmiss_count + Icount.insmiss_count;
  add.wrmiss_count = Dcount.wrmiss_count + Icount.wrmiss_count;
  add.rd_count = Dcount.rd_count + Icount.rd_count;
  add.ins_count = Dcount.ins_count + Icount.ins_count;
  add.write_count = Dcount.write_count + Icount.write_count;
  return add;
}
  
/* printUsage - Print usage info */
void printUsage( char *argv[] ){
  printf( "\nUsage: %s [-hv] -option  <num>  -t <file>\n", argv[0] );
  printf( "Options (default setting is 0):\n" );
  printf( "  -a <num>   Choose associativity for cache level 1.\n" );
  printf( "  -A <num>   Choose associativity for cache level 2.\n" );
  printf( "  -c <num>   Cache capacity (in byte) for cache level 1.\n" );
  printf( "  -C <num>   Cache capacity (in byte) for cache level 2.\n" );
  printf( "  -M <num>   Set cache mode, 0 for 1 level cache, 1 for 2 levels.\n" );
  printf( "  -w <num>   Allocate write miss on cache level 1, 1 for valid.\n" );
  printf( "  -W <num>   Allocate write miss on cache level 2, 1 for valid.\n" );  
  printf( "  -l <num>   Cache level 1 hit time in cycle.\n" );
  printf( "  -L <num>   Cache level 2 hit time in cycle.\n" );
  printf( "  -D <num>   DRAM hit time in cycle.\n" );
  printf( "  -R <num>   Replacement algorithm, 0 for LRU, 1 for RND.\n" );
  printf( "  -e <num>   Number of lines per set for cache level 1.\n" );
  printf( "  -E <num>   Number of lines per set for cache level 2.\n" );
  printf( "  -s <num>   Number of set index in bit for cache level 1.\n" );
  printf( "  -S <num>   Number of set index in bit for cache level 2.\n" );
  printf( "  -b <num>   Number of block offset in bit for cache level 1.\n" );
  printf( "  -B <num>   Number of block offset in bit for cache level 1.\n" );
  printf( "  -t <file>  Trace file.\n" );
  printf( "  -h         Print this help message.\n" );
  printf( "\nExamples of direct map of one level cache:\n" );
  printf( "  %s  -A 1 -c 8 -C 8 -M 0 -l 2 -L 40 -D 200 -W 1 -R 0 -B 5 -t ./test.txt\n", argv[0] );
  printf( "\nExamples of direct map of 2 level cache:\n" );
  printf( "  %s  -a 1 -A 1 -c 4 -C 16 -M 1 -l 2 -L 40 -D 200 -w 1 -W 1 -R 0 -b 5 -B 6 -t ./test.txt\n", argv[0] );
  printf( "\nExamples of direct map for cache 1 and set-associative for cache 2 of 2 level cache:\n" );
  printf( "  %s  -a 1 -A 2 -c 16 -C 256 -M 1 -l 2 -L 40 -D 200 -w 1 -W 1 -R 0 -E 4 -S 10 -b 5 -B 6 -t ./test.txt\n", argv[0] );
  printf( "  %s  -a 1 -A 2 -c 16 -C 256 -M 1 -l 2 -L 40 -D 200 -w 1 -W 1 -R 0 -E 4 -S 10 -b 5 -B 6 -t ./spec026.ucomp.din.txt\n\n", argv[0]);
  exit(EXIT_FAILURE);
}
// parse the configurations
void readCommOpt(int argc, char ** argv, cache_param_t * par, cache_param_t * parL2, char ** trace){
  char c;
  int i = 0;
  while ( ( c = getopt( argc, argv, "a:A:c:C:M:w:W:l:L:D:R:e:E:s:S:b:B:t:h" ) ) != -1 ){
    switch ( c ){
    case 'a':
      par->A = atoi( optarg );
      break;
    case 'A':
      parL2->A = atoi( optarg );
      break;
    case 'c':
      par->C = atoi( optarg ) * 1024;
      break;
    case 'C':
      parL2->C = atoi( optarg ) * 1024;
      break;
    case 'M':
      par->IDMode = atoi( optarg );
      break;
    case 'w':
      par->allocWriMiss = atoi( optarg );
      break;
    case 'W':
      parL2->allocWriMiss = atoi( optarg );
      break;
    case 'l':
      par->hitTime = atoi( optarg );
      break;
    case 'L':
      parL2->hitTime = atoi( optarg );
      break;
    case 'D':
      parL2->hitTimeDM = atoi( optarg );
      break;
    case 'R':
      par->replaceAG = atoi( optarg );
      parL2->replaceAG = atoi( optarg );
      break;
    case 'e':
      par->E = atoi( optarg );
      break;
    case 'E':
      parL2->E = atoi( optarg );
      break;
    case 's':
      par->s = atoi( optarg );
      break;
    case 'S':
      parL2->s = atoi( optarg );
      break;
    case 'b':
      par->b = atoi( optarg );
      break;
    case 'B':
      parL2->b = atoi( optarg );
      break;
    case 't':
      while(optarg[i] != '\0'){
	(*trace)[i] = optarg[i];
	i++;
      }
      (*trace)[i] = '\0';
      break;
    case 'h':
      printUsage( argv );
      exit( 0 );
    default:
      printUsage( argv );
      exit(EXIT_FAILURE);
    }
  }
  if ( par->C == 0 || par->hitTime == 0 || parL2->C == 0 || parL2->hitTime == 0 || parL2->hitTimeDM == 0 || *trace == NULL){
    printf( "%s: Missing required command line argument\n", argv[0] );
    printUsage( argv );
    exit(EXIT_FAILURE);
  }
  
  par->B = (1 << par->b);
  if(par->A == 0){  //fully associative
    par->s = 0;
    par->S = 1;
    par->E = par->C / par->B;
  }
  else if(par->A == 1){ // direct map
    par->E = 1;
    par->S = par->C / par->B;
    par->s = log2(par->S);
  }
  else {      // set asscociative
    par->S = (1 << par->s);
  }

  parL2->B = (1 << parL2->b);
  if(parL2->A == 0){  //fully associative
    parL2->s = 0;
    parL2->S = 1;
    parL2->E = parL2->C / parL2->B;
  }
  else if(parL2->A == 1){ // direct map
    parL2->E = 1;
    parL2->S = parL2->C / parL2->B;
    parL2->s = log2(parL2->S);
  }
  else {      // set asscociative
    parL2->S = (1 << parL2->s);
  }
}

void printSummary(total_t total, cache_count L1_count){
  printf("Metrics                 Total      Instrn        Data        Read       Write\n");
  printf("--------------          -----      ------        ----        ----       -----\n");
  printf("Demand Fetches     %10ld  %10ld  %10ld  %10ld  %10ld\n", total, L1_count.ins_count, L1_count.rd_count + L1_count.write_count, L1_count.rd_count, L1_count.write_count);
  printf(" Fraction of total   %8f    %8f    %8f    %8f    %8f\n\n", (double)total/total, (double)L1_count.ins_count/total, (double)(L1_count.rd_count + L1_count.write_count)/total, (double)L1_count.rd_count/total, (double)L1_count.write_count/total);
  printf("Demand Misses      %10ld  %10ld  %10ld  %10ld  %10ld\n", L1_count.insmiss_count + L1_count.wrmiss_count + L1_count.rdmiss_count, L1_count.insmiss_count, L1_count.rdmiss_count + L1_count.wrmiss_count, L1_count.rdmiss_count, L1_count.wrmiss_count);
  printf(" Demand miss rate    %8f    %8f    %8f    %8f    %8f\n\n", (double)(L1_count.insmiss_count + L1_count.wrmiss_count + L1_count.rdmiss_count)/total, (double)L1_count.insmiss_count/total, (double)(L1_count.rdmiss_count + L1_count.wrmiss_count)/total, (double)L1_count.rdmiss_count/total, (double)L1_count.wrmiss_count/total);
}

double computeAveragetime(cache_count count, total_t total, int hittime, double misstime){
  double tavg;
  double missrate = (double)(count.insmiss_count + count.wrmiss_count + count.rdmiss_count)/total;
  tavg = hittime + missrate * misstime;
  return tavg;
}

void L2_cache(const cache_param_t * par, cache_t * cache, cache_count * L2_count,const  char act,const total_t addr, total_t * L2Total){
  (*L2Total)++;
  int empty = -1;              //index of empty space
  int H = 0;                   //is there a hit
  int toEvict = 0;             //keeps track of what to evict
  //calculate address tag and set index
  mem_addr_t addr_tag = addr >> ( par->s + par->b );
  int tag_size = ( 64 - ( par->s + par->b ) );
  mem_addr_t temp = addr << ( tag_size );
  temp = temp >> tag_size;
  mem_addr_t setid = temp >>  par->b ;
  cache_set set = cache->sets[setid];
  int low = INT_MAX;
  
  if(act == '0'){
    L2_count->rd_count++;
  }
  else if(act == '2'){
    L2_count->ins_count++;
  }
  else {
    L2_count->write_count++;
  }
  for ( int e = 0; e < par->E; e++ ) {
    if ( set.lines[e].valid == 1 ) {
      // coping with hit, including all hits of three operation
      if ( set.lines[e].tag == addr_tag ) {
	L2_count->hit_count++;
	H = 1;
	if(par->replaceAG == LRU){
	  set.lines[e].timestamp = L2_count->TSTAMP;
	  L2_count->TSTAMP++;
	}
	if(act == '1'){
	  set.lines[e].dirty = 1;
	}
      }
      else{
	// coping with dirty miss, when it is write allocated cache, data needed to be loaded from lower level memory
	if(set.lines[e].dirty == 1){
	  set.lines[e].dirty = 0;
	  if(act == '1' && par->allocWriMiss == 1){
	    set.lines[e].dirty = 1;
	  }
	}
	else{
	  //coping with clean miss
	  if(act == '1' && par->allocWriMiss == 1){
	    set.lines[e].dirty = 1;
	  }
	}
	if(par->replaceAG == RND){
	  srand((unsigned)time(0));
	  toEvict = rand()%(par->E);		
	}
	else{
	  if ( set.lines[e].timestamp < low ) {
	    low = set.lines[e].timestamp;
	    toEvict = e;
	  }
	}
      }
    }
    // coping with invalid bit
    else {
      if(act == '1' && par->allocWriMiss == 1){
	set.lines[e].dirty = 1;
      }
      if( empty == -1 ) {
	empty = e;
      }
    }
  }
  // miss handling, including proper eviction
  if ( H != 1 ){
    if(act == '0'){
      L2_count->rdmiss_count++;
    }
    else if(act == '2'){
      L2_count->insmiss_count++;
    }
    else{
      L2_count->wrmiss_count++;
    }
    //if we have an empty line
    if ( empty > -1 ){
      if((act == '1' && par->allocWriMiss == 1) || act == '0' || (act == '2')){
	set.lines[empty].valid = 1;
	set.lines[empty].tag = addr_tag;
	set.lines[empty].addr = addr;
	if(par->replaceAG == LRU){
	  set.lines[empty].timestamp = L2_count->TSTAMP;
	  L2_count->TSTAMP++;
	}
      }
    }
    else if ( empty < 0 ){
      if((act == '1' && par->allocWriMiss == 1) || act == '0' || (act == '2')){
	set.lines[toEvict].tag = addr_tag;
	set.lines[toEvict].addr = addr;
	if(par->replaceAG == LRU){
	  set.lines[toEvict].timestamp = L2_count->TSTAMP;
	  L2_count->TSTAMP++; //  increment TSTAMP here too
	}
      }
    } 
  }
}

int findMInIndex(line_st * lines ,int n){
  int minIndex = 0;
  int min = lines[0].timestamp;
  for (int i = 1; i < n ; i++){
    if(lines[i].timestamp < min){
      minIndex = i;
      min = lines[i].timestamp;
    }
  }
  return minIndex;
}

void L1_cache(const cache_param_t * parL1, const cache_param_t * parL2, cache_t * L1cache, cache_t * L2cache, cache_count * L1_count, cache_count * L2_count, const char act, const total_t addr, total_t * L2Total){
  int empty = -1;              //index of empty space
  int H = 0;                   //is there a hit
  int toEvict = 0;             //keeps track of what to evict
  //calculate address tag and set index
  mem_addr_t addr_tag = addr >> ( parL1->s + parL1->b );
  int tag_size = ( 64 - ( parL1->s + parL1->b ) );
  mem_addr_t temp = addr << ( tag_size );
  temp = temp >> tag_size;
  mem_addr_t setid = temp >> parL1->b ;
  cache_set set = L1cache->sets[setid];
  //  int low = INT_MAX;
  if(act == '0'){
    L1_count->rd_count++;
  }
  else if(act == '2'){
    L1_count->ins_count++;
  }
  else {
    L1_count->write_count++;
  }
  
  for ( int e = 0; e < parL1->E; e++ ) {
    if ( set.lines[e].valid == 1 ) {
      // coping with hit, including all hits of three operation
      if ( set.lines[e].tag == addr_tag ) {
	L1_count->hit_count++;
	H = 1;
	if(parL1->replaceAG == LRU){
	  set.lines[e].timestamp = L1_count->TSTAMP;
	  L1_count->TSTAMP++;
	}
	if(act == '1'){
	  set.lines[e].dirty = 1;
	}
	break;
      }
      else{
	if(e == parL1->E-1){
	  // coping with dirty miss, when it is write allocated cache, data needed to be loaded from lower level memory   
	  if(set.lines[toEvict].dirty == 1){
	    set.lines[toEvict].dirty = 0;
	    //write back to and read from L2 cache for read miss
	    //note: when it is a dirty read miss, we need to write back first and
	    //search for L2, which needs two times of access
	    L2_cache(parL2, L2cache, L2_count, '1', set.lines[toEvict].addr,L2Total);
	    if(act == '0' || act == '2'){
	      L2_cache(parL2, L2cache, L2_count, act, addr,L2Total); 
	    }
	    else{
	      if(parL1->allocWriMiss == 1 && parL2->allocWriMiss == 1){
		L2_cache(parL2, L2cache, L2_count, '0', addr,L2Total);
	      }
	      set.lines[e].dirty = 1;
	    }
	  }
	  //coping with clean miss
	  else{
	    // read from L2 cache
	    if(act == '0' || act == '2'){
	      L2_cache(parL2, L2cache, L2_count, act, addr,L2Total);
	    }
	    // write
	    else{
	      if(parL1->allocWriMiss == 1 && parL2->allocWriMiss == 1){
		L2_cache(parL2, L2cache, L2_count, '0', addr,L2Total);
	      }
	      set.lines[e].dirty = 1;
	    }
	  }
	  if(parL1->replaceAG == RND){
	    srand((unsigned)time(0));
	    toEvict = rand()%(parL1->E);
	  }
	  else{
	    /*
	    if ( set.lines[e].timestamp < low ) {
	      low = set.lines[e].timestamp;
	      toEvict = e;
	    }
	    */
	    toEvict = findMInIndex(set.lines, parL1->E);
	  }
	}

      }
    }
    // coping with invalid bit
    else{
      if(e == parL1->E - 1){
	if(act == '0' || act == '2'){
	  L2_cache(parL2, L2cache, L2_count, act, addr,L2Total);
	}
	else{
	  if(parL1->allocWriMiss == 1 && parL2->allocWriMiss == 1){
	    L2_cache(parL2, L2cache, L2_count, '0', addr,L2Total);
	  }
	  set.lines[toEvict].dirty = 1;
	}
	if( empty == -1 ) {
	  empty = toEvict;
	}
      }
    }
  }
  // miss handling, including proper eviction
  if ( H != 1 ){
    if(act == '0'){
      L1_count->rdmiss_count++;
    }
    else if(act == '2'){
      L1_count->insmiss_count++;
    }
    else{
      L1_count->wrmiss_count++;
    }
    //if we have an empty line
    if ( empty > -1 ){
      if((act == '1' && parL1->allocWriMiss == 1) || act == '0' || (act == '2')){	
	set.lines[empty].valid = 1;
	set.lines[empty].tag = addr_tag;
	set.lines[empty].addr = addr;
	if(parL1->replaceAG == LRU){
	  set.lines[empty].timestamp = L1_count->TSTAMP;
	  L1_count->TSTAMP++;
	}
      } 
    }
    else if ( empty < 0 ){
      if((act == '1' && parL1->allocWriMiss == 1) || act == '0' || (act == '2')){
	set.lines[toEvict].tag = addr_tag;
	set.lines[toEvict].addr = addr;
	if(parL1->replaceAG == LRU){
	  set.lines[toEvict].timestamp = L1_count->TSTAMP;
	  L1_count->TSTAMP++; //  increment TSTAMP here too
	}
      }
    }
  }
}
