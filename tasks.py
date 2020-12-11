# see https://www.pyinvoke.org
from invoke import task

@task
def zero(c):
    """delete all build results"""
    c.run("rm -rf .pio monty.bin syms-*.ld")

@task
def gen(c):
    """process source code with code generator"""
    c.run("util/codegen.py qstr.h lib/monty/ qstr.cpp")

@task(gen)
def native(c):
    """run a script using the native build"""
    c.run("pio run -e native -s")
    c.run(".pio/build/native/program")

@task
def embed(c):
    """run a script using the embedded µC build (NOTYET)"""

@task
def test(c):
    """run C++ tests natively"""
    c.run("pio test -e native", pty=True)

@task
def utest(c):
    """run C++ tests, uploaded to µC"""
    c.run("pio test -e utest", pty=True)

@task
def devs(c):
    """build and upload the DEVS layer"""
    c.run("pio run -e devs -t upload -s")

@task(post=[devs])
def core(c):
    """build and upload the CORE and DEVS layers"""
    c.run("pio run -e core -t upload -s")

@task(post=[core])
def boot(c):
    """build and upload the BOOT, CORE, and DEVS layers"""
    c.run("pio run -e boot -t upload -s")

@task
def mrfs(c):
    """generate the Minimal Replaceable File Storage image"""
    c.run("util/mrfs.py -o rom.mrfs valid/*.py")

@task(zero, mrfs)
def final(c):
    """merge all layers + MRFS into a monty.bin image"""
    buildLayers(c)
    c.run("cat .pio/build/{boot,core,devs}/firmware.bin rom.mrfs >monty.bin")
    c.run("ls -l monty.bin")

@task
def python(c):
    """run Python tests natively (NOTYET)"""

@task
def rpython(c):
    """run Python tests, sent to remote µC (NOTYET)"""

@task(test, python, utest, rpython)
def all(c):
    """shorthand for running: test python utest rpython"""

@task(final)
def wipe(c):
    """erase flash and install a complete fresh image (NOTYET)"""

@task
def health(c):
    """check to verify proper toolchain setup"""
    c.run("uname -sm")
    c.run("python3 --version")
    c.run("inv --version")
    c.run("pio --version")
    c.run("mpy-cross --version")
    #c.run("which micropython || echo NOT FOUND: micropython")
    c.run("monty-conv -v")

@task
def info(c):
    """show some information about layers"""
    buildLayers(c)
    c.run("arm-none-eabi-size .pio/build/????/firmware.elf")
    c.run("tail -n4 syms-*.ld")

@task
def version(c):
    """show git repository version"""
    c.run("git describe --tags")

@task
def serial(c):
    """serial terminal session, use in separate window"""
    c.run("pio device monitor -b115200", pty=True)

def buildLayers(c):
    print("Updating layer builds")
    c.run("pio run -e boot -e core -e devs -s")
