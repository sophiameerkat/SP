#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <sys/types.h> 

char str[20][50];
int fd0;
void fifoname(){
	for (int i = 1; i <= 10; i++)
		sprintf(str[i], "%s%d%s", "Host", i, ".FIFO");
	strcpy(str[0], "Host.FIFO");
	fd0 = open(str[0], O_RDWR);
}

typedef struct ans{
	int id;
	int score;
}Ans;

int cmp(const void * a, const void * b){
	Ans *tmpa = (Ans *)a;
	Ans *tmpb = (Ans *)b;
	if (tmpa -> score >= tmpb -> score)
		return -1;
	return 1;
}

int main(int argc, char** argv){
	int hostid = atoi(argv[1]);
	int randomkey = atoi(argv[2]);
	int depth = atoi(argv[3]);

	int people[150];

	if (depth == 0){//root host
		int fdrc[2][2];//root -> child //root: write //child: read
		int fdcr[2][2];//child -> root //root: read //child: write
		fifoname();//get fifo name
		int fd;//open Hostid.FIFO
		fd = open(str[hostid], O_RDONLY);
		FILE *fp = fdopen(fd, "r");

		//build the pipe between root and child
		pipe(fdrc[0]);
		pipe(fdrc[1]);
		pipe(fdcr[0]);
		pipe(fdcr[1]);

		//1. fork child
		for (int i = 0; i < 2; i++){
			pid_t process;
			process = fork();
			if (process < 0)
				perror("fork child");
			else if (process > 0) {//root host close unused pipe
				close(fdrc[i][0]);//close the read end of root -> child of root
				close(fdcr[i][1]);//close the write end of child -> root of root
			}
			else if (process == 0){//exec child host
				if (i == 0){
					close(fdrc[0][1]);//close the write end of root -> child of child
					close(fdcr[0][0]);//close the read end of child -> root of child
					dup2(fdrc[0][0], 0);//redirect standard input to the read of pipe
					dup2(fdcr[0][1], 1);//redirect standard output to the write of pipe
					close(fdrc[0][0]);
					close(fdcr[0][1]);
				}
				else if (i == 1){
					close(fdrc[1][1]);//close the write end of root -> child of child
					close(fdcr[1][0]);//close the read end of child -> root of child
					dup2(fdrc[1][0], 0);//redirect standard input to the read of pipe
					dup2(fdcr[1][1], 1);//redirect standard output to the write of pipe
					close(fdrc[1][0]);
					close(fdcr[1][1]);
				}
				char arg2[50], arg3[50], arg4[50];
				snprintf(arg2, sizeof(arg2), "%d", hostid);
				snprintf(arg3, sizeof(arg3), "%d", randomkey);
				snprintf(arg4, sizeof(arg4), "%d", 1);
				if (execlp("./host", "./host", arg2, arg3, arg4, NULL) < 0)
					perror("exec child error\n");
				exit(0);
			}
		}

		while (1){//break when it receives -1 -1 -1 -1 -1 -1 -1 -1 from FIFO (after tell the child)
			//2. read the player from Hostid.FIFO through FIFO
			int id[20];
			fflush(fp);
			fscanf(fp, "%d %d %d %d %d %d %d %d", &people[0], &people[1], &people[2], &people[3], &people[4], &people[5], &people[6], &people[7]);
			
			for (int i = 0; i < 8; i++){
				id[people[i]] = i;
			}

			//3. write player to child through pipe
			char str1[150];
			sprintf(str1, "%d %d %d %d\n", people[0], people[1], people[2], people[3]);
			write(fdrc[0][1], str1, strlen(str1));
			fsync(fdrc[0][1]);

			char str2[150];
			sprintf(str2, "%d %d %d %d\n", people[4], people[5], people[6], people[7]);
			write(fdrc[1][1], str2, strlen(str2));
			fsync(fdrc[1][1]);

			if (people[0] == -1){
				for (int i = 0; i < 2; i++){
					close(fdrc[i][1]);
					close(fdcr[i][0]);
				}
				for (int i = 0; i < 2; i++){
					int status;
					wait(&status);
				}
				exit(0);
			}

			Ans p[10];
    		for (int i = 0; i < 8; i++){
	    		p[i].id = people[i];
    			p[i].score = 0;
    		}

			for (int i = 0; i < 10; i++){//read ten times //write nine times
				//13. read winner & money from child through pipe
				int playerid, spend;
				char test[150];
				fsync(fdcr[0][0]);
				read(fdcr[0][0], test, sizeof(test));
				
				sscanf(test, "%d %d", &playerid, &spend);

				int playerid1, spend1;
				char test1[150];
				fsync(fdcr[1][0]);
				read(fdcr[1][0], test1, sizeof(test1));
				sscanf(test1, "%d %d", &playerid1, &spend1);

				if (spend1 > spend){
					p[id[playerid1]].score++;
				}
				else {
					p[id[playerid]].score++;
				}
				if (i != 9){
					//14. write winner to child through pipe
					char str3[60];
					if (spend1 > spend){
						sprintf(str3, "%d\n", playerid1);
					}
					else {
						sprintf(str3, "%d\n", playerid);
					}
					write(fdrc[0][1], str3, strlen(str3));
					fsync(fdrc[0][1]);
					write(fdrc[1][1], str3, strlen(str3));
					fsync(fdrc[1][1]);
				}
			}

			//19. write rank to Host.FIFO through FIFO
			qsort(p, 8, sizeof(Ans), cmp);

			int finalans[20];

			finalans[p[0].id] = 1;
			int count = 1;
			for (int i = 1; i < 8; i++){
				count++;
				if (p[i].score == p[i - 1].score)
					finalans[p[i].id] = finalans[p[i - 1].id];
				else
					finalans[p[i].id] = count;
			}
			
			char str4[150];
			sprintf(str4, "%d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n", randomkey, people[0], finalans[people[0]], people[1], finalans[people[1]], people[2], finalans[people[2]], people[3], finalans[people[3]], people[4], finalans[people[4]], people[5], finalans[people[5]], people[6], finalans[people[6]], people[7], finalans[people[7]]);
			write(fd0, str4, strlen(str4));
			fsync(fd0);
		}
	}
	else if (depth == 1){//child host
		int fdcl[2][2];//child -> leaf //child: write //leaf: read
		int fdlc[2][2];//leaf -> child //child: read //leaf: write
		pipe(fdcl[0]);
		pipe(fdcl[1]);
		pipe(fdlc[0]);
		pipe(fdlc[1]);

		//4. fork leaf
		for (int i = 0; i < 2; i++){
			pid_t process;
			process = fork();
			if (process < 0)
				perror("fork leaf");
			else if (process > 0) {//child host close unused pipe
				close(fdcl[i][0]);//close the read end of child -> leaf of root
				close(fdlc[i][1]);//close the write end of leaf -> child of root
			}
			else if (process == 0){//exec leaf host
				if (i == 0){
					close(fdcl[0][1]);//close the write end of child -> leaf of child
					close(fdlc[0][0]);//close the read end of leaf -> child of child
					dup2(fdcl[0][0], 0);//redirect standard input to the read of pipe
					dup2(fdlc[0][1], 1);//redirect standard output to the write of pipe
					close(fdcl[0][0]);
					close(fdlc[0][1]);
				}
				else if (i == 1){
					close(fdcl[1][1]);//close the write end of child -> leaf of child
					close(fdlc[1][0]);//close the read end of leaf -> child of child
					dup2(fdcl[1][0], 0);//redirect standard input to the read of pipe
					dup2(fdlc[1][1], 1);//redirect standard output to the write of pipe
					close(fdcl[1][0]);
					close(fdlc[1][1]);
				}

				char arg2[50], arg3[50], arg4[50];
				snprintf(arg2, sizeof(arg2), "%d", hostid);
				snprintf(arg3, sizeof(arg3), "%d", randomkey);
				snprintf(arg4, sizeof(arg4), "%d", 2);
				execlp("./host", "./host", arg2, arg3, arg4, NULL);
				exit(0);
			}
		}

		while (1){//break when it receives -1 -1 -1 -1 from fdrc (after tell the leaf)
			//5. read the player from root through stdin
			int p1, p2, p3, p4;
			fflush(stdin);
			scanf("%d %d %d %d", &p1, &p2, &p3, &p4);

			//6. write the player to leaf through pipe
			char str5[150];
			sprintf(str5, "%d %d\n", p1, p2);
			write(fdcl[0][1], str5, strlen(str5));
			fsync(fdcl[0][1]);
			char str6[150];
			sprintf(str6, "%d %d\n", p3, p4);
			write(fdcl[1][1], str6, strlen(str6));
			fsync(fdcl[1][1]);

			if (p1 == -1){
				for (int i = 0; i < 2; i++){
					int status;
					wait(&status);
				}
				exit(0);
			}

			for (int i = 0; i < 10; i++){
				//11. read winner & money from leaf through pipe
				int playerid, spend;
				char test[50];
				fsync(fdlc[0][0]);
				read(fdlc[0][0], test, sizeof(test));
				
				sscanf(test, "%d %d", &playerid, &spend);

				int playerid1, spend1;
				char test1[150];
				fsync(fdlc[1][0]);
				read(fdlc[1][0], test1, sizeof(test1));
				
				sscanf(test1, "%d %d", &playerid1, &spend1);

				//12. write winner & money to root through stdout
				char str7[150];
				if (spend1 > spend)
					printf("%d %d\n", playerid1, spend1);
				else
					printf("%d %d\n", playerid, spend);
				fflush(stdout);

				if (i != 9){
					//15. read winner from root through stdin
					int winner;
					fflush(stdin);
					scanf("%d", &winner);
					
					//16. write winner to leaf through pipe
					char str8[50];
					sprintf(str8, "%d\n", winner);
					write(fdcl[0][1], str8, strlen(str8));
					fsync(fdcl[0][1]);
					write(fdcl[1][1], str8, strlen(str8));
					fsync(fdcl[1][1]);
				}
			}
		}
	}
	else if (depth == 2){//leaf host

		while (1){//break when it receives -1 -1 from fdcl
			int fdlp[2][2];//leaf -> player //leaf: write //player: read
			int fdpl[2][2];//player -> leaf //leaf: read //player: write
			pipe(fdlp[0]);
			pipe(fdlp[1]);
			pipe(fdpl[0]);
			pipe(fdpl[1]);

			//7. read the player from child host through stdin
			int p1, p2;
			fflush(stdin);
			scanf("%d %d", &p1, &p2);

			if (p1 == -1)
				break;

			//8. fork player
			for (int i = 0; i < 2; i++){
				pid_t process;
				process = fork();
				if (process < 0)
					perror("fork player");
				else if (process > 0) {//child host close unused pipe
					close(fdlp[i][0]);//close the read end of leaf -> player of root
					close(fdpl[i][1]);//close the write end of player -> leaf of root
				}
				else if (process == 0){//exec player
					if (i == 0){
						close(fdlp[0][1]);//close the write end of leaf -> player of child
						close(fdpl[0][0]);//close the read end of player -> leaf of child
						dup2(fdlp[0][0], 0);//redirect standard input to the read of pipe
						dup2(fdpl[0][1], 1);//redirect standard output to the write of pipe
					}
					else if (i == 1){
						close(fdlp[1][1]);//close the write end of leaf -> player of child
						close(fdpl[1][0]);//close the read end of player -> leaf of child
						dup2(fdlp[1][0], 0);//redirect standard input to the read of pipe
						dup2(fdpl[1][1], 1);//redirect standard output to the write of pipe
					}

					if (i == 0){
						char arg2[50];
						snprintf(arg2, sizeof(arg2), "%d", p1);
						execlp("./player", "./player", arg2, NULL);
						fprintf(stderr, "error!\n");
					}
					else if (i == 1){
						char arg2[50];
						snprintf(arg2, sizeof(arg2), "%d", p2);
						execlp("./player", "./player", arg2, NULL);
						exit(0);
					}
				}
			}

			for (int i = 0; i < 10; i++){
				//9. read player information from player through pipe
				int playerid, spend;
				char test[150];
				fsync(fdpl[0][0]);
				read(fdpl[0][0], test, sizeof(test));
				sscanf(test, "%d %d", &playerid, &spend);

				int playerid1, spend1;
				char test1[150];
				fsync(fdpl[1][0]);
				read(fdpl[1][0], test1, sizeof(test1));
				sscanf(test1, "%d %d", &playerid1, &spend1);

				//10. write winner & money to child through standard output
				int maxmoney, maxplayer;
				if (spend1 > spend){
					maxmoney = spend1;
					maxplayer = playerid1;
				}
				else {
					maxmoney = spend;
					maxplayer = playerid;
				}
				fprintf(stdout,"%d %d\n", maxplayer, maxmoney);
				
				fflush(stdout);

				if (i != 9){
					//17. read winner from child through stdin
					int winner;
					fflush(stdin);
					scanf("%d", &winner);

					//18. write winner to player through pipe
					char str10[150];
					sprintf(str10, "%d\n", winner);
					write(fdlp[0][1], str10, strlen(str10));
					fsync(fdlp[0][1]);
					write(fdlp[1][1], str10, strlen(str10));
					fsync(fdlp[1][1]);
				}
			}
			for (int i = 0; i < 2; i++){
				close(fdlp[i][1]);
				close(fdpl[i][0]);
			}
			for (int i = 0; i < 2; i++){
				int status;
				wait(&status);
			}
		}
		exit(0);
	}
	return 0;
}
