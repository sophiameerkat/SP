1. I/O multiplexing by select():
I use select() to do I/O multiplexing. First, I use a variable fdmax =
svr.listen_fd to get the maximum file descriptor, then use FD_SET to update
svr.listen_fd to master. Next, in the while loop copy master to read_fds and
use a for loop to process the connection, use FD_ISSET to find if read_fds is
in the set. If it is in the set, check if it is a new connection or to process
the client's data. If it is a new connection, update the value of fd_max and
process for the new connection. If it is to process the client's data, it
process for read_server and write_server individually.
2. Guarantee correctness by file lock:
In the write_server, I use the struct flock to set for the parameters of the
lock. If fcntl(file_fd, F_SETLK, &f1) returns -1, it means that the account is
locked and print "This account is locked." to standard output. Otherwise, it
will lock the part of the account_list to prevent other people changing the
content at the same time and starts the operation.
3. READ_SERVER
When getting the input, check if it is locked before. If it is locked, print
"This account is locked." to standard output. Otherwise, use lseek and read to
find the information of that account in the account_list file, then print the id and the
balance of that account.
4. WRITE_SERVER
If requestP[i].wait_for_write is 0, which means for getting the first input (the instruction for specifying which account to
modify), check if it is locked before. If it is locked, print "This account is
locked." to standard output. Otherwise, print "This account is modifiable." to
standard output and add 1 to requestP[i].wait_for_time. If
requestP[i].wait_for_write is 1, which means for getting the second input
(the instruction for the operation on the account), check if the instruction
is a valid command and do the operation.
