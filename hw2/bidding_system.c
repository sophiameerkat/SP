#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <time.h>

char people[4000][50];
int ind = 0;

typedef struct ans{
	int id;
	int score;
}Ans;

int cal(int num){
	int up = 1, down = 1;
	for (int i = num; i > 8; i--)
		up *= i;
	for (int i = 1; i <= num - 8; i++)
		down *= i;
	return (up / down);
}

int randomnumber(int lower, int upper){
	int num = (rand() % (upper - lower + 1) + lower);
	return num;
}

void combination(int arr[], int tmparr[], int r, int s, int e, int index){
	if (r == index){
		sprintf(people[ind], "%d %d %d %d %d %d %d %d\n", tmparr[0], tmparr[1], tmparr[2], tmparr[3], tmparr[4], tmparr[5], tmparr[6], tmparr[7]);
	    ind++;
	}
	else {
		for (int i = s; i <= e && e - i + 1 >= r - index; i++){
			tmparr[index] = arr[i];
			combination(arr, tmparr, r, i + 1, e, index + 1);
		}
	}
}

int cmp(const void * a, const void * b){
	Ans *tmpa = (Ans *)a;
	Ans *tmpb = (Ans *)b;
	if (tmpa -> score >= tmpb -> score)
		return -1;
	return 1;
}

int main(int argc, char** argv){
	int hostnum, playernum;
	hostnum =  atoi(argv[1]);
	playernum = atoi(argv[2]);

	char str[hostnum + 1][15];
	for (int i = 0; i < hostnum; i++){
		sprintf(str[i], "%s%d%s", "Host", i + 1, ".FIFO");
		mkfifo(str[i], 0666);
	}
	mkfifo("Host.FIFO", 0666);

	int fdarr[15];
    int fd1;

	int arr[15], tmparr[15];
    for (int i = 0; i < playernum; i++)
    	arr[i] = i + 1;

    int maxnum = cal(playernum);
    combination(arr, tmparr, 8, 0, playernum - 1, 0);
    int canuse[15] = {1};
    int key[15];

    for (int i = 0; i < hostnum; i++){
    	pid_t process;
		process = fork();
		key[i] = randomnumber(0, 65535);
		if (process == 0){
			char arg2[20], arg3[20], arg4[20];
    		srand(time(0)); 
			snprintf(arg2, sizeof(arg2), "%d", i + 1);
			snprintf(arg3, sizeof(arg3), "%d", key[i]);
			snprintf(arg4, sizeof(arg4), "%d", 0);
			int a = execlp("./host", "./host", arg2, arg3, arg4, NULL);
			printf("%d", a);
			_exit(0);
		}
    }

    int rank[20];

    Ans p[playernum];
    for (int i = 0; i < playernum; i++){
	    p[i].id = i + 1;
    	p[i].score = 0;
    }
    
    for (int i = 0; i < hostnum; i++)
		fdarr[i] = open(str[i], O_WRONLY);

    for (int i = 0; i < hostnum; i++){
		if (i >= maxnum)
			break;
    	write(fdarr[i], people[i], strlen(people[i]));
    	fsync(fdarr[i]);
    	canuse[i] = 0;
    }

    fd1 = open("Host.FIFO", O_RDONLY);
    FILE *fp = fdopen(fd1, "r");
    for (int i = 0; i < hostnum; i++){
	    if (i >= maxnum)
		    break;
		for (int j = 0; j < 17; j++)
			fscanf(fp, "%d", &rank[j]);

		for (int j = 1; j < 16; j += 2)
			p[rank[j] - 1].score += (8 - rank[j + 1]);

		for (int i = 0; i < hostnum; i++){
			if (rank[0] == key[i]){
				canuse[i] = 1;
				break;
			}
		}
    }

	int cnt = hostnum;
	while (cnt < maxnum){
		for (int j = 0; j < hostnum; j++){
			if (canuse[j] == 1){
				canuse[j] = 0;
				write(fdarr[j], people[cnt], strlen(people[cnt]));
				fsync(fdarr[j]);
				break;
			}
		}
		
		for (int j = 0; j < 17; j++)
			fscanf(fp, "%d", &rank[j]);

		for (int j = 0; j < hostnum; j++){
			if (rank[0] == key[j]){
				canuse[j] = 1;
				break;
			}
		}

		for (int j = 1; j < 16; j += 2)
			p[rank[j] - 1].score += (8 - rank[j + 1]);

		cnt++;
	}

	char finish[50];
	int symbol = -1;
	sprintf(finish, "%d %d %d %d %d %d %d %d\n", symbol, symbol, symbol, symbol, symbol, symbol, symbol, symbol);
	
	for (int i = 0; i < hostnum; i++){
		write(fdarr[i], finish, strlen(finish));
		fsync(fdarr[i]);
	}
	unlink("Host.FIFO");

	qsort(p, playernum, sizeof(Ans), cmp);

	int finalans[16];

	finalans[p[0].id] = 1;
	int count = 1;
	for (int i = 1; i < playernum; i++){
		count++;
		if (p[i].score == p[i - 1].score)
			finalans[p[i].id] = finalans[p[i - 1].id];
		else
			finalans[p[i].id] = count;
	}

	for (int i = 1; i <= playernum; i++)
		printf("%d %d\n", i, finalans[i]);

	for (int i = 0; i < hostnum; i++)
		close(fdarr[i]);
	close(fd1);

	for (int i = 0; i < hostnum; i++){
		int stat;
		wait(&stat);
	}
	for (int i = 0; i < hostnum; i++)
		unlink(str[i]);
	return 0;
}
