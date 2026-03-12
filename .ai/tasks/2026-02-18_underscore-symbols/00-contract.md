# Task Contract: Investigate Underscore Symbol Parsing

**Task ID:** 2026-02-18_underscore-symbols
**Beads Task:** xasm++-nbs7
**Created:** 2026-02-18
**Assigned Role:** Spelunker
**Workflow:** Research
**Priority:** P1

---

## Context

After implementing :N local labels and case-insensitive symbol lookup, A2osX build still has 366 errors (down from 788 - 54% reduction).

**Previous Fixes:**
- ✅ CRLF line endings
- ✅ Editor command stripping (NEW, AUTO, MAN, SAVE, ASM)
- ✅ SCMASM macros (.MA/.EM, >MacroName, ]1 parameters)
- ✅ :N local labels (`:1`, `:2`, etc.)
- ✅ Case-insensitive symbol lookup (uppercase normalization)

---

## Remaining Errors (366 total)

### 1. Underscore-Prefixed Symbols (363 errors)

**Error Pattern:**
```
error: Invalid argument for _WRITE: Unsupported instruction: _WRITE
error: Invalid argument for _STRLEN: Unsupported instruction: _STRLEN
error: Invalid argument for _STRCMP: Unsupported instruction: _STRCMP
```

**Affected Symbols:** 121 unique (3 occurrences each = 363 errors)

`_ACCEPT`, `_ATANF`, `_ATOF`, `_ATOI`, `_ATOL`, `_BIND`, `_CHAUX`, `_CHDIR`, `_CHMOD`, `_CHOWN`, `_CHTYP`, `_CLOSE`, `_CLOSEDIR`, `_CONNECT`, `_COSF`, `_CTIME2TIME`, `_EXECL`, `_EXECV`, `_EXPAND`, `_EXPF`, `_FCLOSE`, `_FEOF`, `_FFLUSH`, `_FGETS`, `_FILENO`, `_FILESEARCH`, `_FOPEN`, `_FORK`, `_FPRINTF`, `_FPUTC`, `_FPUTS`, `_FREAD`, `_FREE`, `_FREEADDRINFO`, `_FSCANF`, `_FSEEK`, `_FSTAT`, `_FTELL`, `_FWRITE`, `_GETADDRINFO`, `_GETC`, `_GETCHAR`, `_GETCWD`, `_GETDEVBYNAME`, `_GETENV`, `_GETGRGID`, `_GETGRNAME`, `_GETHOSTNAME`, `_GETPEERNAME`, `_GETPS`, `_GETPWNAME`, `_GETPWUID`, `_GETSOCKNAME`, `_GETSOCKOPT`, `_IOCTL`, `_KILL`, `_LISTEN`, `_LOADFILE`, `_LOADTXTFILE`, `_LOGF`, `_MALLOC`, `_MKFIFO`, `_MKDIR`, `_MKNOD`, `_OPEN`, `_OPENDIR`, `_PIPE`, `_PRINTF`, `_PUTS`, `_RAND`, `_READ`, `_READDIR`, `_REALLOC`, `_RECV`, `_RECVFROM`, `_RECVMSG`, `_REMOVE`, `_RENAME`, `_REWIND`, `_RMDIR`, `_SELECT`, `_SEND`, `_SENDMSG`, `_SENDTO`, `_SETATTR`, `_SETENV`, `_SETSOCKOPT`, `_SHUTDOWN`, `_SINF`, `_SLEEP`, `_SOCKET`, `_SPRINTF`, `_SQRTF`, `_SRAND`, `_SSCANF`, `_STAT`, `_STRCASECMP`, `_STRCAT`, `_STRCMP`, `_STRCPY`, `_STRDUP`, `_STRFTIME`, `_STRLWR`, `_STRLEN`, `_STRTOF`, `_STRTOL`, `_STRTOUL`, `_STRUPR`, `_TANF`, `_TIME`, `_UNGETC`, `_UNSETENV`, `_WRITE`

**Questions:**
- Why are underscore-prefixed symbols treated as instructions?
- Where in the parser does this happen?
- How should `_SYMBOL` be differentiated from opcodes?
- What parsing rules does SCMASM use for identifiers?

### 2. Path References (3 errors)

**Error Pattern:**
```
error: Invalid argument for USR/SRC/A2OSX.S: Unsupported instruction: USR/SRC/A2OSX.S
```

**Questions:**
- Where do these path references appear?
- Are they from SAVE/ASM directives?
- Should they be stripped like editor commands?

---

## Investigation Tasks

```
□ Run A2osX build and capture detailed error context
□ Search for underscore symbol usage in A2osX source
□ Examine SCMASM parser identifier parsing logic
□ Check where "Unsupported instruction" error is generated
□ Test underscore symbols in isolation (simple test file)
□ Determine if this is expression parser or syntax parser issue
□ Document parser flow for symbol vs instruction distinction
□ Provide fix recommendations for Engineer
```

---

## Deliverables

1. **Root cause analysis** - Why underscore symbols fail
2. **Parser flow documentation** - Identifier vs instruction logic
3. **Implementation recommendations** - Code changes needed
4. **Test cases** - Verify fix works
5. **Priority/complexity estimate** - Engineering effort

---

## Test Command

```bash
cd ~/Projects/Vintage/Apple/A2osX
XASM=~/Projects/Vintage/tools/xasm++/build/bin/xasm++ make clean && make 2>&1 | grep "^error:" | head -20
```

---

## Known Information

**From xasm++-w0qe Investigation:**
- Case-insensitive symbol lookup works for regular symbols (verified)
- Underscore-prefixed symbols fail with "Unsupported instruction" error
- Test file with `_Write .EQ $1C` and `LDA #_WRITE` fails parsing
- Regular symbols (no underscore) work correctly
- The error occurs during parsing, not during symbol lookup

**Symbol Definitions** (`inc/a2osx.i`):
```scmasm
_Write      .EQ $1C
_Read       .EQ $1A
_Open       .EQ $16
_Close      .EQ $18
... (121 total symbols)
```

**Macro Usage** (`A2osX.API.I.txt`):
```scmasm
.MA LIBC
    ldx #_]1      ; Expands to: ldx #_WRITE
    jsr A2osX.LIBC
.EM
```

---

## Success Criteria

- Root cause identified for underscore symbol parsing
- Parser logic documented (identifier vs instruction detection)
- Fix strategy proposed with code locations
- Test cases provided to verify fix
- Complexity estimate for Engineer implementation

---

## Previous Work

- **xasm++-yywb:** Initial investigation (504 errors → root causes)
- **xasm++-x997:** Fixed :N local labels (16 errors resolved)
- **xasm++-w0qe:** Implemented case-insensitive lookup (discovered underscore issue)

---

**Next Step:** Engineer implementation based on findings
