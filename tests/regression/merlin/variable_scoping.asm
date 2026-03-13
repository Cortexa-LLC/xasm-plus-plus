* Test: ]variable Unique Instance Naming
* Syntax: merlin
* CPU: 6502
*
* Bug: ]variables were not generating unique instance names during
*      multi-pass assembly, causing forward-reference resolution to fail.
*
* Fix: Implemented GenerateVariableInstanceName() which creates unique
*      names like "]varname##N" where N is the instance counter.
*
* This test validates ]variable instance naming and scoping behavior.

         ORG   $8000

* Test 1: Basic ]variable definition and use
]COUNT   =     0
]OFFSET  =     $10

START    LDA   #]COUNT       ; Should be $00
         LDA   #]OFFSET      ; Should be $10

* Test 2: ]variable reassignment (each creates new instance)
]COUNT   =     1
         LDA   #]COUNT       ; Should be $01

]COUNT   =     2
         LDA   #]COUNT       ; Should be $02

]COUNT   =     3
         LDA   #]COUNT       ; Should be $03

* Test 3: Multiple ]variables with different scopes
]X       =     $AA
]Y       =     $BB

LOOP1    LDA   #]X
         LDA   #]Y
         
]X       =     $CC           ; New instance of ]X
         LDA   #]X           ; Should be $CC
         LDA   #]Y           ; Should still be $BB

* Test 4: ]variable in expressions
]BASE    =     $8000
]OFFSET  =     $100

         LDA   ]BASE+]OFFSET ; Should be $8100
         STA   ]BASE+]OFFSET+1 ; Should be $8101

* Test 5: Forward reference with ]variable
* This is the critical test - forward refs need unique instance names

         LDA   #]FORWARD     ; Forward reference
         JMP   NEXT

]FORWARD =     $42
NEXT     NOP

         LDA   #]FORWARD     ; Should still be $42

* Test 6: ]variable scope rules
* ]variables should NOT anchor local label scope
* (only non-':' and non-']' labels should anchor scope)

GLOBAL1  NOP                 ; Global label (anchors scope)
:local1  NOP                 ; Local to GLOBAL1

]VAR1    =     $50           ; Should NOT anchor scope

:local2  NOP                 ; Still local to GLOBAL1 (not VAR1)

GLOBAL2  NOP                 ; New global label
:local3  NOP                 ; Local to GLOBAL2

         LDA   GLOBAL1.:local1
         LDA   GLOBAL1.:local2  
         LDA   GLOBAL2.:local3

         RTS
         END
