# see https://www.pyinvoke.org

@task(native, python, flash, mrfs, runner, default=True)
def all(c):
    """compile and run the Python tests, natively and remotely"""
