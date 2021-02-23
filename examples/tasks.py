# see https://www.pyinvoke.org
from invoke import task

@task(default=True)
def run(c):
    """compile and upload this example"""
    c.run("pio run -c ../pio-examples.ini -s", echo=True, hide='stdout', pty=True)
