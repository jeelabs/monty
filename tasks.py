# see https://www.pyinvoke.org
from invoke import task

@task
def clean(c):
    """delete all build results"""
    c.run("rm -rf .pio")

@task
def run(c):
    """build and run the native version"""
    c.run("pio run -s")
    c.run(".pio/build/native/program")

@task
def test(c):
    """run the native test suite"""
    c.run("pio test", pty=True)

@task
def serial(c):
    """start terminal session, for use in a separate window"""
    c.run("pio device monitor -b115200", pty=True)
