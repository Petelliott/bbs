# BBS

## about

a simple bulliten board system over telnet. GNU AGPLv3

## use

#### server side:

````
$ make
$ sudo bbs
````

#### client side:

to connect to a bbs run `$ telnet {server}`

accounts will be made automaticaly if you log in with a new username:

````
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
user: peter
pass: password
welcome, peter
````

the `write` command will prompt you for a title and then let you write. a post
is terminated by two empty lines.

````
> write
title: the write command
this is a post
it can be many lines

even have line breaks


>
````

the `read` command will show you the post defined by the number you pass it.
if you do not provide a number it will give you the most recent post

````
> read 7
#7: peter 'ascii art' 1504238676
                 _ _              _   
                (_|_)            | |  
   __ _ ___  ___ _ _    __ _ _ __| |_
  / _` / __|/ __| | |  / _` | '__| __|
 | (_| \__ \ (__| | | | (_| | |  | |_
  \__,_|___/\___|_|_|  \__,_|_|   \__|
>
````

the `exit` command exits

````
> exit
Connection closed by foreign host.
$
````

the `list` command will show the last n posts. if no argument is provided, it defaults to 10

````
> list 5
#8: peter2 'peter2' 1504298632
#7: peter 'ascii art' 1504238676
#6: peter 'RE: lists work now' 1504238554
#5: peter 'list work now' 1504238097
#4: peter 'lists ' 1504237913
````
