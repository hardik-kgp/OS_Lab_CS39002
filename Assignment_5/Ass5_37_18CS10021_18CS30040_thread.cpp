/*Name: Hardik Aggarwal 
Roll : 18CS10021

Name: Sriyash Poddar
Roll: 18CS30040

*/

#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <pthread.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/time.h>
#include <queue>
#include <vector>
#include <chrono>

using namespace std;

#define QUEUE_SIZE 8
#define PRI_MAX 10
#define ID_MAX 100000
#define COMPUTE_MAX 4
#define WAIT_MAX 3

void error(string msg){
    perror(msg.c_str());
    exit(1);
}

void delay(time_t _time) 
{
    float temp = (float)_time;
    clock_t start_time = clock(); 
    while (clock() < start_time + temp*CLOCKS_PER_SEC); 
}

typedef struct _job
{
    pid_t prod_pid;
    int prod_num;
    int priority;
    time_t compute_time = 0;
    pid_t pid;
}JOB;


typedef struct _shared_data
{
    JOB job_queue[QUEUE_SIZE];
    int job_created;
    int job_completed;
    int max_size;
    int queue_size;
    int max_jobs ;
    pthread_mutex_t lock;
}SHARED_DATA;

SHARED_DATA *SHM;

void init_shared_data(int max_jobs){

    SHM = (SHARED_DATA *)(malloc(sizeof(SHARED_DATA)));
    SHM->job_created = 0;
    SHM->job_completed = 0;
    SHM->queue_size = 0;
    SHM->max_size = QUEUE_SIZE;
    SHM->max_jobs = max_jobs;

    // initialising mutex locking
    pthread_mutexattr_t lock_attr;
    pthread_mutexattr_init(&lock_attr);
    pthread_mutexattr_setpshared(&lock_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&SHM->lock, &lock_attr);
}

JOB pop(SHARED_DATA *SHM){
    int max_pos = 0;
    for(int i=1; i<SHM->queue_size; i++){
        if(SHM->job_queue[max_pos].priority < SHM->job_queue[i].priority)
            max_pos = i;
    }
    JOB j = SHM->job_queue[max_pos];
    SHM->queue_size -= 1;
    for(int i = max_pos; i<SHM->queue_size; i++){
        SHM->job_queue[i] = SHM->job_queue[i+1];
    }
    return j;
}

void push(SHARED_DATA *SHM, JOB j){
    SHM->job_queue[SHM->queue_size] = j;
    SHM->queue_size += 1;
}

JOB create_job(int prod_num){
    JOB temp;
    temp.prod_pid = getpid();
    temp.prod_num = prod_num;
    temp.priority = rand()%PRI_MAX + 1;
    temp.compute_time = rand()%COMPUTE_MAX + 1;
    temp.pid = rand()%ID_MAX + 1;
    return temp;
}

void printjob(JOB j)
{
    cout<<"Producer: "<<j.prod_num<<"\t";
    cout<<"Producer pid: "<<j.prod_pid<<"\t";
    cout<<"Priority: "<<j.priority<<"\t";
    cout<<"Compute Time: "<<j.compute_time<<"\t";
    cout<<"Job ID: "<<j.pid<<"\n\n";
}

bool insert_job(int prod_num){
    JOB temp = create_job(prod_num);
    bool done = false;
    while (true){
        pthread_mutex_lock(&SHM->lock);
        
        if(SHM->job_created == SHM->max_jobs){
            done = true;
            pthread_mutex_unlock(&SHM->lock);
            break;
        }
        else if(SHM->queue_size < SHM->max_size){
            push(SHM, temp);
            SHM->job_created += 1;
            if(SHM->job_created == SHM->max_jobs) done = true;
            printjob(temp);
            pthread_mutex_unlock(&SHM->lock);
            break;
        }
        pthread_mutex_unlock(&SHM->lock);
    }
    return done;
}

void *producer_process(void *num){
    bool done = false;
    int* prod_num = (int *)(num);
    while(!done){
        delay(rand()%4);
        done = insert_job(*prod_num);
    }
    
}

bool remove_job(int con_num){
    JOB temp;
    bool done = false;
    while (true){
        pthread_mutex_lock(&SHM->lock);
        if(SHM->job_completed == SHM->max_jobs){
            done = true;
            pthread_mutex_unlock(&SHM->lock);
            break;
        }
        else if(SHM->queue_size > 0){
            temp = pop(SHM);
            cout<<"Consumer: "<<con_num<<"\t";
            cout<<"Consumer pid: "<<getpid()<<"\t";
            printjob(temp);
            pthread_mutex_unlock(&SHM->lock);
            break;
        }
        pthread_mutex_unlock(&SHM->lock);
    }
    pthread_mutex_lock(&SHM->lock);
    delay(temp.compute_time);
    if(temp.compute_time != 0){
        SHM->job_completed += 1;
        if(SHM->job_completed == SHM->max_jobs) done = true;    
    }
    pthread_mutex_unlock(&SHM->lock);
    return done;
}

void *consumer_process(void* num){
    bool done = false;
    int *con_num = (int *)(num);
    while(!done){
        delay(rand()%4);
        done = remove_job(*con_num);
    }
    
}

int main(){
    srand(time(0));

    int NP, NC, max_jobs;
    cout << "Enter number of producers: ";
    cin >> NP;
    cout << "Enter number of consumers: ";
    cin >> NC;
    cout << "Enter number of total jobs: ";
    cin >> max_jobs;

    init_shared_data(max_jobs);
    auto begin = std::chrono::high_resolution_clock::now();
    pid_t pid;
    pthread_t* producer_list;
    pthread_t* consumer_list;

    producer_list = (pthread_t *)malloc(NP*sizeof(pthread_t));
    consumer_list = (pthread_t *)malloc(NC*sizeof(pthread_t));
    
    for(int i=0; i<NP; i++){
        int *x = new int(i+1);
        pthread_create(&producer_list[i], NULL, &producer_process, (void *)(x));
    }

    for(int i = 0; i<NC; i++){
        int *x = new int(i+1);
        pthread_create(&consumer_list[i], NULL, &consumer_process, (void *)(x));
        
    }

    for(int i =0 ;i<NP;i++){
        pthread_join(producer_list[i], NULL);
    }
    for(int i = 0;i<NC;i++){
        pthread_join(consumer_list[i], NULL);
    }

    while(true){        
        pthread_mutex_lock(&SHM->lock);
        if((SHM->job_completed) == max_jobs && (SHM->job_created) == max_jobs){
            auto consumed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-begin);
            cout<<"Jobs Executed: "<< max_jobs <<" Time Taken "<< (float)consumed.count()/1000000 <<"s."<<endl;
            pthread_mutex_unlock(&SHM->lock);
            break;
        }
        pthread_mutex_unlock(&SHM->lock);
    }
    
    return 0;
}