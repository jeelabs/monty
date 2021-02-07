# see https://www.pyinvoke.org
from invoke import task

import io, os, subprocess, sys
from src.runner import compileIfOutdated, compareWithExpected, printSeparator

os.environ["MONTY_VERSION"] = subprocess.getoutput("git describe --tags --always")

@task
def x_codegen(c):
    """process sources with code generator"""
    c.run("src/codegen.py qstr.h lib/monty/ qstr.cpp")

@task
def x_examples(c):
    """build each of the example projects"""
    examples = os.listdir("examples")
    examples.sort()
    for ex in examples:
        if os.path.isdir("examples/" + ex):
            print(ex)
            c.run("pio run -d examples/%s -t size -s" % ex, warn=True)

@task(help={"host": "hostname for rsync (required)",
            "dest": "destination directory (default: monty-test)"})
def x_rsync(c, host, dest="monty-test"):
    """copy this entire area to the specified host"""
    c.run("rsync -av --exclude .git --exclude .pio . %s:%s/" % (host, dest))

@task
def x_sizes(c):
    """show µC build sizes, w/ and w/o assertions"""
    c.run("pio run -t size | tail -8 | head -2")
    c.run("pio run -e noassert | tail -7 | head -1")

@task
def x_tags(c):
    """update the (c)tags file"""
    c.run("ctags -R lib src test")

@task
def x_version(c):
    """show git repository version"""
    print(os.environ["MONTY_VERSION"])

@task(x_codegen, help={"file": "name of the .py or .mpy file to run"})
def native(c, file="pytests/hello.py"):
    """run script using the native build  [pytests/hello.py]"""
    c.run("pio run -e native -s", pty=True)
    cmd = ".pio/build/native/program"
    if file:
        cmd += " " + compileIfOutdated(file)
    c.run(cmd)

@task
def test(c):
    """run C++ tests natively"""
    c.run("pio test -e native", pty=True)

@task(native,help={"tests": "specific tests to run, comma-separated"})
def python(c, tests=""):
    """run Python tests natively          [in pytests/: {*}.py]"""
    num, fail, match = 0, 0, 0

    if tests:
        files = [t + ".py" for t in tests.split(",")]
    else:
        files = os.listdir("pytests")
        files.sort()
    for file in files:
        if file[-3:] == ".py":
            num += 1
            py = "pytests/" + file
            try:
                mpy = compileIfOutdated(py)
            except FileNotFoundError as e:
                printSeparator(py, e)
                fail += 1
            else:
                try:
                    r = c.run(".pio/build/native/program %s" % mpy,
                              timeout=2.5, hide=True)
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

    print(f"{num} tests, {match} matches, {fail} failures")

@task(x_codegen)
def flash(c):
    """build embedded and re-flash attached µC"""
    try:
        # only show output if there is a problem, not the std openocd chatter
        r = c.run("pio run", hide=True, pty=True)
        print(r.stdout.split("\n", 1)[0]) # ... but do show the target info
    except Exception as e:
        # captured as stdout due to pty, which allows pio to colourise
        print(e.result.stdout, end="", file=sys.stderr)
        sys.exit(1)

@task
def upload(c):
    """run C++ tests, uploaded to µC"""
    c.run("pio test", pty=True)

@task(help={"tests": "specific tests to run, comma-separated"})
def runner(c, tests=""):
    """run Python tests, sent to µC       [in pytests/: {*}.py]"""
    match = "{%s}" % tests if "," in tests else (tests or "*")
    c.run("src/runner.py pytests/%s.py" % match, pty=True)

@task(test, python, upload, flash, runner)
def all(c):
    """shorthand for running test, python, upload, flash, and runner"""

@task(help={"addr": "flash address (default: 0x08010000)",
            "write": "also write to flash using st-flash"})
def mrfs(c, addr="0x08010000", write=False):
    """create Minimal Replaceable File Storage image [rom.mrfs]"""
    c.run(f"src/mrfs.py -o rom.mrfs pytests/*.py")
    #c.run("cd lib/mrfs/ && g++ -std=c++11 -DTEST -o mrfs mrfs.cpp")
    #c.run("lib/mrfs/mrfs wipe && lib/mrfs/mrfs save pytests/*.mpy" )
    if write:
        try:
            r = c.run(f"st-flash write rom.mrfs {addr}", hide="both")
            print(r.tail("stdout", 1).strip())
        except Exception as e:
            r = e.result
        print(r.tail("stderr", 1).strip())

@task
def clean(c):
    """delete all build results"""
    c.run("rm -rf .pio examples/*/.pio monty.bin")

@task
def health(c):
    """verify proper toolchain setup"""
    c.run("uname -sm")
    c.run("python3 --version")
    c.run("inv --version")
    c.run("pio --version")
    c.run("mpy-cross --version")
    #c.run("which micropython || echo NOT FOUND: micropython")

@task
def serial(c):
    """serial terminal session, use in separate window"""
    c.run("pio device monitor -b115200", pty=True)
