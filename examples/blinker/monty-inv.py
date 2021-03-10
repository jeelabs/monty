# see https://www.pyinvoke.org

@task(flash, default=True)
def all(c):
    """compile and upload the "blinker" demo"""

# remove irrelevant tasks
del mrfs, native, python, runner, serial, test, upload, watch
