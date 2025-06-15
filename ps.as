.entry LIST
.extern W

MAIN:   add     r3, LIST
LOOP:   prn     #48
        mcro    a_mc
        cmp     K, #-6
        bne     &END
        mcroend
        lea     W, r6
        inc     r6
        mov     r3, K
        sub     r1, r4
        bne     END
        a_mc
        dec     W

.entry MAIN
        jmp     &LOOP
        add     L3, L3
END:    stop

STR:    .string "abcd"
LIST:   .data 6, -9
        .data -100
K:      .data 31
.extern L3
