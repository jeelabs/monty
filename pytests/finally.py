def f(throw):
    print("enter", throw)
    try:
        print("try1")
        if throw:
            raise Exception()
        print("try2")
    except:
        print("except")
    else:
        print("else")
    finally:
        print("finally")
    print("leave")

f(False)
print()
f(True)
