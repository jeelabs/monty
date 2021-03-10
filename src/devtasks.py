# see https://www.pyinvoke.org
from invoke import task, call

import configparser, os, sys
from src.runner import compileIfOutdated, compareWithExpected, printSeparator

dry = "-R" in sys.argv or "--dry" in sys.argv
exe = ".pio/build/native/program" # for native builds

# root must be non-empty if-and-only-if invoked "out of tree"
root = os.environ.get("MONTY_ROOT", "")
if " " in root:
    print("Spaces in path not supported: '%s'" % root)
    sys.exit(1)
if root and root[-1:] != "/":
    root += "/" # yeah, only macos & linux

def inRoot(f):
    return os.path.join(root, f)

def pio(args, *more):
    cmd = args.split(" ", 1)
    if root:
        cmd[1:1] = ["-c", inRoot("platformio.ini")]
    return " ".join(["pio"] + cmd + list(more))

# parse the platformio.ini file and any extra_configs it mentions
cfg = configparser.ConfigParser(interpolation=configparser.ExtendedInterpolation())
cfg.read_dict({"invoke": {}, "codegen": {}, "platformio": {}})
cfg.read(inRoot('platformio.ini'))

for f in cfg["platformio"].get("extra_configs", "").split():
    cfg.read(f)
python_skip = cfg["invoke"].get("python_skip", "").split()
runner_skip = cfg["invoke"].get("runner_skip", "").split()

@task(incrementable=["verbose"],
      help={"verbose": "print some extra debugging output (repeat for more)",
            "strip": "strip most generated code from the source files",
            "norun": "only report the changes, don't write them out"})
def generate(c, strip=False, verbose=False, norun=False):
    """pass source files through the code generator"""
    # construct codegen args from the [codegen] section in platformio.ini
    cmd = [inRoot("src/codegen.py")]
    cmd += strip*["-s"] + verbose*["-v"] + norun*["-n"]
    cmd += ["qstr.h", "lib/monty/"]
    cmd += cfg["codegen"].get("all", "").split()
    cmd += ["builtin.cpp"]
    for k, v in cfg["codegen"].items():
        if k != "all" and v:
            cmd += ["+" + k.upper()]
            cmd += v.split()
    cmd += ["qstr.cpp"]
    c.run(" ".join(cmd))

@task(call(generate, strip=True))
def clean(c):
    """delete all build results"""
    c.run("rm -rf .pio examples/*/.pio kcov-out/ src/__pycache__ verify/*.mpy")

@task(generate, default=not root,
      help={"file": "name of the .py or .mpy file to run"})
def native(c, file="verify/hello.py"):
    """run script using the native build  [verify/hello.py]"""
    c.run(pio("run -e native -s"), pty=True)
    c.run("%s %s" % (exe, compileIfOutdated(file)))

def shortTestOutput(r):
    gen = (s for s in r.tail('stdout', 10).split("\n"))
    for line in gen:
        if line.startswith("==="):
            break
    for line in gen:
        if line:
            print(line)

@task(generate, iterable=["ignore"],
      help={"tests": "specific tests to run, comma-separated",
            "ignore": "one specific test to ignore",
            "coverage": "generate a code coverage report using 'kcov'"})
def python(c, ignore=[], coverage=False, tests=""):
    """run Python tests natively          [in verify/: {*}.py]"""
    c.run(pio("run -e native -s"), pty=True)
    if dry:
        msg = tests or "verify/*.py"
        print("# tasks.py: run and compare each test (%s)" % msg)
        return

    num, match, fail, skip, timeout, cmd = 0, 0, 0, 0, 2.5, [exe]
    if coverage:
        timeout = 60 # just in case
        cmd = ["kcov", "--collect-only", "kcov-out", exe]
        print("generating a code coverage report, this may take several minutes")
        c.run("rm -rf kcov-out")

    if tests:
        files = [t + ".py" for t in tests.split(",")]
    else:
        files = os.listdir("verify/")
        files.sort()
    for fn in files:
        if fn.endswith(".py") and fn[:-3] not in ignore:
            if not tests and fn[1:2] == "_" and fn[0] != 'n':
                skip += 1
                continue # skip non-native tests
            num += 1
            py = "verify/" + fn
            try:
                mpy = compileIfOutdated(py)
            except FileNotFoundError as e:
                printSeparator(py, e)
                fail += 1
            else:
                try:
                    r = c.run(" ".join(cmd + [mpy]), timeout=timeout, hide=True)
                except Exception as e:
                    r = e.result
                    msg = "[...]\n%s\n" % r.tail('stdout', 5).strip("\n")
                    if r.stderr:
                        msg += r.stderr.strip()
                    else:
                        msg += "Unexpected exit: %d" % r.return_code
                    printSeparator(py, msg)
                    fail += 1
                else:
                    if compareWithExpected(py, r.stdout):
                        match += 1

    if not dry:
        print(f"{num} tests, {match} matches, "
              f"{fail} failures, {skip} skipped, {len(ignore)} ignored")
    if coverage:
        c.run(" ".join(["kcov", "--report-only", "kcov-out", exe]))
        print("the coverage report is ready, see kcov-out/index.html")
    if num != match:
        c.run("exit 1") # yuck, force an error ...

@task(generate, help={"filter": 'filter tests by name (default: "*")'})
def test(c, filter='*'):
    """run C++ tests natively"""
    try:
        r = c.run(pio("test -e native -f '%s'" % filter),
                              hide='stdout', pty=True)
    except Exception as e:
        print(e.result.stdout)
    else:
        shortTestOutput(r)

@task(generate)
def flash(c):
    """build embedded and re-flash attached µC"""
    try:
        # only show output if there is a problem, not the std openocd chatter
        r = c.run(pio("run"), hide=True, pty=True)
        out = r.stdout.split("\n", 1)[0]
        if out:
            print(out) # ... but do show the target info
    except Exception as e:
        # captured as stdout due to pty, which allows pio to colourise
        print(e.result.stdout, end="", file=sys.stderr)
        sys.exit(1)

@task(help={"filter": 'filter tests by name (default: "*")'})
def upload(c, filter="*"):
    """run C++ tests, uploaded to µC"""
    try:
        r = c.run(pio("test -f '%s'" % filter), hide='stdout', pty=True)
    except Exception as e:
        print(e.result.stdout)
    else:
        shortTestOutput(r)

@task(iterable=["ignore"],
      help={"tests": "specific tests to run, comma-separated",
            "ignore": "one specific test to ignore"})
def runner(c, ignore=[], tests=""):
    """run Python tests, sent to µC       [in verify/: {*}.py]"""
    match = "{%s}" % tests if "," in tests else (tests or "*")
    iflag = ""
    if ignore:
        iflag = "-i " + ",".join(ignore)
    cmd = [inRoot("src/runner.py"), iflag, "verify/%s.py" % match]
    c.run(" ".join(cmd), pty=True)

@task(help={"offset": "flash offset (default: 0x0)",
            "file": "save to file instead of uploading to flash"})
def mrfs(c, offset=0, file=""):
    """upload tests as Minimal Replaceable File Storage image"""
    #c.run("cd lib/mrfs/ && g++ -std=c++17 -DTEST -o mrfs mrfs.cpp")
    #c.run("lib/mrfs/mrfs wipe && lib/mrfs/mrfs save verify/*.mpy" )
    mrfs = inRoot("src/mrfs.py")
    if file:
        c.run("%s -o %s verify/*.py" % (mrfs, file))
    else:
        c.run("%s -u %s verify/*.py" % (mrfs, offset), pty=True)

@task
def serial(c):
    """serial terminal session, use in separate window"""
    if not dry:
        print("--- Quit: Ctrl-C | Help: Ctrl-T + Ctrl-H ---")
    if root:
        c.run("pio device monitor -d %s --echo --quiet" % root, pty=True)
    else:
        c.run("pio device monitor --echo --quiet", pty=True)

@task(help={"file": "the Python script to send whenever it changes",
            "remote": "run script remotely iso natively"})
def watch(c, file, remote=False):
    """watch-exec/upload-print loop for quick Python iteration"""
    cmd = [inRoot("src/watcher.py")] + remote*["-r"] + [file]
    c.run(" ".join(cmd), pty=True)
