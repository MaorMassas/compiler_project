.entry 1INVALID
.extern

MAIN:   add     #3, #4, r1
LOOP:   prn     ##48
        cmp     , #-6
        bne     &9Wrong
        lea     #4, r6
        inc     #r6
        mov     r3
        sub     r9, r4
        bne     @END
        dec

.entry MAIN EXTRA
        jmp     &
        add     L3 L3
END:    stop now

STR:    .string abcd
LIST:   .data
        .data 6, , 9
1K:     .data 10

.extern L3
.entry L3

DUP:    .data 5
DUP:    .string "oops"
