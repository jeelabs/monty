# see https://www.pyinvoke.org
from invoke import exceptions, task

import io, os, subprocess
from src.runner import compileIfOutdated, compareWithExpected

os.environ["MONTY_VERSION"] = subprocess.getoutput("git describe --tags")

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
def native(c, file=""):
    """run a script using the native build"""
    c.run("pio run -e native -s", pty=True)
    cmd = ".pio/build/native/program"
    if file:
        cmd += " " + compileIfOutdated(file)
    c.run(cmd, pty=True)

@task
def test(c):
    """run C++ tests natively"""
    c.run("pio test -e native", pty=True)

@task
def python(c):
    """run Python tests natively"""
    c.run("pio run -e native -s", pty=True)
    num, fail, match = 0, 0, 0

    tests = os.listdir("valid")
    tests.sort()
    for file in tests:
        if file[-3:] == ".py":
            num += 1
            try:
                py = "valid/" + file
                mpy = compileIfOutdated(py)
                out = io.StringIO()
                c.run(".pio/build/native/program %s" % mpy, out_stream=out)
                if compareWithExpected(py, out.getvalue()):
                    match += 1
            except exceptions.UnexpectedExit:
                fail += 1

    print(f"\n{num} tests, {match} matches, {fail} failures")

@task(x_codegen)
def embed(c):
    """embedded build and upload to µC"""
    c.run("pio run -s", pty=True)

@task
def upload(c):
    """run C++ tests, uploaded to µC"""
    c.run("pio test", pty=True)

@task(embed)
def runner(c):
    """run Python tests, uploaded to µC"""
    c.run("src/runner.py valid/*.py", pty=True)

@task(test, python, upload, runner)
def all(c):
    """shorthand for running test, python, upload, and runner"""

@task
def mrfs(c):
    """generate Minimal Replaceable File Storage image"""
    c.run("src/mrfs.py -o rom.mrfs valid/*.py")

@task
def clean(c):
    """delete all build results"""
    c.run("rm -rf .pio monty.bin")

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
