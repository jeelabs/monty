# see https://www.pyinvoke.org

@task(flash, mrfs, runner, serial, default=True)
def all(c):
    """compile and test the "minimal" demo"""
