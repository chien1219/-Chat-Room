# Chat-Room
Chat room using TCP connection

implement and test under Ubuntu 16.04

supported multi-clients by select()

>gcc my_server.c -o server

>gcc my_client.c -o client

./client [IP] [Port]

usage:

name          -change your own name (anonymous by default)

tell A msg    -send 'msg' to A

yell msg      -tell all users in the connection 'msg'

who           -ask server for users list in the connection

exit          -disconnect
