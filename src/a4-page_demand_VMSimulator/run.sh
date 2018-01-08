#!/bin/sh

page=256
memsize=25600 # 100*page
policy="sec"
virtual=2560000 # 100*memsize
nref=25600000 # 10*virtual address space

./mrefgen -m $virtual -l 3 -f 3 -w 0.4 -n $nref | ./a4vmsim $page $memsize $policy