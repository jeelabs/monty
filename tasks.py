# see https://www.pyinvoke.org

from src.devtasks import *

if not os.path.isfile(inRoot("tasks.py")):
    print("Please run invoke from '%s'" % os.path.dirname(__file__))
    sys.exit(1)

# the following tasks are not available for use out-of-tree

@task(call(generate, strip=True))
def diff(c):
    """strip all sources and compare them to git HEAD"""
    c.run("git diff", pty=True)

@task(generate)
def builds(c):
    """show µC build sizes, w/ and w/o assertions or Python VM"""
    c.run("pio run -t size | tail -8 | head -2")
    c.run("pio run -e noassert | tail -7 | head -1")
    c.run("pio run -e nopyvm | tail -7 | head -1")

@task
def examples(c):
    """build each of the example projects"""
    examples = os.listdir("examples")
    examples.sort()
    for ex in examples:
        if os.path.isdir("examples/%s" % ex):
            if os.path.isfile("examples/%s/README.md" % ex):
                print(ex)
                c.run("pio run -c examples/pio-examples.ini "
                    "-d examples/%s -t size -s" % ex, warn=True)

@task(post=[clean, test, call(python, python_skip),
            upload, flash, mrfs, call(runner, runner_skip),
            builds, examples])
def all(c):
    """i.e. clean test python upload flash mrfs runner builds examples"""
    # make sure the JeeH library is not found locally, i.e. unset this env var
    os.environ.pop("PLATFORMIO_LIB_EXTRA_DIRS", None)

@task
def x_tags(c):
    """update the (c)tags file"""
    c.run("ctags -R lib src test")

@task
def version(c):
    """display the current version tag (using "git describe)"""
    c.run("git describe --tags --always")
