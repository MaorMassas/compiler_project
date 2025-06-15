.entry MAIN
.extern W

MAIN:
        mov   r1, LIST
        add   r2, r3
        mcro  a_mc
        clr   r5
        not   r6
        inc   r7
        mcroend
        sub   LIST, r4
        lea   STR, r5
        red   r2
        prn   #23
        jmp   &LOOP

LOOP:
        cmp   r3, K
        bne   &END
        a_mc
        dec   K
        jsr   &NEXT

NEXT:
        rts

END:
        stop

STR:
        .string "example"

LIST:
        .data 12, -8
        .data 22, 5

K:
        .data 45


