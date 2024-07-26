#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>

#define MAX_RUNWAYS 10
#define BACKUP_LOAD_CAPACITY 15000

struct Message
{
    long mtype;
    char keyword[100];
    int plane_id;
    int airport_departure;
    int airport_arrival;
    int total_weight;
    int plane_type;
    int num_passengers;
}; //for message queue

struct Runway {
    int number;
    int load_capacity;
    int is_available;
    pthread_mutex_t lock;
};

void* handle_plane(void* arg); //for handling departures and arrivals

struct Runway runways[MAX_RUNWAYS + 1];
int num_runways;

int find_runway(int plane_weight) {
    int min_diff = INT_MAX;
    int runway_number = -1;
    bool use_backup=true;
    
    for (int i=0; i<num_runways; i++)
    {
        if (runways[i].load_capacity>=plane_weight)
        {
            use_backup=false;
            break;
        }
    }
    
    //if use_backup remains true, no runway is capable to handle the given load, except the backup runway
    
    if (use_backup)
        return num_runways;

    for (int i = 0; i < num_runways; i++) {
        if (runways[i].is_available) {
            int diff = runways[i].load_capacity - plane_weight;
            if (diff >= 0 && diff < min_diff) {
                min_diff = diff;
                runway_number = i;
            }
        }
    }

    if (runway_number == -1) //no available runways have enough load capacity
    {
        while (true)
        {
            for (int i = 0; i < num_runways; i++)
            {
                if (runways[i].is_available)
                {
                    int diff = runways[i].load_capacity - plane_weight;
                    if (diff >= 0 && diff < min_diff)
                    {
                        min_diff = diff;
                        runway_number = i;
                    }
                }
            }
            
            if (runway_number!=-1) // a suitable runway just got free
                break;
        }
    }

    return runway_number;
}

int main() {
    
    int airport_number;
    printf("Enter Airport Number: ");
    scanf("%d", &airport_number);

    
    printf("Enter number of Runways: ");
    scanf("%d", &num_runways);
    
    printf ("Enter loadCapacity of Runways (give as a space separated list in a single line):");
    
    for (int i=0; i< num_runways; i++)
        scanf("%d", &runways[i].load_capacity);

    for (int i = 0; i < num_runways; i++) //initializing all airports
    {
        runways[i].number = i + 1;
        runways[i].is_available = 1;
        pthread_mutex_init(&runways[i].lock, NULL);
    }

    
    runways[num_runways].number = num_runways + 1; //initializing backup airport
    runways[num_runways].load_capacity = BACKUP_LOAD_CAPACITY;
    runways[num_runways].is_available = 1;
    pthread_mutex_init(&runways[num_runways].lock, NULL);
    
    int msgid;

    msgid = msgget( (key_t) 12345, 0644 | IPC_CREAT);
    if (msgid == -1)
    {
        perror("Error creating message queue");
        exit(EXIT_FAILURE);
    }

    bool isCleanup=false; //to see if a cleanup message has been received yet
    
    while (true)
    {
        
        if (isCleanup) //check if all runways are free
        {
            bool f= true;
            for (int i=0; i<=num_runways; i++)
            {
                if (runways[i].is_available==0)
                {
                    f=false;
                    break;
                }
            }
            
            if (f) // all runways are free
                break;
        }
        
        struct Message message;
        if (msgrcv(msgid, &message, sizeof(struct Message), 1, IPC_NOWAIT)==-1) //no message yet
            continue;
            
        else if ((strcmp (message.keyword, "departure")==0 && message.airport_departure==airport_number) || (message.airport_arrival==airport_number && strcmp (message.keyword, "arrival")==0))
        {
            
            msgrcv(msgid, &message, sizeof(struct Message), 1, 0);
            
            pthread_t plane_thread;
            if (pthread_create(&plane_thread, NULL, handle_plane, (void*)&message) != 0) //create the departure or arrival thread
            {
                perror("Error creating thread");
                exit(EXIT_FAILURE);
            }
        }
        
        else if (strcmp (message.keyword, "cleanAirports")) //received from airtrafficcontroller.c
        {
            msgrcv(msgid, &message, sizeof(struct Message), 1, 0);
            isCleanup=true;
        }
    }

    for (int i = 0; i < num_runways + 1; i++)
    {
        pthread_mutex_destroy(&runways[i].lock);
    }

    return 0;
}

void* handle_plane(void* arg) {
    struct Message* message = (struct Message*)arg;
    
    struct Message* message_send; //send a message to airtrafficcontroller.c if a plane has departed or unloaded
    
    int msgid;

    msgid = msgget( (key_t) 12345, 0644 | IPC_CREAT);
    if (msgid == -1)
    {
        perror("Error creating message queue");
        exit(EXIT_FAILURE);
    }
    
    char buf[100]= "takeoff"; //plane has departed
    
    snprintf (message_send->keyword, sizeof (message_send->keyword), buf);
    message_send->plane_id=message->plane_id;
    message_send->airport_departure=message->airport_departure;
    message_send->airport_arrival=message->airport_arrival;
    message_send->total_weight=message->total_weight;
    message_send->plane_type=message->plane_type;
    message_send->num_passengers=message->num_passengers;

    if (strcmp(message->keyword, "departure")==0)
    {
        int runway_number = find_runway(message->total_weight);

        runways[runway_number].is_available=0;
        pthread_mutex_lock(&runways[runway_number].lock);
        sleep (3); //3 seconds for loading
        pthread_mutex_unlock(&runways[runway_number].lock);
        runways[runway_number].is_available=1;
        
        if(msgsnd(msgid, (void *)&message_send, 100, 0) == -1)
        {
            fprintf(stderr, "Error in sending message %s",buf);
            exit(1);
        }
    }
    
    else
    {
        int runway_number = find_runway(message->total_weight);

        runways[runway_number].is_available=0;
        pthread_mutex_lock(&runways[runway_number].lock);
        sleep (3); //3 seconds for unloading
        pthread_mutex_unlock(&runways[runway_number].lock);
        runways[runway_number].is_available=1;
        
        char buf1[100]= "unloaded"; //send message to airtrafficcontroller.c that the plane has been unloaded
        snprintf (message_send->keyword, sizeof (message_send->keyword), buf1);
        
        if(msgsnd(msgid, (void *)&message_send, 100, 0) == -1)
        {
            fprintf(stderr, "Error in sending message %s",buf);
            exit(1);
        }
    }

    pthread_exit(NULL);
}