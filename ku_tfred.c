#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

/* 
        pread(fd, tempBuffer, 5, offset); 
        numToRead = atoi(tempBuffer); offset+=5; 
        printf("%d\n", numToRead); 
 
        pread(fd, tempBuffer, 5, offset); 
        temp = atoi(tempBuffer); offset += 5; 
        printf("%d\t", temp); 
 
        pread(fd, tempBuffer, 5, offset); 
        temp = atoi(tempBuffer); offset += 5; 
        printf("%d\t", temp); 
*/ 

pthread_mutex_t mutex;

struct Thread_arg{
        int init_offset;
        int fd;
        int operationNum;
        int interval;
        int *arr;
};


void *calculate_Freq(void *send);
int readDigits(int numToRead);

int main(int argc, char *argv[]) 
{ 
        pthread_mutex_init(&mutex, NULL);

        pthread_t *pthread_arr;
        
        int thr_id;
        int status;

        int i;
        int threadNum = atoi(argv[1]); 
        /*
        *struct Thread_arg *send = (struct Thread_arg *)malloc(sizeof(struct Thread_arg));
        *구조체 배열을 사용해야함.. 왜..?
        */
        struct Thread_arg **send = (struct Thread_arg **)malloc(sizeof(struct Thread_arg *) * threadNum);
        
        int interval = atoi(argv[2]); 

        int numToRead; 
        int operationNum; 
        int arr_indexNum;
 
        int temp; 
 
        char tempBuffer[6]; 
        int offset = 0; 
        char *fileName = (char *)malloc((sizeof(char) * strlen(argv[3]))+1); 
        memset(fileName, '\0', strlen(argv[3])); 
        strcpy(fileName, argv[3]); 
 
        //strcat(fileName, ".txt"); 
        //printf("%s\n", fileName);
        //file descriptor is static. 
        int fd = open(fileName, O_RDONLY); 
        if(fd<0){ 
                printf("read fail\n"); 
                close(fd); 
                return -1; 
        } 


        pread(fd, tempBuffer, 5, offset);
        numToRead = atoi(tempBuffer);
        int digits = readDigits(numToRead);
        offset += (digits +1);
        
        //arr_indexNum : 한 array에 존재해야 하는 index 숫자들...
        arr_indexNum = 10000/interval + ((10000 % interval) != 0);

        //operationnum per thread.
        operationNum = numToRead/threadNum + ((numToRead % threadNum) != 0);

        int *freq_arr;
        int *offset_arr;

        //frequency array
        freq_arr = (int *)(malloc(sizeof(int) * arr_indexNum));

        //initial offset array. 
        offset_arr = (int *)(malloc(sizeof(int) * threadNum));

        //thread_arr = (pthread_t *)sizeof(malloc(sizeof(p_thread) * threadNum));

        if(!freq_arr || !offset_arr){
                printf("배열 생성 오류\n");
                return -1;
        }

        //initailize freq_arr to 0
        for(int i = 0; i < arr_indexNum; i++){
                freq_arr[i] = 0;
        }
        //initialize offsetArray

        offset_arr[0] = offset;
        for(int i = 1; i < threadNum; i++){
                //offset + 5 * (number of operations required for each thread)
                offset_arr[i] = (operationNum * 5) + offset_arr[i-1];
        }

        //send->init_offset = offset_arr;



        //Thread function start

        pthread_arr = (pthread_t *)malloc(sizeof(pthread_t) * threadNum);
        if(!pthread_arr){
                printf("pthread malloc failed\n");
                return -1;
        }

        for(int i=0; i<threadNum; i++)
        {
                send[i] = (struct Thread_arg *)malloc(sizeof(struct Thread_arg));
                send[i]->arr = freq_arr;
                send[i]->fd = fd;
                send[i]->interval = interval;
                send[i]->operationNum = operationNum;
                send[i]->init_offset = offset_arr[i];
        }

        /*
        printf("--arr_indexNum : %d interval: %d, operationNum: %d--\n", arr_indexNum, interval, operationNum);
        send->arr = freq_arr;    send->fd = fd;      send->interval = interval;    send->operationNum = operationNum;
        */

        for(int i = 0; i < threadNum; i++)
        {       /*
                pthread_mutex_lock(&mutex);
                send->init_offset = offset_arr[i];
                pthread_mutex_unlock(&mutex);
                */

                //printf("sending offset : %d...\n", send->init_offset);
                thr_id = pthread_create(&pthread_arr[i], NULL, calculate_Freq, (void *)send[i]);
                //printf("send_offset : %d\n", send->init_offset);

                if(thr_id < 0){
                        perror("pthread_create()");
                        exit(0);
                }

        }
        

        for(int i=0; i< threadNum; i++){
                
                if(pthread_join(pthread_arr[i], (void**)&status) != 0)
                {
                        printf("join failed\n");
                        return -1;
                }
                
        }

        for(int i=0; i< arr_indexNum; i++)
        {
                printf("%d\n", freq_arr[i]);
        }
        return 0;
} 
           

void *calculate_Freq(void *send){
       
        int offset = ((struct Thread_arg *)send)->init_offset;

        int fd = ((struct Thread_arg *)send)->fd;
        int interval = ((struct Thread_arg *)send)->interval;
        int operationNum = ((struct Thread_arg *)send)->operationNum;
        int *arr = ((struct Thread_arg *)send)->arr;

        int num;
        char tempBuffer[6];
        
        for(int i=0; i < operationNum; i++){
                if(pread(fd, tempBuffer, 5, offset) == 0){
                        return NULL;
                }
                num = atoi(tempBuffer);

                pthread_mutex_lock(&mutex);
                arr[num/interval] = arr[num/interval] + 1;
                pthread_mutex_unlock(&mutex);

                offset = offset + 5;
        }
        
        return NULL;
}

int readDigits(int numToRead){
        int i = 0;
        while(numToRead != 0){
                numToRead/=10;
                i++;
        }
        return i;
}

