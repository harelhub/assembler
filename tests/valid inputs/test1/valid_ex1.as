;this is a valid program without macros

sum:  .data 10    
      mov A, r1
      cmp A,r4
      add #5, sum
      sub A, sum
A:    .data 23
Str:  .string "my sentence"
      .extern MyLabel
      lea MyLabel, r6
      .entry A
      bne Here(#9,A)   
      clr r1	
Here: red r5
      prn r5    
      inc A	  
      stop	
