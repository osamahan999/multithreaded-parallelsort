#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

int validPartition(int partition); //checks if int is a valid number for a partition input
int inputCheck(int argc, char *argv[]); //checks all input 
void initializeArray(int *arr, int total); //initializes arr with random ints < 10000
void printArr(int *arr, int total); //prints arr
int cmpintp(const void *p1, const void *p2, void *p3);
void *merge(void *mData); //given merge changed to take struct
void *sortFunc(void *pParam); //called with threading to sort partitions

//used for sortFunc
typedef struct {
    int *data;
    int total;
    int threadIndex;

} parameters;

//used for merge
typedef struct {
    int *arr;
    int *temp;
    int left;
    int middle;
    int right;
} mergeData;



int main(int argc, char *argv[]) {
    
    printf("CS149 Fall 2020 parallel sort from Osama Hanhan\n");


    int *arr; //will hold our numbers
    
    //initial arg total check
    if (argc != 3) {
        printf("3 args only\n");
        exit(0);
    }

    inputCheck(argc, argv); //input check


    int total = atoi(argv[1]); //argv[1]
    int partitionTotal = atoi(argv[2]);

    arr = (int *) malloc(total * sizeof(int)); //malloc
    initializeArray(arr, total); //random nums in arr
    
    printf("Before: ");
    printArr(arr, total); //initial BEFORE print;

    //initial partition
    if (partitionTotal == 1) {
        qsort_r(&arr[0], total, sizeof(arr[0]), cmpintp, NULL);
    } else {

        pthread_t tid[partitionTotal]; //partitions tid arr


        int jumptotal = total / partitionTotal - 1;
        int threadNum = 0; //counter for tid
        

        for (int i = jumptotal; i < total; i = i + jumptotal + 1) {
            parameters *pParam = (parameters *) malloc(sizeof(parameters));

            
            pParam->data = &arr[i - jumptotal];
            pParam->total = jumptotal + 1;
            pParam->threadIndex = threadNum;



            pthread_create(&tid[threadNum], NULL, sortFunc, (void *)pParam);
            threadNum++;


        }

        for (int i = 0; i < threadNum; i++) {
            pthread_join(tid[i], NULL);   
        }



        int numOfIterations = 0; //total of iterations for merge
        int tempPartitionTotal = partitionTotal; //temp variable to not change original values


        //gets log2(partitiontotal)
        while (tempPartitionTotal != 1) {
            if (tempPartitionTotal % 2 != 0) break;

            numOfIterations++;
            tempPartitionTotal /= 2;
        }


        int *temp = (int *) malloc(total * sizeof(int)); //malloc temp arr

        for (int i = 1; i <= numOfIterations; i++) {
            int mergePartition = total/partitionTotal; //used for finding amt of iteration merges
            int tempExponential = i; //temp variable to do exponential math

            pthread_t tid2[total / mergePartition]; //merge tid arr
            int tidCounter = 0; //tid counter for tid2

            while (tempExponential != 0) {
                mergePartition *= 2;
                tempExponential--;
            }
            


            for (int j = 0; j <= total - mergePartition; j += mergePartition) {
                mergeData *mData = (mergeData *) malloc(sizeof(mergeData));

                mData->arr = arr;
                mData->temp = temp;
                mData->left = j;
                mData->middle = (j + j + mergePartition)/2 ;
                mData->right = j + mergePartition;
    
                printf("merge[%d,%d]:\t base=%p,\t left=%d,\t middle=%d,\t right=%d\n", i - 1, tidCounter, &(arr), mData->left, mData->middle, mData->right);

                pthread_create(&tid2[tidCounter], NULL, merge, (void *)mData);
                tidCounter++;


            }


            for (int k = 0; k < tidCounter; k++) {
                pthread_join(tid2[k], NULL);
            }

        }

        free(temp);

    }

    printf("After: ");
    printArr(arr, total);

    free(arr);

}


//given merge function
void *merge(void *mData) {

    mergeData *param = (mergeData *)mData;

    int i = param->left, j = param->middle, k = param->left;
    int *a = param->arr;
    int *tmp = param->temp;

    while (i < param->middle || j < param->right) {
        if (i < param->middle && j < param->right) {
            if (a[i] < a[j]) tmp[k++] = a[i++];
            else tmp[k++] = a[j++]; 
        } else if (i == param->middle) tmp[k++] = a[j++];
        else if (j == param->right) tmp[k++] = a[i++]; 
    }

    for (i = param->left; i < param->right; i++) {
        a[i] = tmp[i];
    }
    
    free(param);
    pthread_exit(0);

}

void *sortFunc(void *pParam) {
    

    
    parameters *param = (parameters *)pParam; //temp variable to cast to

    qsort_r(param->data, param->total, sizeof(int), cmpintp, NULL);
    printf("Sort[%d]:\t base=%p,\t nmemb=%d,\t size=%ld\n", param->threadIndex, &(param->data), param->total, sizeof(*param->data));

    free(param);
    pthread_exit(0);
}




//given comparison function
int cmpintp(const void *p1, const void *p2, void *p3) {
    int v1 = *((int *)p1);
    int v2 = *((int *)p2);
    
    return (v1 -v2);
}




//prints the array
void printArr(int *arr, int total) {
    int i;
    for (i = 0; i < total; i++) printf("%d ", (int) arr[i]);
    printf("\n");
}

//initializes the array with random values < 10000
void initializeArray(int *arr, int total) {
    int i;

    //psuedo random seed with time
    struct timeval time;
    gettimeofday(&time, NULL);

    int t = time.tv_usec * 5; //random math to maybe make it more chaotic?

    for (i = 0; i < total; i++) {
        arr[i] = rand_r(&t) % 10000;
    }
}


//checks all input conditions
int inputCheck(int argc, char *argv[]) {


    if ((atoi(argv[1]) < 1) | (atoi(argv[2]) < 1)) {
        printf("Input invalid\n");
        exit(0);
    }

    if (!validPartition(atoi(argv[2]))) {
        printf("# of partitions not 2^n \n");
        exit(0);
    } 
    if (atoi(argv[1]) % atoi(argv[2]) != 0) {
        printf("# of ints not divisible by # of partitions \n");
        exit(0);
    } 
}

//checks if = 1 or if log2(n)
int validPartition(int partition) {
    if (partition == 1) return 1; //1 partition works

    while (partition != 1) {
        if (partition % 2 != 0) return 0;
        partition /= 2;
    }

    return 1;
}