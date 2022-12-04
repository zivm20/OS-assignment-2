209904606

mytee script:

Script started on 2022-12-04 20:14:30+02:00 [TERM="xterm-256color" TTY="/dev/pts/2" COLUMNS="121" LINES="24"]
]0;zivm20@MainServer: ~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[01;32mzivm20@MainServer[00m:[01;34m~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[00m$ ls
f1.txt  [0m[01;34mfolder[0m  Makefile  mync.c  myshell.c  mytee.c  readme.txt  typescript
]0;zivm20@MainServer: ~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[01;32mzivm20@MainServer[00m:[01;34m~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[00m$ cat f1.txt
random stuff
]0;zivm20@MainServer: ~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[01;32mzivm20@MainServer[00m:[01;34m~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[00m$ my [K[K[K.[Kmake all
gcc -Werror  mytee.c -o mytee
gcc -Werror  mync.c -o mync
gcc -Werror  myshell.c -o myshell
]0;zivm20@MainServer: ~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[01;32mzivm20@MainServer[00m:[01;34m~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[00m$ mytee f1.txt f2.txt[1@.[1@/
hello!
hello!
test2!
test2!
^C
]0;zivm20@MainServer: ~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[01;32mzivm20@MainServer[00m:[01;34m~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[00m$ cat f1.txt
hello!
test2!
]0;zivm20@MainServer: ~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[01;32mzivm20@MainServer[00m:[01;34m~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[00m$ cat f2.txt
hello!
test2!
]0;zivm20@MainServer: ~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[01;32mzivm20@MainServer[00m:[01;34m~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[00m$ ,[K./.[K[K/.[1P.[C./.[C[Kmyshell
>DIR
.gitattributes mytee .. f3.txt myshell f1.txt  f3.txt .gitignore folder mync.c f2.txt readme.txt myshell.c . Makefile mytee.c typescript .vscode .git mync 
>COPY f1.txt f4.txt
>echo < f4.txt
hello!
test2!

>echo "" tesing this" > f4.txt
>echo : < f4.txt
"tesing this"

>cat f4.txt
"tesing this"
>cat f2.txt
hello!
test2!
line 3
4
5
6
7>cat f2.txt | head -4
hello!
test2!
line 3
4
>cat f2.txt | head -4 > f3.5 txt
>cat f3.txt
hello!
test2!
line 3
4
>^C
]0;zivm20@MainServer: ~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[01;32mzivm20@MainServer[00m:[01;34m~/Desktop/codes/University/Year 3/Semester 1/OS/OS-assignment-2[00m$ exit
exit

Script done on 2022-12-04 20:29:48+02:00 [COMMAND_EXIT_CODE="130"]




