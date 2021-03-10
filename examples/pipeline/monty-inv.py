# see https://www.pyinvoke.org

@task(native, python, default=True)
def all(c):
    """compile and test the "pipeline" demo"""

@task(help={"args": "comma-separated list of commands"})
def run(c, args):
    """process commands as a pipeline"""
    c.run("pio run -c ../pio-examples.ini -e native -s", env={"MONTY_VERSION": "pipeline"}, pty=True)
    c.run(".pio/build/native/program %s" % args.replace(",", " "), pty=True)

# remove irrelevant tasks
del flash, mrfs, runner, serial, upload
