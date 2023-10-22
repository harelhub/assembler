; errors of labels using and declarations

label1: .data 10
        .entry label2
        .extern label1
        .extern label3
label3: .string "hellow"
label4: mov r3, r2
label4: add r3, r2
        .extern label5
        .extern label5
        .entry label6
        .entry label6
        .entry label7
        .extern label7
        .extern label8
        .entry label8
        jmp label9
        .entry label10
        
                
        
