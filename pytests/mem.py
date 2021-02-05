n = 0
for z in range(25):
    for i in range(100):
        l = [i,1,2,3,4,5]
        for x in l:
            n += x * i
    print(z, n)
