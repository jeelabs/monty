# see https://www.pyinvoke.org
from invoke import task

@task
def zero(c):
    """delete all build results"""
    c.run("rm -rf .pio")

@task
def native(c):
    """run the native build"""
    c.run("pio run -e native -s")
    c.run(".pio/build/native/program")

@task
def test(c):
    """run the native C++ test suite"""
    c.run("pio test -e native", pty=True)

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
def info(c):
    """show some information about segments"""
    c.run("pio run -s")
    c.run("arm-none-eabi-size .pio/build/*/firmware.elf")
    c.run("tail -2 util/syms-*.ld")

@task
def serial(c):
    """serial terminal session, for use in a separate window"""
    c.run("pio device monitor -b115200", pty=True)
