; Test: .DA directive with string literals containing commas
; Bug fix: Bug 24 - commas inside $$"..." should not split operands
; Expected: Each .DA should emit the string bytes correctly

	.DA $$"(a),Y"        ; Should emit: 28 61 29 2c 59
	.DA $$"aa,X)"        ; Should emit: 61 61 2c 58 29
	.DA $$"a,X"          ; Should emit: 61 2c 58
	.DA #5,$$"(a),Y"     ; Should emit: 05 28 61 29 2c 59
