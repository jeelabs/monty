# see https://www.pyinvoke.org
from invoke import task

@task
def run(c):
    """execute a bytecode file"""
    c.run("pio run -e native -s", pty=True)
    c.run("mpy-cross jetest.py")
    c.run(".pio/build/native/program jetest.mpy", pty=True)
