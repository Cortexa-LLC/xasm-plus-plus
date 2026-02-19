* A2oSX-style macro test
* Tests parameter substitution patterns used in A2oSX

        .OR $2000

* Test 1: Simple parameter substitution
LDYA    .MA
        ldy ]1
        lda ]1+1
        .EM

* Test 2: Prefixed parameters (immediate mode)
LDYAI   .MA
        ldy #]1
        lda #/]1
        .EM

* Test 3: Multiple parameters
STYA    .MA
        sty ]1
        sta ]1+1
        .EM

* Test 4: Zero page addressing
CLEAR   .MA
        lda #0
        sta ]1
        .EM

* Test usage
        >LDYA $40       * Load Y with $40, A with $41
        >LDYAI $1234    * Load YA with immediate $1234
        >STYA $50       * Store YA to $50/$51
        >CLEAR $80      * Clear zero page location $80
