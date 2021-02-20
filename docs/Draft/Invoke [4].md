# Using PyInvoke as main workflow

* used a main dev tool, e.g. `inv python` for a quick native test
* see <https://www.pyinvoke.org>
* and <https://interrupt.memfault.com/blog/building-a-cli-for-firmware-projects>
* it's all scripted in Python, see `tasks.py`
* docs are a bit hard to navigate, but it's ok for simple uses

## design
* the `generate` cmd scans `platformio.ini` to pick up the `[codegen]` section
* the `MONTY_VERSION` env var is set to the github version for PIO use
* try to keep each command name unique in the first character
* use `inv --print-completion-script=bash` for command completion setup
* `inv -l` and `inv -h <cmd>` are great self-documenting features

## overview
```text
$ inv -l
Available tasks:

  all         i.e. clean test python upload flash mrfs runner builds examples
  builds      show µC build sizes, w/ and w/o assertions or Python VM
  clean       delete all build results
  examples    build each of the example projects
  flash       build embedded and re-flash attached µC
  generate    pass source files through the code generator
  health      verify proper toolchain setup
  mrfs        upload tests as Minimal Replaceable File Storage image
  native      run script using the native build  [pytests/hello.py]
  python      run Python tests natively          [in pytests/: {*}.py]
  runner      run Python tests, sent to µC       [in pytests/: {*}.py]
  serial      serial terminal session, use in separate window
  test        run C++ tests natively
  upload      run C++ tests, uploaded to µC
  x-rsync     copy this entire area to the specified host
  x-tags      update the (c)tags file
  x-version   show git repository version
```