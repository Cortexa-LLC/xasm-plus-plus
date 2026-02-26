* Test: Inline comments after directives (no semicolon)
* Syntax: SCMASM
* CPU: 65C02
*
* A2osX uses whitespace-separated inline comments on many directives.
* These must be ignored rather than parsed as part of the expression.

*--------------------------------------
K.FD.MAX	.EQ	64
K.FILE.MAX	.EQ	32
K.PS.MAX	.EQ	16
ZPTMP		.EQ	$80
K.Data		.EQ	$100
*--------------------------------------
		.OR	$2000
*--------------------------------------
* .OR with inline comment
		.DUMMY
		.OR	K.Data
*--------------------------------------
		.OR ZPTMP				6 Bytes
		.OR ZPTMP+8				7 Bytes
*--------------------------------------
* .BS with numeric count and inline comment
		.BS 9					9 bytes, S.IOCTL
		.BS 51					18 bytes for GetFileInfo
		.BS 64					64 bytes, S.STAT
*--------------------------------------
* .BS with symbol*literal expression and inline comment
		.BS K.FD.MAX*2			pFDs
		.BS K.FD.MAX*2			pNames
		.BS K.FILE.MAX*2		pFILEs
		.BS K.PS.MAX*2			pPSs
		.ED
*--------------------------------------
* .EQ with inline comment
TIMEOUT		.EQ 180				float
TIMING		.EQ $58+00			5/6 for 50/60Hz
*--------------------------------------
* .SE with inline comment
COUNT		.SE 3				loop count
*--------------------------------------
		.OR $2000
		NOP
MAN
SAVE usr/src/inline_comments.s
ASM
