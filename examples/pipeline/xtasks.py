# see https://www.pyinvoke.org
from invoke import task

@task(help={"args": "comma-separated list of commands"})
def run(c, args):
    """process commands as a pipeline"""
    c.run("pio run -c ../pio-examples.ini -e native -s", env={"MONTY_VERSION": "pipeline"}, pty=True)
    c.run(".pio/build/native/program %s" % args.replace(",", " "), pty=True)
