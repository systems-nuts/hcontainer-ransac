#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define NUM_OBJ 25000
int main(){
	srand(time(NULL));
	FILE* fp;

        if (!(fp=fopen("ransac_data", "w"))) {
                printf("can not opern file\n");
                return 1;
        }
	int i=0;
        for (i=0;i < NUM_OBJ; i++)
                fprintf(fp,"%f %f\n", rand()/1000000.0, rand()/10000000.);

        fclose(fp);
	return 0;
}
