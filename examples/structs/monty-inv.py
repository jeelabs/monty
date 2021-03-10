# see https://www.pyinvoke.org

@task(native, flash, serial, default=True)
def all(c):
    """compile and test the "minimal" demo"""

# remove irrelevant tasks
#del flash, mrfs, runner, serial, upload
