# see https://www.pyinvoke.org
from invoke import task

@task
def clean(c):
    """delete all build results"""
    c.run("rm -rf .pio monty")

@task
def gen(c):
    """run the code generator"""
    c.run("python3 src/codegen.py qstr.h lib/monty/ qstr.cpp")

@task(gen)
def build(c, config="native"):
    """build one of the supported executables"""
    c.run(f"pio run -c configs/{config}.ini -s")

@task(build)
def run(c):
    """build and run the "features" test natively"""
    c.run(f".pio/build/native/program verify/features.mpy verify/rom.mrfs")

@task
def tags(c):
    """rebuild the "tags" source index file"""
    c.run("ctags -R src/ lib/monty/ test/")

@task(gen)
def test(c, filter="*"):
    """run the C++ unit test suites"""
    c.run(f"pio test -c configs/native.ini -f '{filter}'", pty=True)

@task(build)
def verify(c):
    """run the Python unit test suites"""
    c.run("make -C verify BOARD=native")
