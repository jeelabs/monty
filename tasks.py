# see https://www.pyinvoke.org
from invoke import task

@task
def clean(c):
    """delete all build results"""
    c.run("rm -rf .pio monty.bin")

@task
def gen(c):
    """process source code with code generator"""
    c.run("src/codegen.py qstr.h lib/monty/ qstr.cpp")

@task(gen)
def native(c):
    """run a script using the native build"""
    c.run("pio run -e native -s", pty=True)
    c.run(".pio/build/native/program", pty=True)

@task
def test(c):
    """run C++ tests natively"""
    c.run("pio test -e native", pty=True)

@task
def utest(c):
    """run C++ tests, uploaded to µC"""
    c.run("pio test -e utest", pty=True)

@task
def boot(c):
    """build and upload the BOOT, CORE, and DEVS layers"""
    c.run("pio run -s")

@task
def mrfs(c):
    """generate the Minimal Replaceable File Storage image"""
    c.run("src/mrfs.py -o rom.mrfs valid/*.py")

@task
def all(c):
    """shorthand for running: test python utest rpython (NOTYET)"""

@task
def health(c):
    """check to verify proper toolchain setup"""
    c.run("uname -sm")
    c.run("python3 --version")
    c.run("inv --version")
    c.run("pio --version")
    c.run("mpy-cross --version")
    #c.run("which micropython || echo NOT FOUND: micropython")

@task
def version(c):
    """show git repository version"""
    c.run("git describe --tags")

@task
def serial(c):
    """serial terminal session, use in separate window"""
    c.run("pio device monitor -b115200", pty=True)
