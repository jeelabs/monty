# see https://www.pyinvoke.org
from invoke import task

@task(default=True, help={"file": "the '.mpy' file to run"})
def run(c, file="../../pytests/hello.mpy"):
    """execute a bytecode file"""
    c.run("pio run -c ../pio-examples.ini -e native -s", env={"MONTY_VERSION": "minimal"}, pty=True)
    c.run(".pio/build/native/program %s" % file)
