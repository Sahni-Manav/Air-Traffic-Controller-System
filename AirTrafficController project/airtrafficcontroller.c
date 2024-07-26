/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>

struct Message
{
    long msg_type;
    char keyword[100];
    int plane_id;
    int airport_departure;
    int airport_arrival;
    int total_weight;
    int plane_type;
    int num_passengers;
};

int main()
{
    printf ("Enter the number of airports to be handled/managed: "); //between 2 to 10
    int n;
    scanf ("%d", &n);
    
    struct Message message;
    int msgid;
    char buf[100];
    
    msgid = msgget((key_t) 12345, 0644|IPC_CREAT);    
    if (msgid == -1)
    {
        fprintf(stderr, "Error in creating message queue\n");
        exit(1);
    }
    
    FILE *f;
    f= fopen ("AirTrafficController.txt", "a");
    
    //whenever a change is made in the sheet
    while (true)
    {
        if (msgrcv(msgid, (void *)&message, 100, 1, IPC_NOWAIT) == -1) //no message
            continue;
        
        if (strcmp(message.keyword, "Cleanup")==0)
        {
            msgrcv(msgid, (void *)&message, 100, 1, 0);
            
            struct Message message1;
            char buf1[100]= "cleanAirports";
            
            strcpy (message1.keyword, buf1);
            message1.msg_type=1;
            message1.plane_type=0;
            message1.plane_id=0;
            message1.airport_arrival=0;
            message1.airport_departure=0;
            message1.total_weight=0;
            message1.num_passengers= 0;
            
            if(msgsnd(msgid, (void *)&message1, 100, 0) == -1)
            {
                fprintf(stderr, "Error in sending message %s",buf);
                exit(1);
            }
            break;
        }
        
        else if (strcmp(message.keyword, "takeoff"))
        {
            msgrcv(msgid, (void *)&message, 100, 1, 0);
            fprintf (f, "Plane %d has departed from Airport %d and will land at Airport %d.", message.plane_id, message.airport_departure, message.airport_arrival);
            
            sleep (30);
            
            struct Message message1;
            char buf1[100]= "arrival";
            
            strcpy (message1.keyword, buf1);
            message1.msg_type=1;
            message1.plane_type=message.plane_type;
            message1.plane_id=message.plane_id;
            message1.airport_arrival=message.airport_arrival;
            message1.airport_departure=message.airport_departure;
            message1.total_weight=message.total_weight;
            message1.num_passengers= message.num_passengers;
            
            if(msgsnd(msgid, (void *)&message1, 100, 0) == -1)
            {
                fprintf(stderr, "Error in sending message %s",buf);
                exit(1);
            }
            
        }
        
        else if (strstr(message.keyword, "unloaded")!=NULL)
        {
            msgrcv(msgid, (void *)&message, 100, 1, 0);
            
            struct Message message1;
            char buf1[100]= "landed";
            
            strcpy (message1.keyword, buf1);
            message1.msg_type=1;
            message1.plane_type=message.plane_type;
            message1.plane_id=message.plane_id;
            message1.airport_arrival=message.airport_arrival;
            message1.airport_departure=message.airport_departure;
            message1.total_weight=message.total_weight;
            message1.num_passengers= message.num_passengers;
            
            if(msgsnd(msgid, (void *)&message1, 100, 0) == -1)
            {
                fprintf(stderr, "Error in sending message %s",buf);
                exit(1);
            }
        }
        else if (strcmp (message.keyword, "planeData"))
        {
            
            msgrcv(msgid, (void *)&message, 100, 1, 0);
            
            struct Message message1;
            
            char buf1[100]= "departure";
            
            strcpy (message1.keyword, buf1);
            message1.msg_type=1;
            message1.plane_type=message.plane_type;
            message1.plane_id=message.plane_id;
            message1.airport_arrival=message.airport_arrival;
            message1.airport_departure=message.airport_departure;
            message1.total_weight=message.total_weight;
            message1.num_passengers= message.num_passengers;
            
            if(msgsnd(msgid, (void *)&message1, 100, 0) == -1)
            {
                fprintf(stderr, "Error in sending message %s",buf);
                exit(1);
            }
        }
    }
    
    return 0;
    
}
