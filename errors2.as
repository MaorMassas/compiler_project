START: add  #5

prn  MISSING_LABEL

mcro mov
  inc r2
mcroend

inc  #5

r3:  .data 10

nstru  r0, r1

.extern

ARR:  .data 40000001

FOOBAR: prn #12

mcro  FOOBAR
  dec r4
mcroend
FOOBAR
stop
