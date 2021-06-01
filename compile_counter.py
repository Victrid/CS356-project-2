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
tr = """
-----------------------------------------------------------
               The damned penguin warns:
-----------------------------------------------------------
         _nnnn_                      
        dGGGGMMb     ,''''''''''''''''''''''''''''''''''.
       @p~qp~~qMb    | Damn you've compiled this kernel |
       M|\|| /|M|   _|      """
bg=                          """ fuckin' times!         |
       @,----.JM| -' ;..................................'
      JS^\_|_/ qKL
     dZP  -^-   qKRb
    dZP          qKKb
   fZP            SMMb
   HZM            MMMM
   FqM            MMMM
 __| ".        |\dS"qML
 |    `.       | `' \Zq
_)      \.___.,|     .'
\____   )MMMMMM|   .'
     `-'       `--'
-----------------------------------------------------------
"""
print(tr+str(num).ljust(4) +bg)
num += 1
f = open("counter", "w+")
f.write(str(num))
f.close()
