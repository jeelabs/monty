# Conventions

## Directory layout
```
├── docs/               # this documentation
├── examples/
│   ├── blinker/        # minimal stacklet demo, see the README
│   └── ...
├── lib/
│   ├── arch-native/    # only included in native builds
│   ├── arch-stm32/     # only included in ... yeah, STM32 builds
│   ├── monty/          # main code area for Monty
│   └── ...
├── pytests/            # Python tests for native & remote use
├── src/                # small main.cpp and some utility scripts
└── test/               # Unity C++ tests for native & remote use
    └── ...
```

[MPY]: https://micropython.org/

## Code formatting

* no braces around single statements
* opening brace at end of previous line
* space after if, while, etc keywords
* empty body in for/while is written as {}, not ; and placed on same line
* space after comma, not after ( or before )
* space after name in **declarations** of functions and arrays
* no space after name in their **actual use**, i.e. in calls and indexing
* `template` is placed on a separate line, before the class/function
* only "auto" or "void" in front of function decl, return type placed after `->`
* `const` comes after the type, aka the "east const" convention
* pointer declarations are written as `int* v` iso `int *v`
* short methods are defined inline, without adding "inline" keyword
* local functions defined as static, this helps the compiler optimise
* a `struct` is a class with public visibility, no need for "class ... public:"
* hard tabs set to 8, indents set to 4
* line lengths limited to 80 where possible
* comments use `// ...`, preferred over /\*...\*/

## Code generation

Some parts of the source code in Monty are generated using the `src/codegen.py`
script, which is exclusively tailored for Monty.  Code generation is done by
scanning all source code at the start of each build and _replacing_ certain
markers with _automatically_ generated results. These makers start with the text
`//CG`. There are a few variants:

* `//CG tag ...` - this is a newly-entered source line which has never been
  processed
* `//CG: tag ...` - the same, but now the code generator has seen it (and
  presumably saved the request for other parts of its processing)
* `//CG1 tag ...` to `//CG3 tag ...` - this line and the next 1..3 lines go
  together, the code generator "owns" those next lines, and changes them as
  needed
* `//CG< tag ...` - this marks a range owned by the code generator, which runs
  until a line with `//CG>` is seen

Note that "owned by" means that the text will be **replaced** by the code
generator as it sees fit. The text inside these ranges should not be edited
manually. At best, it breaks code generation - at worst, your changes will be
lost.

Code generation is used for a wide range of tasks, from shorthand for repetitive
expansions that would be cumbersome to write by hand, to parsing external
headers and transforming them to the format needed in Monty. Another very common
use is to collect information about the code in various places, and then
_generate_ associated code elsewhere, such as dispatch tables for several
classes used all over Monty.

Qstr IDs are also assigned here: the code generator will identify **every**
occurrence of the form `Q(<NNN>,"...")` and replace `<NNN>` with a unique id.
Then, in `qstr.cpp`, all these ids and strings are dumped in `VaryVec` format.

See Invoke's `tasks.py` for details about how and when code generation is
triggered. Do not run `codegen.py` manually, as it wants its arguments in a
specific order - use `inv gen` for this instead.

## Git and GitHub

Some quick notes about the workflow used for collaboration and new features:

* all the code is in `git`, and located at <https://github.com/jeelabs/monty>
* the most stable code is in the default branch, i.e. `main`
* new code is developed in separate branches, with a suitably-chosen name
* when pushed to GitHub, a CI "run" is automatically started for basic testing
* to consider merging this into main, a pull request (PR) is created on GitHub
* the PR's title is the default commit message body, but can be adjusted later
* further commits to the associated branch automatically get added to the PR
* `git rebase -i ...` should be used to rename / squash / fixup commits
* for rebases to commits already on GitHub, `git push -f` is unavoidable
* this PR branch must be rebased to main/HEAD if other merges have occurred
* when ready for merging, the commits in the PR should be clean w/ good titles
* merging into `main` is always done on GitHub, using the web interface
* _at this point_, the comment must be adusted to describe the PR's main purpose

![](github-commit.png ':size=50%x')

* the last step is a button titled **"Confirm merge"** - click, and that's it!
* since auto-delete is enabled, the associated PR branch will also be deleted
* after this point, the PR can be found in the "Closed" section and via its #id
