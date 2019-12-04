#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mqueue.h>
#include <fcntl.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MQNAME  "/messageQueue"
/*
struct childSend{
int init_offset;
char *fileName;
int operationNum;
int interval;
int arr_indexNum;
};
*/

int readDigits(int numToRead);

int main(int argc, char *argv[])
    {

    mq_unlink(MQNAME);
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 8192;

    unsigned int prio;
    mqd_t mfd;
    //struct childSend *toSend = (struct childSend *)malloc(sizeof(struct childSend));
    int fd = open(argv[3], O_RDONLY);

    int i;
    int processNum, operationNum, interval, arr_indexNum, numToRead;
    int init_offset;

    char temp[30];
    char *fileName;
    fileName = (char *)malloc((sizeof(char) * strlen(argv[3])) + 1);
    memset(fileName, '\0', strlen(argv[3]));
    strcpy(fileName, argv[3]);

    processNum = atoi(argv[1]);
    interval = atoi(argv[2]);

    arr_indexNum = 10000 / interval + ((10000 % interval) != 0);

    int *freq_arr = (int *)malloc(sizeof(int) * arr_indexNum);
    int *offset_arr = (int *)malloc(sizeof(int) * processNum);

    for (i = 0; i < arr_indexNum; i++)
        freq_arr[i] = 0;

    int *temp_arr = (int *)malloc(sizeof(int) * arr_indexNum);
    memset(temp_arr, '\0', sizeof(int) * arr_indexNum);

    //operationnum per thread.
    pread(fd, temp, sizeof(int), 0);
    numToRead = atoi(temp);
    init_offset = readDigits(numToRead) + 1; //2인지 1인지 헷갈리누..


    //offset을 제대로 옮겼는지 확인해보기.

    operationNum = numToRead / processNum + ((numToRead % processNum) != 0);


    init_offset = readDigits(numToRead) + 1;


    //보내줄 구조체 초기화.
    /*
    toSend->arr_indexNum = arr_indexNum;
    strcpy(toSend->fileName, fileName);
    toSend->interval = interval;
    toSend->operationNum = operationNum;
    */

    mfd = mq_open(MQNAME, O_RDWR | O_CREAT, 0666, &attr);
    if (mfd == -1) {
        perror("open error");
        exit(0);
    }

    for(i=0; i < processNum; i++)
    {
        offset_arr[i] = init_offset + i * 5 * operationNum;
    }

    //정보 담고있는 프로세스 수만큼 보내기.
    for (i = 0; i < processNum; i++)
    {
    /*toSend->init_offset = init_offset + (i-1) * operationNum * 5;*/
        if (mq_send(mfd, (char *)(offset_arr+i), sizeof(offset_arr[i]), 32) == -1)
        {
            perror("mq_send()");
        }
    }

    pid_t pid[processNum];
    int child_status;



    //프로세스 분할 시작.
    for (i = 0; i < processNum; i++)
    {
        if ((pid[i] = fork()) == 0)
        {   
        //여기서부터 분할정복_분할 시작.
        //struct childSend received;
        /*
        mqd_t mq = mq_open(MQNAME, O_RDWR | O_CREAT, 0666, &attr);
        struct 
        if (mq == -1) {
            perror("open error");
            exit(0);
        }
        */
        unsigned int prio;
        int i, num;
        char tempBuffer[5];
        int offset;
        int *toSend_arr = (int *)malloc(sizeof(int) * arr_indexNum);
        memset(toSend_arr, 0, sizeof(int) * arr_indexNum);

        if(mq_receive(mfd, (char *)&offset, 10000, &prio) == -1){
            perror("mq_recieve");
        }

        for (int i = 0; i < operationNum; i++) {
            if (pread(fd, tempBuffer, 5, offset) == 0) {
                break;
            }
            num = atoi(tempBuffer);


            toSend_arr[num / interval] = toSend_arr[num / interval] + 1;
            offset = offset + 5;
        }

        if (mq_send(mfd, (char *)toSend_arr, sizeof(int) * arr_indexNum, 32) == -1) {
            perror("mq_send()");
        }
        close(fd);
        //mq_close(mfd);
        exit(0);
        }
    }

    for (i = 0; i < processNum; i++)
    {
        pid_t wpid = wait(&child_status);
        if (!WIFEXITED(child_status))
            return -1;
    }

    for (i = 0; i < processNum; i++)
    {
        if(mq_receive(mfd, (char *)temp_arr, 10000, &prio)==-1){
            perror("mq_receive()");
        }
        temp_arr = (int *)temp_arr;
        for (int j = 0; j < arr_indexNum; j++) {
            freq_arr[j] += temp_arr[j];
        }
    }

    mq_close(mfd);
    mq_unlink(MQNAME);
    close(fd);

    for (i = 0; i < arr_indexNum; i++)
    {
        printf("%d\n", freq_arr[i]);
    }

}

int readDigits(int numToRead) {
    int i = 0;
    while (numToRead != 0) {
        numToRead /= 10;
        i++;
        }
        return i;
}
