# see https://www.pyinvoke.org
from invoke import task

@task
def zero(c):
    """delete all build results"""
    c.run("rm -rf .pio monty.bin syms-*.ld")

@task
def native(c):
    """run a script using the native build"""
    c.run("pio run -e native -s")
    c.run(".pio/build/native/program")

@task
def embed(c):
    """run a script using the embedded µC build (NOTYET)"""

@task
def test(c):
    """run the native C++ tests"""
    c.run("pio test -e native", pty=True)

@task
def utest(c):
    """run the uploaded µC C++ tests"""
    c.run("pio test -e utest", pty=True)

@task
def devs(c):
    """build and upload the DEVS segment"""
    c.run("pio run -e devs -t upload -s")

@task(post=[devs])
def core(c):
    """build and upload the CORE and DEVS segments"""
    c.run("pio run -e core -t upload -s")

@task(post=[core])
def boot(c):
    """build and upload the BOOT, CORE, and DEVS segments"""
    c.run("pio run -e boot -t upload -s")

@task
def mrfs(c):
    """generate the Minimal Replaceable File Storage image (NOTYET)"""

@task(mrfs)
def final(c):
    """merge all segments + MRFS into a monty.bin image"""
    buildSegments(c)
    c.run("cat .pio/build/{boot,core,devs}/firmware.bin >monty.bin")
    c.run("ls -l monty.bin")

@task
def python(c):
    """run the native Python tests (NOTYET)"""

@task
def rpython(c):
    """run the remote µC Python tests (NOTYET)"""

@task(test, python, utest, rpython)
def all(c):
    """shorthand for running: test python utest rpython"""

@task(final)
def wipe(c):
    """erase flash and install a complete fresh image (NOTYET)"""

@task
def health(c):
    """health check to verify proper toolchain setup (NOTYET)"""

@task
def info(c):
    """show some information about segments"""
    buildSegments(c)
    c.run("arm-none-eabi-size .pio/build/*/firmware.elf")
    c.run("tail -4 syms-*.ld")

@task
def serial(c):
    """serial terminal session, use in separate window"""
    c.run("pio device monitor -b115200", pty=True)

def buildSegments(c):
    print("Updating segment builds")
    c.run("pio run -e boot -e core -e devs -s")
