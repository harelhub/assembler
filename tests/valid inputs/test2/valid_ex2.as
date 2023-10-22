;this is a valid program with 2 macros

X:     .data 20, -10, +13, 15
str:   .string "world# is@ nice!"
       mcr macro1
       add #5, r1
       sub r3,X
       endmcr
       dec X
       .entry X
       .entry str
       .extern outside
       macro1
       mcr macro2
       clr r5
       mov r2,r5
       endmcr  
       lea outside, r4
       jmp r4
       not r3
       macro2
       jsr outside(#2,X)
       rts   
