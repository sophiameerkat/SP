#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>
#include "scheduler.h"
#define SIGUSR3 SIGWINCH

char arr[10000];
jmp_buf env, SCHEDULER;
FCB_ptr Current, Head;
FCB fcb[5];
int P, Q, task, release_time, idx = 0, mutex = 0, cnt1 = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0;
int qu[5];

sigset_t newmask, unblockmask;
void handler(int signo){
	if (signo == SIGUSR1 || signo == SIGUSR2){
		char str[50] = "get signal";
		write(1, str, sizeof(str));
		sigprocmask(SIG_BLOCK, &newmask, NULL);
		longjmp(SCHEDULER, 1);
	}
	else{
		char str[50] = "";
		int index = 0;
		for (int i = 1; i <= 4; i++){
			if (qu[i] == 1)
				index += sprintf(str+index, "%d", i);
		}
		write(1, str, sizeof(str));
		Current = Current -> Previous;
		sigprocmask(SIG_BLOCK, &newmask, NULL);
		longjmp(SCHEDULER, 1);
	}
}

void unblock(){
	sigprocmask(SIG_SETMASK, &unblockmask, NULL);
}

int main(int argc, char** argv){
	P = atoi(argv[1]);
	Q = atoi(argv[2]);
	task = atoi(argv[3]);
	release_time = atoi(argv[4]);
	for (int i = 1; i <= 4; i++)
		qu[i] = 0;
	if (task == 3){
		signal(SIGUSR1, handler);
		signal(SIGUSR2, handler);
		signal(SIGUSR3, handler);
		sigemptyset(&newmask);
		sigemptyset(&unblockmask);
		sigaddset(&newmask, SIGUSR1);
		sigaddset(&newmask, SIGUSR2);
		sigaddset(&newmask, SIGUSR3);
	}
	int val = setjmp(env);
	if (val == 0){
		for (int i = 1; i <= 4; i++){
			fcb[i].Name = i;
			if (i != 4)
				fcb[i].Next = &fcb[i + 1];
			else if (i == 4)
				fcb[i].Next = &fcb[1];
			if (i != 1)
				fcb[i].Previous = &fcb[i - 1];
			else if (i == 1)
				fcb[i].Previous = &fcb[4];
		}
		Head = &fcb[1];
		Current = fcb[1].Previous;
		funct_5(1);
	}
	else if (val == 1)		
		Scheduler();
}

void funct_1(int name){
	int val1 = setjmp(fcb[name].Environment);
	if (val1 == 0)
		funct_5(2);
	else if (val1 == 1){
		if (task == 1){
			for (int i = 1; i <= P; i++){
				for (int j = 1; j <= Q; j++){
					sleep(1);
					arr[idx] = '1';
					idx++;
				}
			}
			longjmp(SCHEDULER, -2);
		}
		else if (task == 2){
			for (int i = 1; i <= P; i++){
				if (mutex == 2 || mutex == 3 || mutex == 4){
					int fl1 = setjmp(fcb[name].Environment);
					if (fl1 != 0)
						continue;
					longjmp(SCHEDULER, 1);
				}
				else {
					mutex = 1;
					for (int j = 1; j <= Q; j++){
						sleep(1);
						arr[idx] = '1';
						idx++;
					}
				}
				if (i % release_time == 0){
					mutex = 0;
					int f1 = setjmp(fcb[name].Environment);
					if (f1 != 0)
						continue;
					if (i != P)
						longjmp(SCHEDULER, 1);
				}
				if (i == P){
					mutex = 0;
					int f11 = setjmp(fcb[name].Environment);
					if (f11 == 0)
						longjmp(SCHEDULER, -2);
				}
			}
		}
		else if (task == 3){
			static int cnt1 = 1;
			if (mutex == 2 || mutex == 3 || mutex == 4){
				qu[1] = 1;
				longjmp(SCHEDULER, 1);
			}
			else {
				qu[1] = 0;
				mutex = 1;
				for (int i = cnt1; i <= P; i++){
					for (int j = 1; j <= Q; j++){
						sleep(1);
						arr[idx] = '1';
						idx++;
					}
					sigset_t pend;
					sigpending(&pend);
					if (sigismember(&pend, SIGUSR1)){
						cnt1 = i + 1;
						unblock();
					}
					else if (sigismember(&pend, SIGUSR2)){
						cnt1 = i + 1;
						mutex = 0;
						unblock();
					}
					else if (sigismember(&pend, SIGUSR3)){
						cnt1 = i + 1;
						unblock();
					}
				}
				mutex = 0;
				longjmp(SCHEDULER, -2);
			}
		}
	}
}

void funct_2(int name){
	int val2 = setjmp(fcb[name].Environment);
	if (val2 == 0)
		funct_5(3);
	else if (val2 == 1){
		if (task == 1){
			for (int i = 1; i <= P; i++){
				for (int j = 1; j <= Q; j++){
					sleep(1);
					arr[idx] = '2';
					idx++;
				}
			}
			longjmp(SCHEDULER, -2);
		}
		else if (task == 2){
			for (int i = 1; i <= P; i++){
				if (mutex == 1 || mutex == 3 || mutex == 4){
					int fl2 = setjmp(fcb[name].Environment);
					if (fl2 != 0)
						continue;
					longjmp(SCHEDULER, 1);
				}
				else {
					mutex = 2;
					for (int j = 1; j <= Q; j++){
						sleep(1);
						arr[idx] = '2';
						idx++;
					}
				}
				if (i % release_time == 0){
					mutex = 0;
					int f2 = setjmp(fcb[name].Environment);
					if (f2 != 0)
						continue;
					if (i != P)
						longjmp(SCHEDULER, 1);
				}
				if (i == P){
					mutex = 0;
					int f12 = setjmp(fcb[name].Environment);
					if (f12 == 0)
						longjmp(SCHEDULER, -2);
				}
			}
		}
		else if (task == 3){
			static int cnt2 = 1;
			if (mutex == 1 || mutex == 3 || mutex == 4){
				qu[2] = 1;
				longjmp(SCHEDULER, 1);
			}
			else {
				qu[2] = 0;
				mutex = 2;
				for (int i = cnt2; i <= P; i++){
					for (int j = 1; j <= Q; j++){
						sleep(1);
						arr[idx] = '2';
						idx++;
					}
					sigset_t pend;
					sigpending(&pend);
					if (sigismember(&pend, SIGUSR1)){
						cnt2 = i + 1;
						unblock();
					}
					else if (sigismember(&pend, SIGUSR2)){
						cnt2 = i + 1;
						mutex = 0;
						unblock();
					}
					else if (sigismember(&pend, SIGUSR3)){
						cnt2 = i + 1;
						unblock();
					}
				}
				mutex = 0;
				longjmp(SCHEDULER, -2);
			}
		}
	}
}

void funct_3(int name){
	int val3 = setjmp(fcb[name].Environment);
	if (val3 == 0)
		funct_5(4);
	else if (val3 == 1){
		if (task == 1){
			for (int i = 1; i <= P; i++){
				for (int j = 1; j <= Q; j++){
					sleep(1);
					arr[idx] = '3';
					idx++;
				}
			}
			longjmp(SCHEDULER, -2);
		}
		else if (task == 2){
			for (int i = 1; i <= P; i++){
				if (mutex == 1 || mutex == 2 || mutex == 4){
					int fl3 = setjmp(fcb[name].Environment);
					if (fl3 != 0)
						continue;
					longjmp(SCHEDULER, 1);
				}
				else {
					mutex = 3;
					for (int j = 1; j <= Q; j++){
						sleep(1);
						arr[idx] = '3';
						idx++;
					}
				}
				if (i % release_time == 0){
					mutex = 0;
					int f3 = setjmp(fcb[name].Environment);
					if (f3 != 0)
						continue;
					if (i != P)
						longjmp(SCHEDULER, 1);
				}
				if (i == P){
					mutex = 0;
					int f13 = setjmp(fcb[name].Environment);
					if (f13 == 0)
						longjmp(SCHEDULER, -2);
				}
			}
		}
		else if (task == 3){
			static int cnt3 = 1;
			if (mutex == 1 || mutex == 2 || mutex == 4){
				qu[3] = 1;
				longjmp(SCHEDULER, 1);
			}
			else {
				qu[3] = 0;
				mutex = 3;
				for (int i = cnt3; i <= P; i++){
					for (int j = 1; j <= Q; j++){
						sleep(1);
						arr[idx] = '3';
						idx++;
					}
					sigset_t pend;
					sigpending(&pend);
					if (sigismember(&pend, SIGUSR1)){
						cnt3 = i + 1;
						unblock();
					}
					else if (sigismember(&pend, SIGUSR2)){
						cnt3 = i + 1;
						mutex = 0;
						unblock();
					}
					else if (sigismember(&pend, SIGUSR3)){
						cnt3 = i + 1;
						unblock();
					}
				}
				mutex = 0;
				longjmp(SCHEDULER, -2);
			}
		}
	}
}

void funct_4(int name){
	int val4 = setjmp(fcb[name].Environment);
	if (val4 == 0)
		longjmp(env, 1);
	else if (val4 == 1){
		if (task == 1){
			for (int i = 1; i <= P; i++){
				for (int j = 1; j <= Q; j++){
					sleep(1);
					arr[idx] = '4';
					idx++;
				}
			}
			longjmp(SCHEDULER, -2);
		}
		else if (task == 2){
			for (int i = 1; i <= P; i++){
				if (mutex == 1 || mutex == 2 || mutex == 3){
					int fl4 = setjmp(fcb[name].Environment);
					if (fl4 != 0)
						continue;
					longjmp(SCHEDULER, 1);
				}
				else {
					mutex = 4;
					for (int j = 1; j <= Q; j++){
						sleep(1);
						arr[idx] = '4';
						idx++;
					}
				}
				if (i % release_time == 0){
					mutex = 0;
					int f4 = setjmp(fcb[name].Environment);
					if (f4 != 0)
						continue;
					if (i != P)
						longjmp(SCHEDULER, 1);
				}
				if (i == P){
					mutex = 0;
					int f14 = setjmp(fcb[name].Environment);
					if (f14 == 0)
						longjmp(SCHEDULER, -2);
				}
			}
		}
		else if (task == 3){
			static int cnt4 = 1;
			if (mutex == 1 || mutex == 2 || mutex == 3){
				qu[4] = 1;
				longjmp(SCHEDULER, 1);
			}
			else {
				qu[4] = 0;
				mutex = 4;
				for (int i = cnt4; i <= P; i++){
					for (int j = 1; j <= Q; j++){
						sleep(1);
						arr[idx] = '4';
						idx++;
					}
					sigset_t pend;
					sigpending(&pend);
					if (sigismember(&pend, SIGUSR1)){
						cnt4 = i + 1;
						unblock();
					}
					else if (sigismember(&pend, SIGUSR2)){
						cnt4 = i + 1;
						mutex = 0;
						unblock();
					}
					else if (sigismember(&pend, SIGUSR3)){
						cnt4 = i + 1;
						unblock();
					}
				}
				mutex = 0;
				longjmp(SCHEDULER, -2);
			}
		}
	}
}

void funct_5(int name){
	int a[10000];
	if (name == 1)
		funct_1(1);
	else if (name == 2)
		funct_2(2);
	else if (name == 3)
		funct_3(3);
	else if (name == 4)
		funct_4(4);
}
