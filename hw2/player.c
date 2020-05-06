#include <stdio.h>
#include<stdlib.h> 
#include<unistd.h> 
#include<string.h> 

int main(int argc, char** argv){
	int playerid, money;
	playerid = atoi(argv[1]);
	money = 100 * playerid;
	char send_to_leaf[20], get_from_leaf[20];
	sprintf(send_to_leaf, "%d %d\n", playerid, money);

	for (int i = 0; i < 10; i++){
		if (i != 0){
			scanf("%s", &get_from_leaf);
		}
		printf("%s", send_to_leaf);
		fflush(stdout);
	}

    return 0;
}
