; errors with macros

mov r2, r3
mcr m1 4
   jmp r3
   add #1, r5
endmcr
inc r2
prn #5
mcr m2
   mov r1, r3
   dec r2
endmcr     a   
stop      
