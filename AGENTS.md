# AGENTS.md

Compiler + interpreter for a new language (`cpl2`). Flat single project: no packages, no CI,
no README, no formatters. Sources at the repo root; code comments, grammar notes and commit
messages are in Italian — keep them that way.

## Build

```sh
make                # -> interpreter, compiler, asm   (no clean/test target)
```

- `interpreter` = `c++ -g -O2 interpreter.cpp` (~5s, needs C++)
- `compiler`    = `cc parser.c`
- `asm`         = `cc asm.c`
- `lemon` must be installed; **any version will do.** The project pins no lemon version and never
  will — regenerate with whatever `lemon` is installed. Differences in the generated template
  boilerplate are not a problem: a newer lemon rewrites ~300 unrelated lines of `cpl2.c` and that's
  expected. Never hand-edit or hand-reconcile `cpl2.c` to make the diff smaller; commit the output
  as-is. The only hard requirement is that the generated parser still builds and parses the grammar
  with no conflicts (lemon exits 0 — currently true).
- `cpl2.c` **and** `cpl2.h` **and** `cpl2.out` are all generated from `cpl2.gram`; only `cpl2.c` has a
  Makefile rule. `cpl2.c`/`cpl2.h` are committed — regenerate and commit them whenever you touch the
  grammar.

## Compiler — sources in argv, one `.pcd` per source

```sh
./compiler fib.cpl primo.cpl          # cpl2 sources -> fib.pcd, primo.pcd
./compiler < fib.cpl > fib.pcd        # no args: still reads stdin, still writes stdout
./asm < fib.pcd                       # binary p-code -> readable text (inspect only)
```

- With file arguments the compiler writes **nothing on stdout**: each source produces its own p-code
  next to itself, extension replaced by `.pcd` (`fib.cpl` -> `fib.pcd`, `test/a.cpl` ->
  `test/a.pcd`, no extension -> append `.pcd`). With no arguments it keeps the old stdin/stdout
  behaviour, so the `< fib.cpl > fib.pcd` pipeline is unchanged.
- **Input path resolution** (`openSource()` in `parser.c`): the typed path is tried first and wins
  if it opens. Only when it fails **because it doesn't exist** (`ENOENT`) *and* the **base name**
  carries no dot is `".cpl"` appended as a fallback (`fib` -> `fib.cpl`, `dir.d/x` -> `dir.d/x.cpl`
  — the dot in the directory is not an extension). A name that already has an extension is never
  rewritten (`sorg.txt` tries `sorg.txt` and nothing else, not `sorg.txt.cpl` nor `sorg.cpl`), and
  a failure other than "absent" (permissions) is **not** a fallback: `fib` unreadable reports `fib`,
  never silently compiles `fib.cpl`.
- The **resolved** path — not the typed one — is what `srcFile` and `pcodePath()` receive, so
  `./compiler fib` resolving to `fib.cpl` gives diagnostics prefixed `fib.cpl: ` and still writes
  `fib.pcd` (same stem, so the output name never changes). The "cannot open" diagnostic names every
  path that was actually tried, i.e. both `fib` and `fib.cpl` only when the fallback ran.
- **Compiler exit code == parse error count**, summed over every source given, including sources
  that couldn't be opened or whose `.pcd` couldn't be written (those are reported and skipped, the
  run continues).
- Diagnostics (`syntax error at line N!`, `stringa non terminata`, `numero terminato male`) go to
  **stderr** with the source path as prefix (`b.cpl: syntax error at line 3!`); in stdin mode there
  is no prefix. They are no longer baked into the `.pcd`, so the file only ever holds p-code — a
  failed compile just stops early and leaves the p-code without `PCODEEND`.
- All diagnostics go through `diag()` in `parser.c` (including the `%syntax_error` action in
  `cpl2.gram`); the p-code destination is the global `emit_out` in `emit.c`.
- `.pcd` header must be exactly `#cpl20.0.1` — hardcoded in `parse()` (`parser.c`) and again in
  `pcodeProgram::loadPcd` (`interpreter.cpp`). Bump both together.
- File format: one p-code per line, first char = `pcode + 31`, rest of line = operand (int or
  string). Note `P_PLUS` (1) encodes as a leading space.
- Last line must be `PCODEEND 0`, otherwise `loadPcd` prints `<file> has errors!` and runs nothing.
- `emit_asm` is hardcoded to `0` in `parse()`; `./asm` is the only way to get readable p-code.

## `asm` is a checker, not a pipeline stage

`asm.c` is only there to eyeball a `.pcd`: `./asm < fib.pcd` prints the readable listing. It still
contains the reverse pass (readable -> binary, keyed off the first line) and that round trip is
byte-exact today, but nothing needs it — the interpreter reads the binary `.pcd` the compiler just
wrote, so don't build anything on top of it.

## Running a program — one argument, or `-bench` plus one argument

`./interpreter` takes **one argument**: the p-code to run. The single flag is `-bench`, and it
goes **before** the path.

```sh
./compiler mio.cpl             # writes mio.pcd next to the source
./interpreter mio.pcd          # single run; the .pcd can be omitted: ./interpreter mio
./interpreter -bench mio.pcd   # nanobench loop instead of a single run — ~30-60s for fib(34)
```

Neither running nor benchmarking your own file requires editing `main()` any more. The two
functions behind those two lines are:

```cpp
runPcd("mio.pcd");     // single run  (asserts the stack unwound to sp==-1)
benchPcd("mio.pcd");   // nanobench loop around runPcd(), same exit code
```

A name with no extension gets `.pcd` appended, and the dot is looked for **in the base name
only**, so `dir.d/x` stays `dir.d/x.pcd`. A name that already has an extension is used verbatim:
`./interpreter fib.cpl` tries to load `fib.cpl` and says it is not compiled p-code. `-bench` is
matched as an exact string and only in first position: every other argument starting with `-` is
a path, so `./interpreter -h` still tries to open a file named `-h`, and
`./interpreter --bench mio.pcd` prints the usage and runs nothing.

A file that can't be opened, isn't compiled p-code, or has compile errors prints the reason on
stderr and exits 1 without running anything. The same holds with `-bench`: `benchPcd()` loads the
file once before measuring and returns 1 on failure, so an unloadable file prints no nanobench
table. A clean run — or a clean measurement — exits 0. `main()` keeps printing the GC report on
stdout afterwards in both modes, so `> out.txt` captures the program output, the nanobench table
*and* that report.

Open work, see `todo.md`. Plain `./interpreter` is **not** a smoke test: with no arguments, with
more than one argument, and with a bare `-bench` it prints usage on stderr and exits 1 without
running or measuring anything.

Debug switches in `interpreter.cpp`:
- `#define PRINT_PCODE_EXECUTION` — per-instruction `pc / sp / obj` trace. Start here.
- `#define RUN_WITH_SWITCH` — misnamed. When defined, `run()` uses a computed-goto fast path for
  15 hot pcodes (`pcodejump[]`) and falls back to virtual `pcode::exec` via `default_pcode` for
  everything else; when undefined it's a plain `prg[pc].exec()` loop. The duplicated dispatch is
  deliberate, don't "simplify" it away.

## Wiring

- `parser.c` is the compiler's single translation unit: it `#include`s `pcodes.c`, then `emit.c`,
  then the lemon-generated `cpl2.c`. `cpl2.h` holds only `TOK_*` codes; the hand-written lexer in
  `parser.c` (`getTokenCodeFromString`) assigns them.
- Grammar actions in `cpl2.gram` emit p-code directly through `emit()/emit_i()/emit_s()` in
  `emit.c`. `emit.c` and `pcodes.c` are shared with the interpreter.
- `interpreter.cpp` `#include`s `gc.cpp` (generational mark & sweep; tunables at the top of
  `gc.cpp`) and `pcodes.h`/`pcodes.c`. `nanobench.h` is vendored.
- `gc_test.cpp` is standalone (its own `GCObject` copy and `main`), not in the Makefile:
  `c++ -g -O2 gc_test.cpp -o gc_test && ./gc_test`. `test/gc_test.cpp` is a different, untracked
  variant — don't confuse them.

## Adding a p-code: four coordinated edits

1. `pcodes.h` — `#define P_XXX n`; bump `MAX_PCODE` if it's now the highest (used by `asm.c`'s
   `FindPCode` loop bound).
2. `pcodes.c` — add `pcodetxt[P_XXX]="XXX"` in `initpcodetxt()`.
3. `cpl2.gram` — emit it, then regenerate `cpl2.c`/`cpl2.h`.
4. `interpreter.cpp` — a `pcodeXXX` class overriding `exec()` **and** a `case P_XXX:` in
   `makePCode()`. Only add it to `pcodejump[]` if it is genuinely hot.

The trailing `// 78` numeric comments in `pcodes.c` are hand-written and already stale — two
entries have the values swapped (`P_ABSTRACT`=85, `P_EVENT`=78). Trust `pcodes.h`.

## Don't assume a construct runs

Only 46 of the 105 defined pcodes are implemented in `makePCode()`. The remaining 59 fall through
to `pcodeNotImpl`, which prints `not impl!` and throws `domain_error`. These compile without
errors and then blow up at runtime: `import`/`from`, `class`/`end`/`end class`, `abstract`,
`event`/`emitevent`, `try`/`catch`/`raise`, `pre`/`post`/`assert`, float/date/time/tuple constants
and types, `is`, the `and`/`or` pcodes, `P_IF_TRUE`, and slice/`LOADX`/`STOREX`. The grammar also
emits `P_COMMENT` (`"da fare ..."`) for array/dict comprehensions — likewise unimplemented.

The only builtin is `hello` (registered through the `BUILTIN()` macro in `interpreter.cpp`).

## Tests

No runner. Verification is manual: `./compiler test/x.cpl` then `./asm < test/x.pcd` and diff the
listing against the expected block. Several fixtures in `test/` embed their **expected p-code as a
comment block inside the same file** (`test/simple_class.cpl` is the clearest example) — that's the
intended check. `test_expr.txt` is a scratchpad of cpl2 snippets by feature plus a commented-out
C++ reference block; it has no expected outputs.

## Git hygiene

- **There is no `.gitignore`,** so build output sits untracked in the root: `compiler`,
  `interpreter`, `asm`, `tt`, `interpreter.o`, `cpl2.out`, `*.pcd`, `*.out`, plus `tmp/`,
  `gc_test.cpp`, `test/gc_test.cpp`. Never `git add -A` — add sources explicitly.
- The compiler drops each `.pcd` next to its source, so compiling a fixture also litters `test/`
  (`test/simple_class.pcd`, ...). Still build output — don't commit it.
- `tmp/` holds scratch copies of `gc.cpp`/`interpreter.cpp`. Ignore it.
- HEAD is currently detached; local branches `main` and `with_GC` mirror origin.
