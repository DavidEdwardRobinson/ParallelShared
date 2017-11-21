#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include<pthread.h>

struct threadArguments {
    size_t s;
    double **originalMatrix;
    double **workingMatrix;
    int start;          //Start variable refers to the ith element to be calculated, not position in array
    int n;
    double p;
};

void populateMatrix(size_t s, double matrix[s][s]) {
    int i, j;
    double r;
    for (i = 0; i < s; i++)
        for (j = 0; j < s; j++) {
            r = rand() % 10;
            matrix[i][j] = r;
        }
}

void printMatrix(size_t s, double matrix[s][s]) {
    int i, j;
    for (i = 0; i < s; i++)
        for (j = 0; j < s; j++)
            printf(j == s ? "%f\t\n" : "%f\t", matrix[i][j]);
}

void writeToFile(size_t s, double matrix[s][s]) {
    FILE *f = fopen("matrixResultThreaded.txt", "w");
    int i, j;
    for (i = 0; i < s; i++)
        for (j = 0; j < s; j++)
            fprintf(f, "%f\n", matrix[i][j]);
    fclose(f);
}

void readFromFile(size_t s, double matrix[s][s]) {
    FILE *f = fopen("matrix.txt", "r");
    int i, j;
    for (i = 0; i < s; i++)
        for (j = 0; j < s; j++)
            fscanf(f, "%lf", &matrix[i][j]);
    fclose(f);
}

void deepCopy(size_t s, double** matrix, double** matrixCopy){
    for (int i=0; i < s; i++) {
        for (int j = 0; j < s; j++) {
            matrixCopy[i][j]=matrix[i][j];
        }
    }
}

void * threadSolver( void* args ){
    struct threadArguments *arg=(struct threadArguments*)args;
    //will be ran simultaneosuly by multiple threads, need to protect memory??
    double a, b, c, d, diff, element;
    double * biggestDiff=malloc(sizeof(double));
    int i=(int) floor(arg->start/(arg->s-2))+1;
    int j=  arg->start%( arg->s-2); //only from initial J in first i loop
    int count=0;

    for (i; i < ( arg->s-1); i++) {
        for (j; j < ( arg->s-1); j++) {
            if (count< arg->n){
                a = arg->originalMatrix[(i - 1)][j];
                b = arg->originalMatrix[(i + 1)][j];
                c = arg->originalMatrix[i][(j + 1)];
                d = arg->originalMatrix[i][(j - 1)];
                element = ((a + b + c + d) / 4.0);
                arg->workingMatrix[i][j] = element;
                diff = fabs( arg->workingMatrix[i][j] -  arg->originalMatrix[i][j]);
                //could get rid of if check and have variable for j condition, in last i loop set to n-(i*(s-2))
                if (diff > *biggestDiff) { *biggestDiff = diff;}
                count++;
            }
        }
        j=1;
    }

    return biggestDiff; //free later
}


void threadedSolver(size_t s, double originalMatrix[s][s], int t, double p){
    double biggestDiff=p;
    int i, elements, start;
    double ** workingMatrix=(double **)malloc(s * sizeof(double));
    for (int i=0; i<s; i++){
        workingMatrix[i]=(double *)malloc(s *sizeof(double));
    }

    while (biggestDiff >= p) {
        biggestDiff = 0.0;
        deepCopy(s, originalMatrix, workingMatrix);
        pthread_t threads[t];
        //array of thread addresses

        for (i=0; i<t; i++){
            struct threadArguments* arg=malloc(sizeof(struct threadArguments));
            arg->p=p;
            arg->originalMatrix=originalMatrix;
            arg->workingMatrix=workingMatrix;   //same
            arg->s = s;
            if (i!=(t-1)){
                arg->n=(int) floor((pow((s-2),2)/t));
                arg->start=(elements*t)+1;
            } else {
                arg->n=(int) modf( pow((s-2),2),t);
                arg->start=start=pow((s-2),2) - elements +1;
            }
            pthread_create(threads[i],NULL, threadSolver,arg );
            //biggestDiff=greatest of all threads, should not go past here until all threads complete
        }
        for (i=0; i<t;i++){
            double* diff;
            pthread_join(threads[i],diff);
            if (*diff>biggestDiff){
                biggestDiff=*diff;
            }
        }
        deepCopy(s, workingMatrix, originalMatrix);
    }
}


int main() {
    const size_t s = 40;
    int t = 1;
    float p = 0.001;
    double ** initialMatrix=(double **)malloc(s * sizeof(double));
    for (int i=0; i<s; i++){
        initialMatrix[i]=(double *)malloc(s *sizeof(double));
    }

    readFromFile(s, initialMatrix);
    threadedSolver(s, initialMatrix, t, p);
    //free later
}





