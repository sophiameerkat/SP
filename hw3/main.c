#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <signal.h> 
#define SIGUSR3 SIGWINCH

int main(int argc, char** argv){
	int P, Q;
	scanf("%d%d", &P, &Q);
	int num_signal;
	scanf("%d", &num_signal);
	int ssignal[11];
	for (int i = 0; i < num_signal; i++)
		scanf("%d", &ssignal[i]);
	sigset_t newmask, oldmask;
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1);
	sigaddset(&newmask, SIGUSR2);
	sigaddset(&newmask, SIGUSR3);
	sigprocmask(SIG_BLOCK, &newmask, &oldmask);
	int fd[2];
	pipe(fd);

	pid_t process;
	process = fork();
	if (process == 0){
		close(fd[0]);
		dup2(fd[1], 1);
		char arg2[50], arg3[50], arg4[50], arg5[50];
		snprintf(arg2, sizeof(arg2), "%d", P);
		snprintf(arg3, sizeof(arg3), "%d", Q);
		snprintf(arg4, sizeof(arg4), "%d", 3);
		snprintf(arg5, sizeof(arg5), "%d", 0);
		if (execlp("./hw3", "./hw3", arg2, arg3, arg4, arg5, NULL) < 0)
			perror("exec child error\n");
	}
	else {
		close(fd[1]);
		int status;
		for (int i = 0; i < num_signal; i++){
			sleep(5);
			if (ssignal[i] == 1)
				kill(process, SIGUSR1);
			else if (ssignal[i] == 2)
				kill(process, SIGUSR2);
			else if (ssignal[i] == 3)
				kill(process, SIGUSR3);
			char qu[1024] = "";
			read(fd[0], qu, sizeof(qu));
			if (ssignal[i] == 3){
				int len = strlen(qu);
				for (int j = 0; j < len - 1; j++)
					printf("%d ", qu[j] - '0');
				printf("%d\n", qu[len - 1] - '0');
			}
		}
		char con_arr[512];
		read(fd[0], con_arr, sizeof(con_arr));
		printf("%s\n", con_arr);
		wait(&status);
		close(fd[0]);
	}
}