#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>

#define PASSENGER 1
#define CARGO 0
#define READ 0
#define WRITE 1
#define PASSENGER_EXTRA_WEIGHT (75*7)
#define CARGO_EXTRA_WEIGHT (75*2)

struct Message
{
    long msg_type;
    char keyword[100];
    int planeID;
    int departure;
    int arrival;
    int planeWeight;
    int planeType;
    int numPassengers;
}; //for message queue

void passengerProcess(int pipe_read, int pipe_write)
{
    close(pipe_read);
    
    printf ("Enter Weight of Your Luggage: "); //between 1 to 25
    int weight;
    scanf ("%d", &weight);
    
    printf ("Enter Your Body Weight: "); //between 10 to 100
    int Bweight;
    scanf ("%d", &Bweight);
    
    char message1[10];
    snprintf(message1, sizeof(message1), "%d", weight);
    
    char message2[10];
    snprintf(message2, sizeof(message2), "%d", Bweight);
    
    strcat (message1, message2);
    
    write(pipe_write, message1, sizeof(message1));
    
    close(pipe_write);
}

int main ()
{
    
    int totalWeight=0;
    struct Message message;
    message.msg_type=1;
    
    printf ("Enter plane ID: "); //between 1 and 10
    int id;
    scanf ("%d", &id);
    
    if (id<=0)
    {
        fprintf (stderr, "ID has to be positive.");
        return 1;
    }
    
    message.planeID= id;
    
    printf ("Enter Type of Plane: ");
    int type;
    scanf ("%d", &type); //passenger 1 cargo 0
    
    message.planeType= type;
    
    if (type== PASSENGER)
    {
        printf ("Enter Number of Occupied Seats: "); //between 1 to 10
        int n;
        scanf ("%d", &n);
        
        message.numPassengers= n;
        
        int fd[n][2];
        
        for (int i=0; i<n; i++)
        {
            if (pipe (fd[i])==-1)
            {
                fprintf (stderr, "Pipe failed.");
                return 1;
            } //all pipes created
        
            pid_t pid = fork();
            
            if (pid==-1)
            {
                fprintf (stderr, "Fork failed.");
                return 1;
            }
            
            else if (pid==0) //child process
            {
                passengerProcess(fd[i][READ], fd[i][WRITE]);
            }
            wait (NULL);
        }
        
        //parent process
        int weights[n][2];
        
        for (int i=0; i<n; i++)
        {
            close (fd[i][WRITE]);
            
            char buffer[10];
            int nbytes = read(fd[i][READ], buffer, sizeof(buffer));
            
            if (nbytes >= 0)
            {
                buffer[nbytes] = '\0';
                // printf("Parent received message from child %d: %s\n", i+1, buffer);
                
                char *w;
                w= strtok (buffer, " ");
                weights[i][0]= atoi(w); //luggage weight
                
                w= strtok(NULL, " ");
                weights[i][1]= atoi(w); //body weight
            }
            
            close(fd[i][READ]);
        }
        
        totalWeight= PASSENGER_EXTRA_WEIGHT;
        for (int i=0; i<n; i++)
            totalWeight+= weights[i][0]+ weights[i][1];
        
    }
    
    else if (type== CARGO)
    {
        message.numPassengers=0;
        
        printf ("Enter Number of Cargo Items: "); //between 1 and 100
        int n;
        scanf ("%d", &n);
        
        printf ("Enter Average Weight of Cargo Items: "); //between 1 to 100
        int avg;
        scanf ("%d", &avg);
        
        totalWeight= CARGO_EXTRA_WEIGHT;
        totalWeight+= avg*n;
        
    }
    
    else
    {
        fprintf (stderr, "Incorrect value entered.");
        return 1;
    }
    
    printf ("Enter Airport Number for Departure: "); // between 1 and 10
    int departureAirport;
    scanf ("%d", &departureAirport);
    
    printf ("Enter Airport Number for Arrival: "); // between 1 and 10
    int arrivalAirport;
    scanf ("%d", &arrivalAirport);
    
    message.arrival = arrivalAirport;
    message.departure= departureAirport;
    message.planeWeight= totalWeight;
    
    int msgid;
    char buf[100];
    
    msgid = msgget((key_t) 12345, 0644);    
    if (msgid == -1)
    {
        fprintf(stderr, "Error in creating message queue\n");
        exit(1);
    }
    
    message.msg_type = 1;
    snprintf(buf, sizeof(buf), "planeData");
    strcpy(message.keyword, buf);
    
    if(msgsnd(msgid, (void *)&message, 100, 0) == -1)
    {
        fprintf(stderr, "Error in sending message %s",buf);
        exit(1);
    }
    
    struct Message m1;
    
    while (true)
    {
        if (msgrcv(msgid, (void *)&m1, 100, 1, IPC_NOWAIT) == -1) //no message
            continue;
            
        else if (strcmp (m1.keyword, "landed")==0 && m1.planeID==id);
        {
            msgrcv(msgid, (void *)&m1, 100, 1, 0);
            break;
        }
    }
    
    printf ("Plane %d has successfully traveled from Airport %d to Airport %d!", id, departureAirport, arrivalAirport);
    
    return 0;
}