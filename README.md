# Two_Level_Cache_Simulator by De Lan

## This is a two level efficient cache simulator

Achieved all configurations of direct-mapped, set-associative, fully associative and functionalities of memory read and write of high accuracy and performances

## Usage:

make

./myCache: Missing required command line argument

Usage: ./myCache [-hv] -option  <num>  -t <file>
  
Options (default setting is 0):

  -a <num>   Choose associativity for cache level 1.
  
  -A <num>   Choose associativity for cache level 2.
  
  -c <num>   Cache capacity (in byte) for cache level 1.
  
  -C <num>   Cache capacity (in byte) for cache level 2.
  
  -M <num>   Set cache mode, 0 for 1 level cache, 1 for 2 levels.
  
  -w <num>   Allocate write miss on cache level 1, 1 for valid.
  
  -W <num>   Allocate write miss on cache level 2, 1 for valid.
  
  -l <num>   Cache level 1 hit time in cycle.
  
  -L <num>   Cache level 2 hit time in cycle.
  
  -D <num>   DRAM hit time in cycle.
  
  -R <num>   Replacement algorithm, 0 for LRU, 1 for RND.
  
  -e <num>   Number of lines per set for cache level 1.
  
  -E <num>   Number of lines per set for cache level 2.
  
  -s <num>   Number of set index in bit for cache level 1.
  
  -S <num>   Number of set index in bit for cache level 2.
  
  -b <num>   Number of block offset in bit for cache level 1.
  
  -B <num>   Number of block offset in bit for cache level 1.
  
  -t <file>  Trace file.
  
  -h         Print this help message.

Examples of direct map of one level cache:

  ./myCache  -A 1 -c 8 -C 8 -M 0 -l 2 -L 40 -D 200 -W 1 -R 0 -B 5 -t ./test.txt

Examples of direct map of 2 level cache:

  ./myCache  -a 1 -A 1 -c 4 -C 16 -M 1 -l 2 -L 40 -D 200 -w 1 -W 1 -R 0 -b 5 -B 6 -t ./test.txt

Examples of direct map for cache 1 and set-associative for cache 2 of 2 level cache:

  ./myCache  -a 1 -A 2 -c 16 -C 256 -M 1 -l 2 -L 40 -D 200 -w 1 -W 1 -R 0 -E 4 -S 10 -b 5 -B 6 -t ./test.txt
  
  ./myCache  -a 1 -A 2 -c 16 -C 256 -M 1 -l 2 -L 40 -D 200 -w 1 -W 1 -R 0 -E 4 -S 10 -b 5 -B 6 -t ./spec026.ucomp.din.txt
