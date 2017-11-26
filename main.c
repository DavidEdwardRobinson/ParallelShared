#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

struct threadArguments {
    size_t s;
    double **originalMatrix;
    double **workingMatrix;
    int start;                  //Start variable is ith element to be calculated(1 is first element), not position in array
    int n;                      //elements to be calculated by thread
};



//matrix copy function, only copies outside edges on initial copy
void deepCopy(size_t s, double **matrix, double **matrixCopy, int initialFlag) {
    if (initialFlag == 1){
        for (int i=0; i < s; i++)
            for (int j=0; j < s; j++)
                matrixCopy[i][j] = matrix[i][j];
    } else {
        for (int i=1; i < (s-1); i++)
            for (int j=1; j < (s-1); j++)
                matrixCopy[i][j] = matrix[i][j];
    }


}

void *threadSolver(void *args) {
    struct threadArguments *arg = (struct threadArguments *) args;
    double a, b, c, d, diff, element;
    double *biggestDiff = malloc(sizeof(double));
    int i = (int) floor(arg->start / (arg->s - 2)) + 1;     //Start Row: round down(startElement/elementsPerRow) + 1
    int j = (int) (arg->start % (arg->s - 2));               //Start Col: remainder (startElement/elementsPerRow)
    int count = 0;                                            //count of elements to process

    for (i; i < ((arg->s) - 1); i++) {
        for (j; j < ((arg->s) - 1); j++) {
            if (count < arg->n) {                           //if thread still has elements to calculate
                a = arg->originalMatrix[(i - 1)][j];
                b = arg->originalMatrix[(i + 1)][j];
                c = arg->originalMatrix[i][(j + 1)];
                d = arg->originalMatrix[i][(j - 1)];
                element = ((a + b + c + d) / 4.0);          //calculate new element from adjacent
                arg->workingMatrix[i][j] = element;
                diff = fabs(arg->workingMatrix[i][j] - arg->originalMatrix[i][j]); //difference between og and calculated element
                if (diff > *biggestDiff) { *biggestDiff = diff; }                  //set biggest difference
                count++;
            }
        }

        j = 1;                                                //after first loop, start col =1
    }
    return biggestDiff;                                      //return biggest diff for thread
}

void threadedSolver(size_t s, double **originalMatrix, int t, double p) {
    double biggestDiff = p;
    int i;
    double **workingMatrix = (double **) malloc(s * sizeof(double));
    for (i = 0; i < s; i++) {
        workingMatrix[i] = (double *) malloc(s * sizeof(double));
    }
    deepCopy(s, originalMatrix, workingMatrix, 1);


    while (biggestDiff >= p) {
        biggestDiff = 0.0;                                          //set to 0 else diff>biggest diff at end of loop will never assign
        pthread_t threads[t];
        int noElements = (int) floor((pow((s - 2), 2) / t));       //elements per thread = round down of elements to be calculated/ NoThreads
                                                                    //if t> elements to be calculated, last thread will do all the work
        for (i = 0; i < t; i++) {
            struct threadArguments *arg = malloc(sizeof(struct threadArguments));
            arg->originalMatrix = originalMatrix;
            arg->workingMatrix = workingMatrix;
            arg->s = s;
            arg->start = i * noElements + 1;

            if (i != (t - 1)) {                                 //if not last thread, normal elements/thread
                arg->n = noElements;
            } else {                                            //when last thread
                arg->n = (int) pow((s - 2), 2) -
                         (i * noElements);                      //elements = total elements - elements assigned so far

            }
            pthread_create(&threads[i], NULL, threadSolver, arg);//send work to thread solver
            // free(arg);                                        //can't free used by thread, can't free outside loop var not found?
        }


        for (i = 0; i < t; i++) {
            double *diff = NULL;
            pthread_join(threads[i], (void **) &diff);            //wait for threads to complete
            if (*diff > biggestDiff) {                             //biggest diff from all threads=biggest diff
                biggestDiff = *diff;
            }
            free(diff);
        }

        deepCopy(s, workingMatrix, originalMatrix, 0);             //copy the working matrix onto original matrix
    }
    for (i=0; i<sizeof(workingMatrix[0]); i++){                 //now done with matrix copy
        free(workingMatrix[i]);
    }
    free(workingMatrix);
}



void readFromFile(size_t s, double **matrix) {
    char buffer [50];
    sprintf(buffer, "%d.txt", s);
    FILE *f = fopen(buffer, "r");
    int i, j;
    for (i = 0; i < s; i++)
        for (j = 0; j < s; j++)
            fscanf(f, "%lf", &matrix[i][j]);
    fclose(f);
}

void writeToFile(size_t s, double **matrix, float diff, int t) {
    char buffer[50];
    sprintf(buffer, "%dRT%d.txt", s,t);
    FILE *f = fopen(buffer, "w");
    int i, j;
    for (i = 0; i < s; i++){
        for (j = 0; j < s; j++) {
            fprintf(f, "%f,", matrix[i][j]);
        }
        fprintf(f, "\n");
    }
    fprintf(f,"Time taken: %f ms", diff);
    fclose(f);
}


//usage:Call, arg1 size, arg2 threads
int main(int argc, char *argv[]) {
    const size_t s = atoi(argv[1]);
    int t = atoi(argv[2]);
    double p = 0.001;
    double **initialMatrix = (double **) malloc(s * sizeof(double));
    for (int i = 0; i < s; i++) {
        initialMatrix[i] = (double *) malloc(s * sizeof(double));
    }
    readFromFile(s, initialMatrix);
    clock_t start,end;
    start=clock();
    threadedSolver(s, initialMatrix, t, p);
    end=clock();
    float timeMS = ((float)(end-start) / 1000000.0F ) * 1000;
    writeToFile(s, initialMatrix, timeMS, t);

    for (int i=0; i<sizeof(initialMatrix[0]); i++){
        free(initialMatrix[i]);
    }
    free(initialMatrix);
}
