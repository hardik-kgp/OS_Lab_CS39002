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
    time_t compute_time;
    pid_t pid;
}JOB;

struct ComparePriority{
    bool operator()(JOB const& a, JOB const& b){
        return a.priority < b.priority;
    }  
};

typedef struct _shared_data
{
    priority_queue <JOB, vector<JOB>, ComparePriority> job_queue;
    int job_created;
    int job_completed;
    int max_size;
    int max_jobs;
    pthread_mutex_t lock;
}SHARED_DATA;

int create_shared_data(){
    key_t key1 = ftok("/dev/random", 'b');
	int shmid = shmget(key1,sizeof(SHARED_DATA),0660|IPC_CREAT);
	if (shmid<0) {
		error("Failed to allocate shared memory!\n");
	}
    return shmid;
}

SHARED_DATA *get_shared_data(int shmid, int max_jobs){
    SHARED_DATA *SHM = (SHARED_DATA *)shmat(shmid, (void *)0, 0);
    SHM->job_created = 0;
    SHM->job_completed = 0;
    SHM->max_size = QUEUE_SIZE;
    SHM->max_jobs = max_jobs;

    // initialising mutex locking
	pthread_mutexattr_t lock_attr;
	pthread_mutexattr_init(&lock_attr);
	pthread_mutexattr_setpshared(&lock_attr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&SHM->lock, &lock_attr);

	return SHM;
}

JOB create_job(pid_t prod_pid, int prod_num){
    JOB temp;
    temp.prod_pid = prod_pid;
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

bool insert_job(SHARED_DATA *SHM, pid_t prod_pid, int prod_num){
    JOB temp = create_job(prod_pid, prod_num);
    bool done = false;
    while (true){
        pthread_mutex_lock(&SHM->lock);
        
        if(SHM->job_created == SHM->max_jobs){
            done = true;
            pthread_mutex_unlock(&SHM->lock);
            break;
        }
        else if(SHM->job_queue.size() < SHM->max_size){
            SHM->job_queue.push(temp);
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

void producer_process(SHARED_DATA *SHM, int pid, int num){
    bool done = false;
    while(!done){
        delay(rand()%4);
        done = insert_job(SHM, pid, num);
    }
}

bool remove_job(SHARED_DATA *SHM, pid_t con_pid, int con_num){
    JOB temp;
    bool done = false;
    while (true){
        pthread_mutex_lock(&SHM->lock);
        
        if(SHM->job_completed == SHM->max_jobs){
            done = true;
            pthread_mutex_unlock(&SHM->lock);
            break;
        }
        else if(SHM->job_queue.size() > 0){
            temp = SHM->job_queue.top();
            SHM->job_queue.pop();
            SHM->job_completed += 1;
            if(SHM->job_completed == SHM->max_jobs) done = true;
            cout<<"Consumer: "<<con_num<<"\t";
            cout<<"Consumer pid: "<<con_pid<<"\t";
            printjob(temp);
            pthread_mutex_unlock(&SHM->lock);
            break;
        }
        pthread_mutex_unlock(&SHM->lock);
    }
    delay(temp.compute_time);
    return done;
}

void consumer_process(SHARED_DATA *SHM, int pid, int num){
    bool done = false;
    while(!done){
        delay(rand()%4);
        done = remove_job(SHM, pid, num);
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

    int shmid = create_shared_data();
    SHARED_DATA *SHM = get_shared_data(shmid, max_jobs);

    auto begin = std::chrono::high_resolution_clock::now();
    pid_t pid;

    for(int i=1; i<=NP; i++){
        pid = fork();
        if(pid < 0) error("Error in creating producer process\n");
        if(pid == 0){
            srand(time(0));
            producer_process(SHM, getpid(), i);
            return 0;
        }
    }

    for(int i=1; i<=NC; i++){
        pid = fork();
        if(pid < 0) error("Error in creating consumer process\n");
        if(pid == 0){
            srand(time(0));
            consumer_process(SHM, getpid(), i);
            return 0;
        }
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