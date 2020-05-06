#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
double x[60000][784];
double weight[784][10];
double z[60000][10];
double y_hat[60000][10];
double sum[60000];
char label[60000];
int y[60000][10];
double wgrand[784][10];
int maxindex[60000];
double ztest[10000][10];
double xtest[10000][784];
char labeltest[10000];
int threadnum;

void* cal_multiply(void *upanddown){
	int *bound = (int *)upanddown;
	int start = bound[0];
	int end = bound[1];
	//printf("%d %d\n", start, end);
	for (int i = start; i < end; ++i){
		for (int j = 0; j < 10; ++j)
			z[i][j] = 0;
	}
	for (int i = start; i < end; ++i){
		for (int k = 0; k < 784; ++k){
			for (int j = 0; j < 10; ++j)
				z[i][j] += x[i][k] * weight[k][j];
		}
	}
	pthread_exit(NULL);
}

void funct_1(){
	for (int i = 0; i < 60000; ++i){
		for (int j = 0; j < 10 ; ++j)
			sum[i] = 0; 
	}
	for (int i = 0; i < 60000; ++i){
		double M = z[i][0];
		for (int j = 1; j < 10; ++j){
			if (z[i][j] > M)
				M = z[i][j];
		}
		for (int j = 0; j < 10; ++j)
			z[i][j] -= M;
	}
	for (int i = 0; i < 60000; ++i){
		for (int j = 0; j < 10 ; ++j)
			sum[i] += exp(z[i][j]);
	}
	for (int i = 0; i < 60000; ++i){
		for (int j = 0; j < 10; ++j)
			y_hat[i][j] = exp(z[i][j]) / sum[i];
	}
}

void construct_y(){
	for (int i = 0; i < 60000; ++i){
		for (int j = 0; j < 10; ++j)
			y[i][j] = 0;
	}
	for (int i = 0; i < 60000; ++i){
		int num = (int)label[i];
		y[i][num] = 1;
	}
}

void funct_4(){
	for (int i = 0; i < 784; ++i){
		for (int j = 0; j < 10; ++j)
			wgrand[i][j] = 0;
	}
	for (int k = 0; k < 60000; ++k){
		for (int i = 0; i < 784; ++i){
			for (int j = 0; j < 10; ++j)
				wgrand[i][j] += x[k][i] * (y_hat[k][j] - y[k][j]);
		}
	}
}

void cal_multiply_test(){
	for (int i = 0; i < 10000; ++i){
		for (int j = 0; j < 10; ++j)
			ztest[i][j] = 0;
	}
	for (int i = 0; i < 10000; ++i){
		for (int j = 0; j < 10; ++j){
			for (int k = 0; k < 784; ++k)
				ztest[i][j] += xtest[i][k] * weight[k][j];
		}
	}
}

int cal_correct_cnt(){
	int cnt = 0;
	for (int i = 0; i < 10000; ++i){
		double max = -10, ind;
		for (int j = 0; j < 10 ; ++j){
			if (ztest[i][j] > max){
				max = ztest[i][j];
				ind = j;
			}
		}
		if (ind == labeltest[i])
			cnt++;
	}
	return cnt;
}

void cal_correct(){
	FILE *ans = fopen("result.csv", "w");
	fprintf(ans, "id,label\n");
	for (int i = 0; i < 10000; ++i){
		double max = -10;
		int ind;
		for (int j = 0; j < 10 ; ++j){
			if (ztest[i][j] > max){
				max = ztest[i][j];
				ind = j;
			}
		}
		fprintf(ans, "%d,%d\n", i, ind);
	}
}

void funct_3(){
	for (int i = 0; i < 784; ++i){
		for (int j = 0; j < 10; ++j)
			weight[i][j] = weight[i][j] - 0.01 * wgrand[i][j];
	}
}

int main(int argc, char** argv){
	char s1[100], s2[100], s3[100];
	strcpy(s1, argv[1]);
	strcpy(s2, argv[2]);
	strcpy(s3, argv[3]);
	threadnum = atoi(argv[4]);
	int cnt = 60000 / threadnum;
	FILE* fp1 = fopen(s1, "r");
	FILE* fp2 = fopen(s2, "r");
	FILE* fp3 = fopen(s3, "r");
	//FILE* fp4 = fopen("y_test", "r");
	for (int i = 0; i < 60000; ++i){
		for (int j = 0; j < 784; ++j){
			unsigned char c;
			fscanf(fp1, "%c", &c);
			x[i][j] = (double)c;
		}
	}
	for (int i = 0; i < 784; ++i){
		for (int j = 0; j < 10; ++j)
			weight[i][j] = 0.0;
	}
	for (int i = 0; i < 60000; ++i)
		fscanf(fp2, "%c", &label[i]);
	construct_y();
	for (int i = 0; i < 10000; ++i){
		for (int j = 0; j < 784; ++j){
			unsigned char c;
			fscanf(fp3, "%c", &c);
			xtest[i][j] = (double)c;
		}
	}
	//for (int i = 0; i < 10000; ++i)
	//	fscanf(fp4, "%c", &labeltest[i]);
	for (int i = 0; i < 60; ++i){
		int upanddown[threadnum][2];
		upanddown[0][0] = 0;
		upanddown[0][1] = cnt;
		for (int j = 1; j < threadnum; j++){
			upanddown[j][0] = upanddown[j-1][1];
			upanddown[j][1] = upanddown[j][0] + cnt;
		}
		pthread_t t[60000];
		for (int j = 0; j < threadnum; j++)
			pthread_create(&t[j], NULL, cal_multiply, upanddown[j]);
		for (int j = 0; j < threadnum; ++j)
			pthread_join(t[j], NULL);
		funct_1();
		funct_4();
		funct_3();
	}
	cal_multiply_test();
	//int ct = cal_correct_cnt();
	cal_correct();
	//printf("%lf\n", ct/10000.0);
	return 0;
}