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
    while (true)
    {
        printf("Do you want the Air Traffic Control System to terminate?(Y for Yes and N for No)");
        char ch;
        
        do
        {
            ch = getchar();
        }while (ch == '\n');
        
        if (ch=='Y' || ch=='y')
        {
            
            struct Message message;
            
            message.plane_id=0;
            message.msg_type=1;
            message.airport_arrival=0;
            message.airport_departure=0;
            message.total_weight=0;
            message.plane_type=0;
            message.num_passengers=0;
            
            int msgid;
            char buf[100]= "Cleanup";
            
            msgid = msgget((key_t) 12345, 0644);    
            if (msgid == -1)
            {
                fprintf(stderr, "Error in creating message queue\n");
                exit(1);
            }
                    
            strcpy(message.keyword, buf);
            
            if(msgsnd(msgid, (void *)&message, 100, 0) == -1)
            {
                fprintf(stderr, "Error in sending message %s",buf);
                exit(1);
            }
            
            break;
        }
    }
    

    return 0;
}
