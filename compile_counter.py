#!/bin/python
try:
    f = open("counter", "r")
    try:
        num = int(f.read())
    except ValueError:
        num = 0
    f.close()
except FileNotFoundError:
    num = 0
print("You've compiled "+str(num) + " times!")
num += 1
f = open("counter", "w+")
f.write(str(num))
f.close()
