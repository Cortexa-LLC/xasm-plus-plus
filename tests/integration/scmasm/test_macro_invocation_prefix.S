* Test > prefix for macro invocation
* S-C Macro Assembler syntax

				.OR $8000

* Define a simple macro
        .MA LDYAI
        ldy #]1
        lda /]1
        .EM

* Test traditional invocation (without >)
        LDYAI $2000

* Test > prefix invocation (SCMASM style)
        >LDYAI $1000

* Define another macro
        .MA STYA
        sty ]1
        sta ]1+1
        .EM

* Test > prefix with different macro
        >STYA $40

* Test nested invocation
        .MA COMBO
        >LDYAI ]1
        >STYA ]2
        .EM

        >COMBO $3000,$50
