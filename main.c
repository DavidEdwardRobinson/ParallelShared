/* David Robinson der26 25/11/17
*Note: copied from windows notepad, may need to do dos2unix before compiling
*Code to solve a matrix n*n using relaxation technique
*Usage: function intSize intThreads
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

//arguments required for thread
struct threadArguments {
    size_t s;
    double **originalMatrix;
    double **workingMatrix;
    int startCol;
    int startRow;
    int n;                      //elements to be calculated by thread
};


//matrix copy function, only copies outside edges on initial copy
//edges don't change so no need to copy them after
void deepCopy(size_t s, double **matrix, double **matrixCopy, int initialFlag) {
    if (initialFlag == 1) {
        for (int i = 0; i < s; i++)
            for (int j = 0; j < s; j++)
                matrixCopy[i][j] = matrix[i][j];
    } else {
        for (int i = 1; i < (s - 1); i++)
            for (int j = 1; j < (s - 1); j++)
                matrixCopy[i][j] = matrix[i][j];
    }


}

void *threadSolver(void *args) {
    struct threadArguments *arg = (struct threadArguments *) args;
    double diff;
    double *biggestDiff = malloc(sizeof(double));
    int count = 0;                                            //count of elements to process
    int i = arg->startRow;                                    //pre calculated start row
    int j = arg-> startCol;                                   //pre calculated start col
    for (i; i < ((arg->s) - 1); i++) {
        for (j; j < ((arg->s) - 1); j++) {
            if (count < arg->n) {                           //if thread still has elements to calculate
                arg->workingMatrix[i][j] = ((arg->originalMatrix[(i - 1)][j] +
                        arg->originalMatrix[(i + 1)][j] +
                        arg->originalMatrix[i][(j + 1)] +    //calculate new element from adjacent
                        arg->originalMatrix[i][(j - 1)])/ 4.0);

                diff = arg->workingMatrix[i][j] - arg->originalMatrix[i][j]; //difference between og and calculated element
                if (diff > *biggestDiff || -diff>*biggestDiff) { *biggestDiff = diff; }                  //set biggest difference
                count++;
            } else{
                *biggestDiff=fabs(*biggestDiff);
                return biggestDiff;
            }
        }

        j = 1;                                                //after first loop, start col =1
    }
    *biggestDiff=fabs(*biggestDiff);
    return biggestDiff;                                      //return biggest diff for thread
}

void threadedSolver(size_t s, double **originalMatrix, int t, double p) {
    int i;
    double biggestDiff = p;
    double **workingMatrix = (double **) malloc(s * sizeof(double));
    for (i= 0; i < s; i++) {
        workingMatrix[i] = (double *) malloc(s * sizeof(double));
    }
    deepCopy(s, originalMatrix, workingMatrix, 1);              //initial copy

    int noElements = (int) floor(                                 //elements per thread = round down of elements to be calculated/ NoThreads
          (s - 2)*(s - 2) / t );
    int lastTElements = (int)  (s - 2)*(s - 2)                     //when last thread
                        -  ((t - 1) *  noElements);                 //elements = total elements - elements assigned so far

    struct threadArguments *argArray=(struct threadArguments *)     //array of arguments for thread
            malloc(t*sizeof(struct threadArguments));               //doesn't change for iteration, calculate outside main loop

    for (i=0; i<t; i++){
        argArray[i].originalMatrix = originalMatrix;
        argArray[i].workingMatrix = workingMatrix;
        argArray[i].s = s;
        argArray[i].startRow = (int) floor((i * noElements + 1) / (s - 2)) + 1; //Start Row: round down(startElement/elementsPerRow) + 1
        argArray[i].startCol =  (int) (i * noElements + 1)% (s - 2);            //Start Col: remainder (startElement/elementsPerRow)

        if (i != (t - 1)) {                                                  //if not last thread, normal elements/thread
            argArray[i].n = noElements;
        } else {
            argArray[i].n = lastTElements;                                  //last thread remaining elements
        }
    }
    pthread_t threads[t];

    while (biggestDiff >= p) {
        biggestDiff = 0.0;                                          //set to 0 else diff>biggest diff at end of loop will never assign
        for (i = 0; i < t; i++)
            pthread_create(&threads[i], NULL, threadSolver, &argArray[i]);//send work to thread solver

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
    free(argArray);
    for (i = 0; i < sizeof(workingMatrix[0]); i++) {                 //now done with matrix copy
        free(workingMatrix[i]);
    }
    free(workingMatrix);
}

//input format $size.txt same directory as executable
//file format: number\n
void readFromFile(size_t s, double **matrix) {
    char buffer[50];
    sprintf(buffer, "%d.txt", s);
    FILE *f = fopen(buffer, "r");
    int i, j;
    for (i = 0; i < s; i++)
        for (j = 0; j < s; j++)
            fscanf(f, "%lf", &matrix[i][j]);
    fclose(f);
}

//output format $sizeRT$threads.txt same directory as executable
//file format:csv matrixRow\n with time taken appended
void writeToFile(size_t s, double **matrix, float diff, int t) {
    char buffer[50];
    sprintf(buffer, "%dRT%d.txt", s, t);
    FILE *f = fopen(buffer, "w");
    int i, j;
    for (i = 0; i < s; i++) {
        for (j = 0; j < s; j++) {
            fprintf(f, "%f,", matrix[i][j]);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "Time taken: %f ms", diff);
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
    clock_t start, end;
    start = clock();
    threadedSolver(s, initialMatrix, t, p);
    end = clock();
    float timeMS = ((float) (end - start) / 1000000.0F) * 1000;
    writeToFile(s, initialMatrix, timeMS, t);

    for (int i = 0; i < sizeof(initialMatrix[0]); i++) {
        free(initialMatrix[i]);
    }
    free(initialMatrix);
}
