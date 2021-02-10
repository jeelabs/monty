# see https://www.pyinvoke.org
from invoke import task

@task(help={"file": "the '.mpy' file to run"})
def run(c, file):
    """execute a bytecode file"""
    c.run("pio run -e native -s", pty=True)
    c.run(".pio/build/native/program %s" % file)
