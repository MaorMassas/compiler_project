# 🛠️ Assembler Final Project – Course 20465

> **Goal**  Implement a fully‑featured **two‑pass assembler** that translates the course’s miniature assembly language into 24‑bit machine‑code words and emits the canonical trio of output files: **`.ob`**, **`.ent`**, **`.ext`**.

---

## 1  Background

The Open University course **"מעבדה בתכנות מערכות 20465"** tasks us with building an assembler for a hypothetical 24‑bit CPU.

| Concept          | Details                                                                          |
| ---------------- | -------------------------------------------------------------------------------- |
| Registers        | 8 × GPR `r0–r7`                                                                  |
| Memory           | word‑addressable, 0 – 2¹²²‑1; assembler org `MEMORY_START = 100`                 |
| Addressing modes | 0 Immediate `#num` · 1 Direct `LABEL` · 2 Relative `&LABEL` · 3 Register `r0–r7` |
| Instruction size | 1 – 3 words (first *code‑word* + optional *data* words)                          |
| Op‑codes         | 16 primary codes, some with 4‑bit `funct` sub‑codes                              |
| ARE bits         | Every extra word carries `A`,`R`,`E` flags (Absolute / Relocatable / External)   |

Pipeline:

```
.as  ──►  macro‑expander  ──►  .am  ──►  pass 1  ──►  tables  ──►  pass 2  ──►  .ob / .ent / .ext
```

---

## 2  Repository Layout

```
assembler_project/
├── src/
│   ├── main.c
│   ├── pre_prossecor.c / .h    # macro expansion (.as → .am)
│   ├── first_pass.c / .h       # symbol table + skeleton code image
│   ├── second_pass.c / .h      # address resolution + file emit
│   ├── table.c / table.h       # dynamic tables implementation
│   └── util.c / util.h         # helpers (parsing, binary↔hex, validation)
├── tests/
│   ├── ps.as   ps.am           # sample input after / before macros
│   ├── ps.ent  ps.ext          # expected outputs (see §4)
├── Makefile                    # gcc -Wall -ansi -pedantic -std=c89 …
└── docs/ASSEMBLER_PROJECT_OVERVIEW.md *(this file)*
```

> **Coding standard**  Strict **ANSI‑C 89**, variables declared at block start, ≤81 chars/line. Hebrew inline comments for clarity; API & docs in English.

---

## 3  Build & Run

### 3.1  Prerequisites

* **gcc** (tested with v13) capable of `-std=c89`
* POSIX `make`

### 3.2  Compile

```bash
$ git clone https://github.com/MaorMassas/assembler_project.git
$ cd assembler_project
$ make            # produces ./assembler
```

### 3.3  Assemble Source(s)

```bash
$ ./assembler tests/ps.as other_file
```

On success the assembler emits:

| File    | Purpose                                                                |
| ------- | ---------------------------------------------------------------------- |
| `X.ob`  | Object image – first line `IC  DC`, then one `ADDRESS  VALUE` per line |
| `X.ent` | Resolved addresses for `.entry` labels                                 |
| `X.ext` | Every use of an `.extern` symbol                                       |

---

## 4  Example Session (sample program `ps.as`)

```bash
$ ./assembler tests/ps.as
Assembling tests/ps.as …
.entry LIST
.extern W

MAIN:   add     r3, LIST
LOOP:   prn     #48
        mcro    a_mc
        cmp     K, #-6
        bne     &END
        mcroend
        lea     W, r6
        inc     r6
        mov     r3, K
        sub     r1, r4
        bne     END
        a_mc
        dec     W

.entry MAIN
        jmp     &LOOP
        add     L3, L3
END:    stop

STR:    .string "abcd"
LIST:   .data 6, -9
        .data -100
K:      .data 31
.extern L3
Pass 1  OK  (IC=130, DC=14)
Pass 2  OK  – generated tests/ps.ob  tests/ps.ent  tests/ps.ext
```

### 4.1  Resulting `.ent`

```text
LIST 0130
MAIN 0100
```

### 4.2  Resulting `.ext`

```text
W   0105
W   0118
L3  0122
L3  0123
```

### 4.3  Resulting `.ob` *(first 8 lines)*

```text
  IC DC
0100 5A2C
0101 EA00
0102 B200
0103 1231
0104 0000
…      (truncated)
```

*(Full file in `tests/ps.ob` once you run the assembler locally.)*

---

## 5  Source‑Level Flow

### 5.1  Macro Pre‑Processor

* Detects `mcro name` … `mcroend` blocks, stores up to **100** macros, and outputs `.am`.

### 5.2  First Pass

| Counter | Start | Meaning                  |
| ------- | ----- | ------------------------ |
| `IC`    | 100   | next instruction address |
| `DC`    | 0     | next data address        |

Per line the pass:

1. Updates the **symbol table**.
2. Handles directives `.data`, `.string`, `.extern`, `.entry`.
3. Encodes the first word of each instruction immediately (via `util_encode_codeword()`).
4. Pushes a **PendingWord** for each operand that needs resolution, bumping `IC` accordingly.

### 5.3  Second Pass

* Replaces placeholders based on symbol table, writes `.ob`/`.ext`/`.ent`.
* Appends data‑image after code‑image so that addresses remain contiguous.

---

## 6  Key Header APIs

| Header            | Spotlight APIs                                                             |
| ----------------- | -------------------------------------------------------------------------- |
| `util.h`          | `get_addressing_mode()`, `encode_codeword()`, `num_to_twos_complement24()` |
| `table.h`         | dynamic arrays `Symbol`, `Entry`, `Extern`, `Object`, `PendingWord`        |
| `first_pass.h`    | validation matrix `InstructionInfo[NUM_OPCODES][4][4]`                     |
| `second_pass.h`   | `resolve_pending_words()`, `write_object_file()`                           |
| `pre_prossecor.h` | macro storage struct `MacroDef` and limits                                 |

---

## 7  Error Reporting

Errors follow the pattern `file:line: error: …` and suppress output files. Examples:

* *Line exceeds 80 chars*
* *Duplicate label*
* *Illegal addressing mode for opcode*
* *Undefined symbol in pass 2*
* *Macro redefinition* / *unterminated macro*

---

## 8  Roadmap

* [ ] Peephole optimisation – merge dual register operands
* [ ] Automated unit tests (`make test`)
* [ ] GUI visualiser of the two passes

---

## 9  License

**MIT** © 2025 Maor Massas – Final project for course 20465.
