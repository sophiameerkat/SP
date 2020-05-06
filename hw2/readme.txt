Execution
use command make to generate bidding_system, host and player
use ./bidding_system [host_num] [player_num] to run

Description
1. bidding_system:
a. make FIFO
b. calculate all the possibilities of the combination
c. fork and exec host
d. open the FIFO (Host.FIFO for read, Hostid.FIFO for write)
e. write the player to Hostid.FIFO and read the rank from Host.FIFO (use
an array to store which host is available now when writing to Hostid.FIFO)
f. after all the competitions done, send eight -1 to all the Hostid.FIFO
e. wait and unlink the FIFO

2. host: 
(1) For root host: 
a. open Hostid.FIFO -> build the pipe between root and child
-> fork the child host -> do the redirection for the child host -> exec the
child host
b. read the player from Hostid.FIFO through FIFO
c. write player to child through pipe
d. read winner and money from child through pipe (10 times)
e. write winner to child through pipe (9 times)
f. write rank to Host.FIFO through FIFO
(b c d e f steps are all in a while loop until it receives eight -1)
(2) For child host:
a. build the pipe between child and leaf -> fork the leaf host -> do the
redirection for the leaf host -> exec the leaf host
b. read the player from root through stdin
c. write the player to leaf through pipe
d. read winner and money from leaf through pipe (10 times)
e. write winner and money to root through stdout (10 times)
f. read winner from root through stdin (9 times)
g. write winner to leaf through pipe (9 times)
(b c d e f g steps are all in a while loop until it receives four -1 from root
host)
(3) For leaf host:
a. build the pipe between leaf and player -> read the player from child
through stdin -> fork the player -> do the redirection for the player -> exec
the player
b. read the player information (the money they pays) from player through pipe
(10 times)
c. write winner and money to child through stdout (10 times)
d. read winner from child through stdin (9 times)
e. write winner to player through pipe (9 times)
(a b c d e steps are all in a while loop until it receives two -1 from child
host)

3. player: send the money the player pays to leaf host for ten times and get the
information of the winner from leaf host for nine times.
