#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

void * threadSolver(size_t s, double** originalMatrix, double** workMatrix, int start, int n, double p ){
    //will be ran simultaneosuly by multiple threads, need to protect memory??
    double a, b, c, d, diff, element;
    double biggestDiff = p;
    int i=(int) floor(start/(s-2))+1;
    int j= start%(s-2); //only from initial J in first i loop
    int count=0;

    for (i; i < (s-1); i++) {
        for (j; j < (s-1); j++) {
            if (count<n){
                a = matrix[(i - 1)][j];
                b = matrix[(i + 1)][j];
                c = matrix[i][(j + 1)];
                d = matrix[i][(j - 1)];
                element = ((a + b + c + d) / 4.0);
                workingMatrix[i][j] = element;
                diff = fabs(workingMatrix[i][j] - matrix[i][j]);
                //could get rid of if check and have variable for j condition, in last i loop set to n-(i*(s-2))
                if (diff > biggestDiff) { biggestDiff = diff;}
                count++;
            }
        }
        j=1;
    }
    return *biggestDiff;
}


void threadedSolver(size_t s, double originalMatrix[s][s], int t, double p){
    double biggestDiff=p;
    int i, elements, start;
    while (biggestDiff >= p) {
        biggestDiff = 0.0;
        for (i=0; i<t; i++){
            if (i!=(t-1)){
                elements=(int) floor((pow((s-2),2)/t));
                start=(elements*t)+1;
            } else {
                elements=(int) modf( pow((s-2),2),t);
                start=pow((s-2),2) - elements +1;
            }
            create thread (threadSolver size_t s, **og, **working, start, elements, p);
            //need a struct to encapsulate all arguments? can you pass multiple args to p thread?
            //biggestDiff=greatest of all threads, should not go past here until all threads complete
        }

        memcpy(workingMatrix, initialMatrix, sizeof(initialMatrix));
    }


    //create thread (threadSolver size_t s, **og, **working, index of start = elementsThread*t (not quite right), elements, p)



//Start variable refers to the ith element to be calculated, not position in array

}

int main() {
    const size_t s = 40;
    int t = 1;
    float p = 0.001;
    double initialMatrix[s][s];
    double workingMatrix[s][s];
    readFromFile(s, initialMatrix);
   // solveMatrix(s, *initialMatrix, 1, p);

}







