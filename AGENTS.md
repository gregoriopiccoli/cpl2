# AGENTS.md

Compiler + interpreter for a new language (`cpl2`). Flat single project: no packages, no CI,
no README, no formatters. Sources at the repo root; code comments, grammar notes and commit
messages are in Italian — keep them that way.

## Build

```sh
make                # -> interpreter, compiler, asm, runtest   (no clean/test target)
```

- `interpreter` = `c++ -g -O2 interpreter.cpp` (~5s, needs C++)
- `compiler`    = `cc parser.c`
- `asm`         = `cc asm.c`
- `runtest`     = `cc runtest.c` (the test runner, see "Tests")
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
./asm fib.pcd                         # binary p-code -> readable text (inspect only)
./asm fib                             # name with no dot: tries fib, then fib.pcd
./asm < fib.pcd                       # same, reading the p-code from stdin
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
- The emit functions don't write straight to `emit_out`: they append to an in-memory buffer that
  `parse()` writes out with `emit_flush()` at the end of each source. `emit_pos()` and
  `emit_rotate(from,mid)` let a grammar action move already-emitted p-code (the array
  comprehension uses it). Anything that writes to `emit_out` directly would land out of order.
- `.pcd` header must be exactly `#cpl20.0.1` — hardcoded in `parse()` (`parser.c`) and again in
  `pcodeProgram::loadPcd` (`interpreter.cpp`). Bump both together.
- File format: one p-code per line, first char = `pcode + 31`, rest of line = operand (int or
  string). Note `P_PLUS` (1) encodes as a leading space.
- Last line must be `PCODEEND 0`, otherwise `loadPcd` prints `<file> has errors!` and runs nothing.
- `emit_asm` is hardcoded to `0` in `parse()`; `./asm` is the only way to get readable p-code.

## `asm` is a checker, not a pipeline stage

`asm.c` is only there to eyeball a `.pcd`: `./asm fib.pcd` prints the readable listing. It takes
**at most one path** and reads from it instead of stdin; with no argument (or the lone `-`) the old
`./asm < fib.pcd` pipeline still works, and the listing goes to stdout in both modes — `asm` never
writes a file. It declares no options, so `./asm -b fib.pcd` reports `-b` as unknown on stderr and
exits 1; `--` ends option recognition, so `./asm -- -file.pcd` reads the file named `-file.pcd`. A
path it cannot open is reported on stderr and exits 1, naming the path **as typed**. As a second
chance — like the interpreter's D2 rule, but as a fallback and not a derivation — when the typed
path doesn't exist (ENOENT only) and its base name carries no dot, `asm` retries the same path with
`.pcd` appended; the dot is looked for **in the base name only**, so `dir.d/x` still tries
`dir.d/x.pcd`. An existing typed path always wins: `./asm x` reads `x`, never `x.pcd`.
It still contains the reverse pass (readable ->
binary, keyed off the first line) and that round trip is byte-exact today, but nothing needs it — the
interpreter reads the binary `.pcd` the compiler just wrote, so don't build anything on top of it.
The conversion exit codes are unchanged from the stdin-only days: 2 = binary -> listing, 1 = listing
-> binary, 0 = input that is neither.

## Running a program — one path, plus options

`./interpreter` takes **one path**: the p-code to run. There are two flags, each with two spellings:
the benchmark one, written `--bench` or `-b`, and the help one, written `--help` or `-h`. Both may
go **before or after** the path.

```sh
./compiler mio.cpl             # writes mio.pcd next to the source
./interpreter mio.pcd          # single run; the .pcd can be omitted: ./interpreter mio
./interpreter -b mio.pcd       # nanobench loop instead of a single run — ~30-60s for fib(34)
./interpreter --bench mio.pcd  # same thing, long spelling
./interpreter --help           # usage on stdout, exit 0
```

`--help` and `-h` print the usage — the same text as the error path, the options included — on
**stdout** and exit **0**, running and measuring nothing. They win over the path, so
`./interpreter -h mio.pcd` prints the help and ignores `mio.pcd`; they don't win over a malformed
line, so `./interpreter -h -z mio.pcd` still reports `-z` as unknown. Usage printed because of an
error stays on **stderr** with exit **1**.

Neither running nor benchmarking your own file requires editing `main()` any more. The two
functions behind those two lines are:

```cpp
runPcd("mio.pcd");     // single run  (asserts the stack unwound to sp==-1)
benchPcd("mio.pcd");   // nanobench loop around runPcd(), same exit code
```

A name with no extension gets `.pcd` appended, and the dot is looked for **in the base name
only**, so `dir.d/x` stays `dir.d/x.pcd`. A name that already has an extension is used verbatim:
`./interpreter fib.cpl` tries to load `fib.cpl` and says it is not compiled p-code.

Options are recognized anywhere on the line, long (`--bench`, `--help`) or short (`-b`, `-h`),
and `--` ends option recognition so everything after it is a path. An argument starting with `-`
that is not the lone `-` is an **option**, not a path: an unrecognized one is reported as unknown
on stderr and nothing runs — that includes `-bench`, the old single-dash spelling that no longer
exists (`./interpreter -bench mio.pcd` is an error now). `./interpreter -b -- mio.pcd` measures
`mio.pcd`. A path that really starts with `-` needs `./` (`./interpreter ./-b`) or the `--`
terminator.

A file that can't be opened, isn't compiled p-code, or has compile errors prints the reason on
stderr and exits 1 without running anything. The same holds with the benchmark flag: `benchPcd()`
loads the file once before measuring, so an unloadable file prints no nanobench table. `main()` does
**not** print the GC report by default: it is behind `--gc`/`-g` (`if (gcOpzione) { ... }`, just
before `return rc`), and without that option the standard output is only the program's. With `--gc`
it is printed afterwards in both modes, so `> out.txt` then captures the program output, the
nanobench table *and* that report.

Open work, see `todo.md`. Plain `./interpreter` is **not** a smoke test: with no arguments, with
more than one path, and with the benchmark flag and no path it prints usage on stderr and exits 1
without running or measuring anything.

**Known bug, not part of any change:** `benchPcd()` ends with `return r?1:0` where `runPcd()` uses
`return r?0:1`, so `-b` exits **1** on a successful measurement and **0** on an unloadable
file — the inverse of what `interpreter-cli` requires. Fixing it is a separate change.

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
  `emit.c`. `emit.c` and `pcodes.c` are shared with the interpreter. The `for each` loop head is
  emitted by `foreachTesta()` in `parser.c`, shared by the statement and both comprehensions.
- `interpreter.cpp` `#include`s `gc.cpp` (generational mark & sweep; tunables at the top of
  `gc.cpp`) and `pcodes.h`/`pcodes.c`. `nanobench.h` is vendored.
- `cli.c` is shared by **both** binaries and is `#include`d into the two translation units, the
  same way `pcodes.c` and `emit.c` are. It holds only the command-line scan loop: it knows the
  *shape* of an option (long name, short name, single-dash alias, with or without value) and
  nothing about either tool's vocabulary. Each tool declares its own `cliOpzione` table —
  `interpreter.cpp` has `--bench`/`-b` and `--help`/`-h`, `parser.c` has an intentionally empty one until
  the first compiler flag exists. Adding a flag means adding a row to that table; the usage text
  is generated from it via `cliOpzioni()`, so it never needs a second edit. It must stay valid C
  **and** C++ (`interpreter.cpp` is C++, `parser.c` is C), and it never allocates: the caller
  supplies the options table and the positionals array.
- An unknown option is an error, not a path: `./interpreter -h` and `compiler -x fib.cpl` report
  it on stderr and run nothing. The compiler stops the whole invocation for it (unlike an
  unreadable source, which is counted and skipped); the interpreter prints usage and exits 1.
- `gc_test.cpp` is standalone (its own `GCObject` copy and `main`), not in the Makefile:
  `c++ -g -O2 gc_test.cpp -o gc_test && ./gc_test`. `cli_test.c` is standalone in the same way and
  is the only place the option-with-value form is exercised, since neither binary declares one:
  `cc -g -O2 cli_test.c -o cli_test && ./cli_test` (add `-x c++` to compile it as C++ and prove
  the dual-language constraint). `test/gc_test.cpp` is a different, untracked variant — don't
  confuse them.

## Adding a p-code: four coordinated edits

1. `pcodes.h` — `#define P_XXX n`; bump `MAX_PCODE` if it's now the highest (used by `asm.c`'s
   `FindPCode` loop bound).
2. `pcodes.c` — add `pcodetxt[P_XXX]="XXX"` in `initpcodetxt()`.
3. `cpl2.gram` — emit it, then regenerate `cpl2.c`/`cpl2.h`.
4. `interpreter.cpp` — a `pcodeXXX` class overriding `exec()` **and** a `case P_XXX:` in
   `makePCode()`. Only add it to `pcodejump[]` if it is genuinely hot.

The Makefile rule is `asm: asm.c` only, so `make` does **not** rebuild `asm` after a `pcodes.h`/
`pcodes.c` edit: `touch asm.c && make`, or the stale `asm` turns the new mnemonics into pcode 0.

The trailing `// 78` numeric comments in `pcodes.c` are hand-written and already stale — two
entries have the values swapped (`P_ABSTRACT`=85, `P_EVENT`=78). Trust `pcodes.h`.

## Don't assume a construct runs

Only 73 of the 116 defined pcodes are implemented in `makePCode()`. The remaining 43 fall through
to `pcodeNotImpl`, which prints `not impl!` and throws `domain_error`. These compile without
errors and then blow up at runtime: `from`/`FROM`, `class`/`end`/`end class`, `abstract`,
`event`/`emitevent`, `try`/`catch`/`raise`, `pre`/`post`/`assert`, float/date/time/tuple constants
and types, `is`, the `and`/`or` pcodes, `P_IF_TRUE`, and slice.

`switch` runs. The selector is evaluated once and stays on the stack; each `case` constant is
`<const>` + `CASE Lbody` (pops the constant, jumps if equal, keeps the selector; a constant of a
different type, or a `nil` selector, simply doesn't match), then `GOTO Lnext`; every body and the
`else` (implicit when absent) start with `POP`, every body ends with `GOTO Lend`. No fall-through,
first matching `case` wins. `case -1` emits a single `INT_CONST -1`, not `UMINUS`; `case +1`
emits plain `INT_CONST 1` (unary plus emits no p-code anywhere; in a `case` only an integer
may follow it).

`for` runs (`to`/`downto`, with or without `step`). Start, end and step are evaluated once, in that
order (a missing step is `INT_CONST 1` for `to`, `INT_CONST -1` for `downto`), then
`FOR_TO i`/`FOR_DOWNTO i` checks that `i` is declared **in the current context** (not just visible
from a superlevel — the `for` never declares it), that all three are `int` and that the step sign
matches the direction (step 0 is an error), stores the start in `i` and leaves end and step on the
stack. The loop is `LABEL Ltest`, `LOAD i`, `FOR_TEST Lend` (pops the value, jumps out once it is
past the end in the step's direction), body, `FOR_NEXT i` (`i:=i+step`), `GOTO Ltest`, `LABEL Lend`,
`POP`, `POP`. `i` is re-read every iteration, so assigning it in the body affects the loop; after the
loop it holds the first value past the end. Errors print on stdout and throw `domain_error`. Any
future statement that leaves a loop early (`break`, `return`) must pop end and step itself.

`for each` runs on arrays, dicts and tuples (`for each v[,k] in x [with cond]`). The collection is
evaluated once and `FOREACH_INIT` replaces it on the stack with an internal iterator;
`FOREACH_VAR v` / `FOREACH_KEY k` check that each name is declared **in the current context** (same
rule as `for`) and register it in the iterator; the loop is `LABEL Ltest`, `FOREACH_NEXT Lend`
(jumps out when finished, otherwise stores value and key straight into the context, with the
`--typecheck` rules of `STORE`), `[<cond> IF_FALSE Ltest]` for `with` (a filter), body,
`GOTO Ltest`, `LABEL Lend`, `POP`. Array keys are the 0-based `int` index; tuples have no key
(`FOREACH_KEY` on a tuple is an error); dict keys are always `str` (the dict stores `print()` of
the key) and the **dict visiting order is unspecified** — the hash map's. The dict's keys are copied
when the loop starts, so keys added in the body are not visited while values are read at visit
time; arrays are visited live by position. Any other value is a runtime error. A future
`break`/`return` must pop the iterator too.

Array comprehensions run (`[e for each v[,k] in x [with cond]]`) and reuse the `for each` loop:
`ARRAY 0` (the result), the collection, the same loop head, `[<cond> IF_FALSE Ltest]`, `<e>`,
`COMPR_APPEND` (appends the top value to the array under the iterator and pops it), `GOTO Ltest`,
`LABEL Lend`, `POP` — the result stays on the stack. The parser emits `e` before the collection,
so the `array_compr` action moves its p-code into the loop with `emit_rotate()`; the start is
marked by the `lsq` nonterminal shared with `array_const`. Variables follow the `for each` rules
(declared in the current context, keep their last value); the result is an untyped array.

Dict comprehensions run in two forms, both with exactly two variables (the one-variable rules are
commented out in `cpl2.gram`, so `{k:v for each x in a}` is a syntax error):
`{ke:ve for each v,k in x [with cond]}` (value and key of the `for each` on `x`; a tuple is an
error, it has no key) and the parallel form `{ke:ve for each v:w in A : B [with cond]}` (`v` gets
the elements of `A`, `w` those of `B`, position by position). Same shape as the array
comprehension: `DICT 0`, the collection(s), the loop head, `[<cond> IF_FALSE Ltest]`, `<ke>`,
`<ve>`, `COMPR_APPEND 2` (stores the pair into the dict under the iterator via `storeslice`, so
the key is `print()`ed like in `DICT n` and a repeated key keeps the last value), `GOTO Ltest`,
`LABEL Lend`, `POP`. The start is marked by `lbr`, shared with `dict_const`. No new pcode: the
parallel form emits `FOREACH_INIT 2`, which replaces the two collections on top with one
`zipIter` whose value comes from `A` and whose "key" is the element of `B`, so
`FOREACH_VAR`/`FOREACH_KEY`/`FOREACH_NEXT` are unchanged; when only one of the two runs out,
`FOREACH_NEXT` raises a runtime error (different lengths). Without an operand both
`FOREACH_INIT` and `COMPR_APPEND` keep their old meaning, so existing p-code is byte-identical.

`import` without `from` loads and executes modules: the dotted name is mapped to a `.pcd` path
by replacing `.` with `/` and appending `.pcd`, searched relative to the directory of the p-code
that contains the `import` (and propagated to imported modules). Each module is loaded once and
cached in `sys`; it runs in its own context with `theBuiltIn` as superlevel, locked for the
process lifetime. `IMPORT` validates the N string names on top of the stack, calls `loadModule`
for each in order from bottom to top, removes them without printing, and raises `domain_error`
if loading fails. After a successful load, `IMPORT` also adds to the importing context one
symbol named after the last dotted component (`q1` for `q1`, `x` for `a.x`) whose value is the
module context; first-wins if the name already exists, and no module name is copied unqualified.
Module names are read and written through it (`q1.i`, `a.x:=5`) via `LOADX`/`STOREX`, which are
now implemented. `from ... import ...` still ends in `P_FROM` and `pcodeNotImpl`.

The only builtin is `hello` (registered through the `BUILTIN()` macro in `interpreter.cpp`).

## Tests

`runtest` is the runner for the fixtures in `test/` (and for any other directory you point it at).
Every `.cpl` in the directory it scans is a test, in one of two forms:

- `<name>.cpl` + `<name>.out` is a **full test**: `runtest` compiles the `.cpl` with the `compiler`
  binary, runs the resulting p-code with the `interpreter` binary, captures the **standard output**
  of that single run and compares it with the `.out` **byte for byte** — no trimming, no whitespace
  tolerance, no normalisation. Identical output passes, anything else fails.
- `<name>.cpl` with **no** `<name>.out` is a **compile-only test**: `runtest` compiles it with
  `compiler` and that is the whole test — it is never run through `interpreter`. Exit 0 from the
  compiler counts it as passed, anything else counts it as failed (named on stderr like any other
  failure). This is how a source whose pcodes aren't implemented yet (`simple_class.cpl`'s `class`,
  `simple_import.cpl`'s `import`) still gets checked.

Both forms are executed the same way otherwise: the source is copied to a temporary directory
first, so a compile-only test leaves no `.pcd` in the test directory either.

```sh
make && ./runtest              # run every test in test/  (the default directory)
./runtest tmp/prove            # run the tests of another directory (at most one, no recursion)
./runtest > /dev/null          # the report is on stderr, so the exit code still tells the truth
```

- The convention is the **only** definition of a test: a `.cpl` with a `.out` beside it is a full
  test, one without is a compile-only test, and an `.out` with no `.cpl` is reported by name and
  counts as a failure.
- Exit code: **0** when every test passed, **1** when at least one failed (or an orphan `.out` was
  found). Every failing test is named on stderr; a full test's differing lines are shown too.
- Nothing is written into the test directory: the `.pcd` and the captured output go to a temporary
  `mkdtemp()` directory, removed at the end of the run even when tests failed. A pre-existing `.pcd`
  in `test/` is neither read nor touched — the p-code a test runs is the one `runtest` just compiled.
- Full tests run before compile-only ones, each group in name order, so two consecutive runs report
  the same outcome. Tests run in normal mode, never with `--bench`, so a nanobench table never
  enters the comparison.
- Full tests always run with **type checking on**: the command is `./interpreter --typecheck`,
  the only option `runtest` passes (no `--bench`, no `--gc`) and there is no switch to turn it
  off. An assignment the check rejects aborts the program with a non-zero exit, so the test fails
  as "esecuzione fallita" before any comparison — which is why the rejection cases stay in the
  compile-only `test/type_check_err.cpl` and are verified by hand.

**`--test`/`-t` is not this suite.** The interpreter's `--test` branch runs a fixed sequence of
interpreter self-checks (`test_cc_translation()`) and measures inside nanobench; it reads no
directory and compares nothing with a `.out`. Keeping the two systems distinct is deliberate (design
D7); merging them is a separate change.

Verification of the **expected p-code block written inside a source file** stays manual, and it is
what `AGENTS.md` describes with `./asm < x.pcd`: several fixtures in `test/` embed their expected
p-code as a comment block inside the same file (`test/simple_class.cpl` is the clearest example).
`runtest` executes programs and compares output; it does not read those comments. `test_expr.txt` is
a scratchpad of cpl2 snippets by feature plus a commented-out C++ reference block; it is not in
`test/`, so `runtest` never sees it.

## Git hygiene

- **There is no `.gitignore`,** so build output sits untracked in the root: `compiler`,
  `interpreter`, `asm`, `runtest`, `tt`, `interpreter.o`, `cpl2.out`, `*.pcd`, `*.out`, plus `tmp/`,
  `gc_test.cpp`, `test/gc_test.cpp`. Never `git add -A` — add sources explicitly.
- The compiler drops each `.pcd` next to its source, so compiling a fixture also litters `test/`
  (`test/simple_class.pcd`, ...). Still build output — don't commit it.
- `tmp/` holds scratch copies of `gc.cpp`/`interpreter.cpp`. Ignore it.
- HEAD is currently detached; local branches `main` and `with_GC` mirror origin.
