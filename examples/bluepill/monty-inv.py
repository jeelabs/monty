# see https://www.pyinvoke.org

""" This script is loaded by `inv` to define additional tasks for this area.
    The example below defines an "all" task. Uncomment and adjust to taste.
"""

@task(default=True)
def all(c):
    """this is an example "all" task, defined in monty-inv.py"""

    print('''
        See the PyInvoke site for how tasks work: https://www.pyinvoke.org
        One quick option is to simply make "all" dependent on some of the
        other tasks. Since it is set as default, running "inv" without any
        args will run each of the dependent tasks in sequence. For example:

            @task(native, test, python, default=True)
            def all(c):
                """compile natively and run all C++ & Python tests"""

        With this setup, "inv" will compile a native build and run all the
        C++ & Python tests. Adjust as needed to your own preferred workflow.
    ''')
