#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>

#define ERR_EXIT(a) { perror(a); exit(1); }

typedef struct {
	char hostname[512];  // server's hostname
	unsigned short port;  // port to listen
	int listen_fd;  // fd to wait for a new connection
} server;

typedef struct {
	char host[512];  // client's host
	int conn_fd;  // fd to talk with client
	char buf[512];  // data sent by/to client
	size_t buf_len;  // bytes used by buf
	// you don't need to change this.
	int item;
	int wait_for_write;  // used by handle_read to know if the header is read or not.
} request;

typedef struct{
	int id;
	int balance;
}Account;	

server svr;  // server
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list

const char* accept_read_header = "ACCEPT_FROM_READ";
const char* accept_write_header = "ACCEPT_FROM_WRITE";

// Forwards

static void init_server(unsigned short port);
// initailize a server, exit for error

static void init_request(request* reqP);
// initailize a request instance

static void free_request(request* reqP);
// free resources used by a request instance

static int handle_read(request* reqP);
// return 0: socket ended, request done.
// return 1: success, message (without header) got this time is in reqP->buf with reqP->buf_len bytes. read more until got <= 0.
// It's guaranteed that the header would be correctly set after the first read.
// error code:
// -1: client connection error

int main(int argc, char** argv) {
	int i, ret;

	struct sockaddr_in cliaddr;  // used by accept()
	int clilen;

	int conn_fd;  // fd for a new connection with client
	int file_fd;  // fd for file that we open for reading
	char buf[512];
	char buf1[512];
	int buf_len;
	int buf1_len;
	fd_set master;
	fd_set read_fds;
	// Parse args.
	if (argc != 2) {
		fprintf(stdout, "usage: %s [port]\n", argv[0]);
		exit(1);
	}

	// Initialize server
	init_server((unsigned short) atoi(argv[1]));

	// Get file descripter table size and initize request table
	maxfd = getdtablesize();
	requestP = (request*) malloc(sizeof(request) * maxfd);
	if (requestP == NULL) {
		ERR_EXIT("out of memory allocating all requests");
	}
	for (i = 0; i < maxfd; i++) {
		init_request(&requestP[i]);
	}
	requestP[svr.listen_fd].conn_fd = svr.listen_fd;
	strcpy(requestP[svr.listen_fd].host, svr.hostname);

	// Loop for handling connections
	fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);
        file_fd = open("account_list", O_RDWR);
	//printf("fd %d\n", file_fd);
	//Account input;
	//lseek(file_fd, 0, SEEK_SET);
	//while (read(file_fd, &input, sizeof(Account)))
	//	fprintf(stdout, "%d %d\n", input.id, input.balance);
	int fdmax = svr.listen_fd;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	FD_SET(svr.listen_fd, &master);
	int flag[20] = {0};
	while (1) {
		// TODO: Add IO multiplexing
		// Check new connection
		read_fds = master;
		select(fdmax + 1, &read_fds, NULL, NULL, NULL);
		for (int i = 0; i <= fdmax; i++){
			if (FD_ISSET(i, &read_fds)){
				if (i == svr.listen_fd){
					clilen = sizeof(cliaddr);
					conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
					if (conn_fd < 0) {
						if (errno == EINTR || errno == EAGAIN) continue;  // try again
						if (errno == ENFILE) {
							(void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
							continue;
						}
						ERR_EXIT("accept")
					}
					else {
						FD_SET(conn_fd, &master);
						if (conn_fd > fdmax)
							fdmax = conn_fd;
						requestP[conn_fd].conn_fd = conn_fd;
						strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
						fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);
					}
				}
				else {
					ret = handle_read(&requestP[i]); // parse data from client to requestP[conn_fd].buf
					if (ret < 0) {
						fprintf(stderr, "bad request from %s\n", requestP[i].host);
						continue;
					}
#ifdef READ_SERVER
					//sprintf(buf,"%s : %s\n",accept_read_header,requestP[i].buf);
					int account_index;
					struct flock f1;
					account_index = atoi(requestP[i].buf);
					f1.l_type = F_RDLCK;
					f1.l_start = account_index * sizeof(Account);
					f1.l_whence = SEEK_SET;
					f1.l_len = sizeof(Account);
					f1.l_pid = 0;
					if (fcntl(file_fd, F_SETLK, &f1) == -1){
						sprintf(buf, "This account is locked.\n");
						write(requestP[i].conn_fd, buf, strlen(buf));
						close(requestP[i].conn_fd);
					}
					else {
						Account tmp;
						lseek(file_fd, (atoi(requestP[i].buf) - 1) * sizeof(Account), SEEK_SET);
						read(file_fd, &tmp, sizeof(Account));
						sprintf(buf, "%d %d\n", tmp.id, tmp.balance);
						write(requestP[i].conn_fd, buf, strlen(buf));
						close(requestP[i].conn_fd);
						free_request(&requestP[i]);
						FD_CLR(i, &master);
					}
#else
					//sprintf(buf,"%s : %s\n",accept_write_header,requestP[i].buf);
					int account_index;
					int money;
					int money1;
					char *token;
					char *deli = " ";
					char arr[3][100];
					int index = 0;
					struct flock f1;
					if (requestP[i].wait_for_write == 0){
						account_index = atoi(requestP[i].buf);
						f1.l_type = F_WRLCK;
						f1.l_start = account_index * sizeof(Account);
						f1.l_whence = SEEK_SET;
						f1.l_len = sizeof(Account);
						f1.l_pid = 0;
						if (fcntl(file_fd, F_SETLK, &f1) == -1 || flag[account_index] == 1){
							sprintf(buf, "This account is locked.\n");
							write(requestP[i].conn_fd, buf, strlen(buf));
							close(requestP[i].conn_fd);
							free_request(&requestP[i]);
							FD_CLR(i, &master);
						}
						else {
							sprintf(buf, "This account is modifiable.\n");    
							write(requestP[i].conn_fd, buf, strlen(buf));
							requestP[i].wait_for_write++;
							flag[account_index] = 1;
						}
					}
					else if (requestP[i].wait_for_write == 1){
						lseek(file_fd, (account_index - 1) * sizeof(Account), SEEK_SET);
						lseek(file_fd, sizeof(int), SEEK_CUR);
						read(file_fd, &money, sizeof(int));
						token = strtok(requestP[i].buf, deli);
						while (token != NULL){
							strcpy(arr[index], token);
							index++;
							token = strtok(NULL, deli);
						}
						if (strcmp(arr[0], "save") == 0){
							if (atoi(arr[1]) < 0){
								sprintf(buf1, "Operation failed.\n");
								write(requestP[i].conn_fd, buf1, strlen(buf1));
							}
							else{
								money += atoi(arr[1]);
								lseek(file_fd, (account_index - 1) * sizeof(Account), SEEK_SET);
								lseek(file_fd, sizeof(int), SEEK_CUR);
								write(file_fd, &money, sizeof(int));
								//Account input;
								//lseek(file_fd, 0, SEEK_SET);
								//while (read(file_fd, &input, sizeof(Account)))
								//    fprintf(stdout, "%d %d\n", input.id, input.balance);	    
							}
						}
						else if (strcmp(arr[0], "withdraw") == 0){
							if (atoi(arr[1]) < 0 || atoi(arr[1]) > money){
								sprintf(buf1, "Operation failed.\n");
								write(requestP[i].conn_fd, buf1, strlen(buf1));
							}
							else{
								money -= atoi(arr[1]);
								lseek(file_fd, (account_index - 1) * sizeof(Account), SEEK_SET);
								lseek(file_fd, sizeof(int), SEEK_CUR);
								write(file_fd, &money, sizeof(int));
								//Account input;
								//lseek(file_fd, 0, SEEK_SET);
								//while (read(file_fd, &input, sizeof(Account)))
								//    fprintf(stdout, "%d %d\n", input.id, input.balance);
							}
						}
						else if (strcmp(arr[0], "transfer") == 0){
							if (atoi(arr[2]) < 0 || atoi(arr[2]) > money){
								sprintf(buf1, "Operation failed.\n");
								write(requestP[i].conn_fd, buf1, strlen(buf1));
							}
							else{
								money -= atoi(arr[2]);
								lseek(file_fd, (atoi(arr[1]) - 1) * sizeof(Account), SEEK_SET);
								lseek(file_fd, sizeof(int), SEEK_CUR);
								read(file_fd, &money1, sizeof(int));
								money1 += atoi(arr[2]);
								lseek(file_fd, (account_index - 1) * sizeof(Account), SEEK_SET);
								lseek(file_fd, sizeof(int), SEEK_CUR);
								write(file_fd, &money, sizeof(int));
								lseek(file_fd, (atoi(arr[1]) - 1) * sizeof(Account), SEEK_SET);
								lseek(file_fd, sizeof(int), SEEK_CUR);
								write(file_fd, &money1, sizeof(int));
								//Account input;
								//lseek(file_fd, 0, SEEK_SET);
								//while (read(file_fd, &input, sizeof(Account)))
								//    fprintf(stdout, "%d %d\n", input.id, input.balance);
							}
						}
						else if (strcmp(arr[0], "balance") == 0){
							if (atoi(arr[1]) <= 0){
								sprintf(buf1, "Operation failed.\n");
								write(requestP[i].conn_fd, buf1, strlen(buf1));
							}
							else {
								money = atoi(arr[1]);
								lseek(file_fd, (account_index - 1) * sizeof(Account), SEEK_SET);
								lseek(file_fd, sizeof(int), SEEK_CUR);
								write(file_fd, &money, sizeof(int));
							}
						}
						f1.l_type = F_UNLCK;
						fcntl(file_fd, F_SETLK, &f1);
						flag[account_index] = 0;
						close(requestP[i].conn_fd);
						free_request(&requestP[i]);
						FD_CLR(i, &master);
					}
#endif
				}
			}
		}
	}
	free(requestP);
	return 0;
}


// ======================================================================================================
// You don't need to know how the following codes are working
#include <fcntl.h>

static void* e_malloc(size_t size);


static void init_request(request* reqP) {
	reqP->conn_fd = -1;
	reqP->buf_len = 0;
	reqP->item = 0;
	reqP->wait_for_write = 0;
}

static void free_request(request* reqP) {
	/*if (reqP->filename != NULL) {
	  free(reqP->filename);
	  reqP->filename = NULL;
	  }*/
	init_request(reqP);
}

// return 0: socket ended, request done.
// return 1: success, message (without header) got this time is in reqP->buf with reqP->buf_len bytes. read more until got <= 0.
// It's guaranteed that the header would be correctly set after the first read.
// error code:
// -1: client connection error
static int handle_read(request* reqP) {
	int r;
	char buf[512];

	// Read in request from client
	r = read(reqP->conn_fd, buf, sizeof(buf));
	if (r < 0) return -1;
	if (r == 0) return 0;
	char* p1 = strstr(buf, "\015\012");
	int newline_len = 2;
	// be careful that in Windows, line ends with \015\012
	if (p1 == NULL) {
		p1 = strstr(buf, "\012");
		newline_len = 1;
		if (p1 == NULL) {
			ERR_EXIT("this really should not happen...");
		}
	}
	size_t len = p1 - buf + 1;
	memmove(reqP->buf, buf, len);
	reqP->buf[len - 1] = '\0';
	reqP->buf_len = len-1;
	return 1;
}

static void init_server(unsigned short port) {
	struct sockaddr_in servaddr;
	int tmp;

	gethostname(svr.hostname, sizeof(svr.hostname));
	svr.port = port;

	svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (svr.listen_fd < 0) ERR_EXIT("socket");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	tmp = 1;
	if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0) {
		ERR_EXIT("setsockopt");
	}
	if (bind(svr.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		ERR_EXIT("bind");
	}
	if (listen(svr.listen_fd, 1024) < 0) {
		ERR_EXIT("listen");
	}
}

static void* e_malloc(size_t size) {
	void* ptr;

	ptr = malloc(size);
	if (ptr == NULL) ERR_EXIT("out of memory");
	return ptr;
}

