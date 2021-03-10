# see https://www.pyinvoke.org

@task(generate, help={"args": "comma-separated list of commands"})
def run(c, args):
    """process commands as a pipeline"""
    c.run("pio run -c ../../platformio.ini -e native -s", env={"MONTY_VERSION": "pipeline"}, pty=True)
    c.run(".pio/build/native/program %s" % args.replace(",", " "), pty=True)

@task(native, python, call(run, "bv,gr,verify/hello.mpy,gr,gc,gr"),
      default=True)
def all(c):
    """compile and test the "pipeline" demo"""

# remove irrelevant tasks
del flash, mrfs, runner, serial, upload
