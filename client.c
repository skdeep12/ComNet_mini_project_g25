#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <error.h>
#include <errno.h>
#include "serverDef.h"
#define rule_ 0
#define mat_ 1
#define num_ 2
#define winner_ 3
#define fnf_ 4
#define score_ 5
#define MAX_NUMBER 25
#define pause_ 9
int lifeline=0;
int sock;
int pause_flag=0;
int row_array[]={0,0,0,0,0};
int col_array[]={0,0,0,0,0};
int diag_array[]={0,0};
int bingo_count=0;    

void send_fnf(int sock,int b)
{
    char *it = (char *)malloc(sizeof(char)*10);
    char message[100];
    strcpy(message,"");
    if(b==pause_)
    {
        strcat(message,"Message-Type: 9\nPause");
        sprintf(it,"%lu",strlen(message));
        writen(sock,it,sizeof(it));
        writen(sock,message,strlen(message));
        return;
    }
    strcat(message,"Message-Type: 4\n");
    int i,j;
 
    char temp[10];
    sprintf(temp,"%d",b);
    strcat(message,temp);

    sprintf(it,"%lu",strlen(message));
    writen(sock,it,sizeof(it));
    writen(sock,message,strlen(message));
}

int checkDigit(char arr[])
{
	int i,n;
	n=strlen(arr);
	for(i=0;i<n;i++)
	{
		if(arr[i]-'0'<0 || arr[i]-'0'>9)
		{
			return 0;
		}
	}
	return 1;

}
int generateRandomNumber(int a,int b)   //This function generates a random number between a and b (both inclusive)
{
	int diff = b-a;
    return (int) (((double)(diff+1)/RAND_MAX) * rand() + a);        //Special function that uses floating-point math to reduce bias
}

int restoreElement(int n,int matrix[][n])
{
	int case1=0;
	int i,j;
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			if(matrix[i][j]==-1)
			{
				case1=1;
				break;
			}
		}
	}
	if(case1)
	{
		while(1)
		{
			int row = generateRandomNumber(0,n-1);
			int col = generateRandomNumber(0,n-1);
			if(matrix[row][col]==-1)
			{
				int restored = generateRandomNumber(1,MAX_NUMBER);
				matrix[row][col] = restored;
				printf("Position (%d %d) restored with %d\n",row+1,1+col,restored);
				break;
			}
		}
		return 0;
	}
	else
	{
		printf("No element can be restored\n");
		return 1;
	}
}

void deleteElement(int n,int matrix[][n])
{
	while(1)
	{
		int row = generateRandomNumber(0,n-1);
		int col = generateRandomNumber(0,n-1);
		if(matrix[row][col]!=-1 && matrix[row][col]!=0)
		{
			printf("Number %d at position (%d %d) deleted\n",matrix[row][col],row+1,1+col);
			matrix[row][col] = -1;
			break;
		}
	}
}

int search2(int i,int n,int matrix[][n])
{
    int a,b;
    //char yes_or_no[10];
    char row_no[10], column_no[10];
    //printf("Check if the number %d is present in your matrix:\n", i );
    //printf("If Yes, enter y, else enter n:\n");

    while(1)
    {
        for(a=0;a<n;a++)
        {
            for(b=0;b<n;b++)
            {
            	if(matrix[a][b]==-1)
            	{
            		printf("X   ");
            	}
            	else if(matrix[a][b]==0)
            	{
            		printf("-   ");
            	}
            	else
            	{
		            printf("%-3d ", matrix[a][b]);
            	}
            }
            printf("\n");
        }
        //printf("\nCheck if the number %d is present in your matrix:\n", i );
        //printf("If Yes, enter y, else enter n:\n");

        //scanf(" %s",yes_or_no);
        /*if(!strcmp(yes_or_no,"n")||!strcmp(yes_or_no,"N"))
        {
            return 0;
        }*/
        //else if(!strcmp(yes_or_no,"y")||!strcmp(yes_or_no,"Y"))
        //{
            while(1)
            {
                printf("To pause the game, enter 'p', else press any key!\n");
                char yes_or_no[10];
                scanf(" %s",yes_or_no);
                if(!strcmp(yes_or_no,"p") || !strcmp(yes_or_no,"P"))
                {
                    send_fnf(sock,9);
                    return -1;
                }
                else
                {
	                printf("Enter the position (row_no, column_no) where you see the number %d in your matrix\n", i);
	                scanf("%s %s", row_no,column_no);

	                int k=0,j=0;
			        if(!checkDigit(row_no) || !checkDigit(column_no) ||atoi(row_no)>n || atoi(column_no)>n)
	                {
	                    printf("Enter valid dimensions of the matrix. Row number and column number should be between 1-5\n");
	                }
	                else if( matrix[atoi(row_no)-1][atoi(column_no)-1]==i)
	                {
	                    matrix[atoi(row_no)-1][atoi(column_no)-1]=-1;
	                    return 0;
	                }
	                else
	                {
	                    printf("Number not found at that position. Start again.\n" );
	                    break;
	                }
	            }
            }
        
    }
    return 0;
}

int search(int round,int i, int n,int matrix[][n])
{
    //	system("clear");
    lifeline=(lifeline+1)%3;
    /*    if(chance_flag==1)
    {
        chance_flag=0;      //skip one chance
        return 0;
    }*/
    if(round==2 && lifeline==0)
    {
        char option2;
        int option;
        printf("It's lifeline time!!\n");
        printf("Each of the player gets to choose a random number between 1-6, with each number having some hidden Jackpots!\n");
        printf("Remember, the options will be shuffled after each round, so you won't have a lifeline at the same option again!\n");
	    while(1)
        {
	        printf("\nEnter a lifeline no[1-6]: \n" );
	        scanf(" %c",&option2);
            if((option2>='a' && option2<='z') || (option2>='A' && option2<='Z' ) || (option2-'0'<1) || (option2-'0'>6) )
            {
                printf("You have entered an invalid option. Please try again!\n");
            }
            else
            {
    	        option = generateRandomNumber(2,7);
    	        if(option==2)
    	        {
    	            printf("Sorry, you will have to miss a chance.\n");
    	            sleep(2);
    	            return 0;   //skip one chance
    	        }
    	        else if(option==3)
    	        {
    	        	//printf("One element restored\n");
    	        	restoreElement(n,matrix);
    	        	sleep(2);
    	        	return 0;
    	        }
    	        else if(option==4)
    	        {
    	        	//printf("One element deleted\n");
    	        	deleteElement(n,matrix);
    	        	sleep(2);
    	        	return 0;
    	        }
    	        else if(option==5)
    	        {
                    printf("Congratulations! Your score will be multiplied by a factor of 1.5.\n" );
    	        	sleep(2);
    	        	return 5;
    	        }
    	        else if(option==6)
    	        {
                    printf("Oops! Your score is scaled down by a factor of 1.5.\n");
    	        	printf("Enter a valid integer\n");
                    sleep(2);
                    return 6;
    	        }
                else if(option==7)
                {
                    printf("Congratulations, your score is increased by 2 points.\n");
                    sleep(2);
                    return 7;
                }
                break;
            }

        }
    }

    int a,b;
    char yes_or_no[10];
    char row_no[10], column_no[10];

    while(1)
    {
    	for(a=0;a<n;a++)
    	{
        	for(b=0;b<n;b++)
        	{
        		if(matrix[a][b]==-1)
        		{
        			printf("X   ");
        		}
            	else if(matrix[a][b]==0)
            	{
            		printf("-   ");
            	}
        		else
        		{
		            printf("%-3d ", matrix[a][b]);
		        }
	        }
        	printf("\n");
    	}
		printf("\nCheck if the number %d is present in your matrix:\n", i );
	    printf("If Yes, enter y, else enter n:....To pause, enter 'p'\n");

	    scanf(" %s",yes_or_no);
        if(!strcmp(yes_or_no,"p") || !strcmp(yes_or_no,"P"))
        {
            send_fnf(sock,9);
            return -1;
        }
	    else if(!strcmp(yes_or_no,"n")||!strcmp(yes_or_no,"N"))
	    {
	        return 0;
	    }
	    else if(!strcmp(yes_or_no,"y")||!strcmp(yes_or_no,"Y"))
	    {
	    	while(1)
	    	{
		        printf("Enter the position (row_no, column_no) where you see the number %d in your matrix\n", i);
		        scanf("%s %s", row_no,column_no );

		        int k=0,j=0;
		        if(!checkDigit(row_no) || !checkDigit(column_no) ||atoi(row_no)>n || atoi(column_no)>n)
		        {
		        	printf("Enter valid dimensions of the matrix.\n");
		        }
		        else if( matrix[atoi(row_no)-1][atoi(column_no)-1]==i)
		        {
		            matrix[atoi(row_no)-1][atoi(column_no)-1]=-1;
		            return 1;
		        }
		        else
		        {
		            printf("Number not found at that position. Start again.\n" );
		            break;
		        }
		    }
	    }
	    else
    	{
        	printf("Enter valid response only in terms of 'y' or 'n':\n");
    	}
    }
}

void string_to_matrix(char *string,int n,int matrix[][n])
{

    int i=0,j=0;
    char *token;
    token=strtok(string," ");
    while( token!=NULL)
    {
        while(j!=n)
        {
            matrix[i][j]=atoi(token);
            j++;
            token=strtok(NULL," ");
        }
        j=0;
        i++;
    }
}



void cut(int n,int number,int matrix[][n])
{
    int i,j,flag=0;
    for(i=0;i<n;i++){
        for(j=0;j<n;j++){
            if(matrix[i][j]==number){
                matrix[i][j]=-1;
                flag=1;
            }
        }
    }

    
    if(flag==0){
        printf("Number not present\n");
    }
}

void bingo(int n,int matrix[][n])
{
    int i,j;
    int row=0,col=0,diag1=0,diag2=0;
    for(i=0;i<n;i++)
    {
        for(j=0;j<n;j++)
        {
            if(row_array[i]==1)
            {
                break;
            }
            if(matrix[i][j]==-1 && row_array[i]==0)
            {
                row=1;
            }
            else if(matrix[i][j]!=-1)
            {
                row=0;
                break;
            }
        }
        if(row==1)
        {
            row_array[i] = 1;
            bingo_count++;
            break;
        }   
    }

    for(j=0;j<n;j++)
    {
        for(i=0;i<n;i++)
        {
            if(col_array[j]==1)break;
            if(matrix[i][j]==-1&&col_array[j]==0)
            {
                col=1;
            }
            else if(matrix[i][j]!=-1)
            {
                col=0;
                break;
            }
        }
        if(col==1)
        {
            col_array[j] = 1;
            bingo_count++;
            break;
        }
    }
    for(i=0;i<n;i++)
    {
        if(diag_array[0]==1)break;
        if(matrix[i][i]==-1&&diag_array[0]==0)
        {
            diag1=1;        
        }
        else if(matrix[i][i]!=-1)
        {
            diag1=0;
            break;
        }
    }
    if(diag1==1)
    {
        diag_array[0]=1;
        bingo_count++;
    }
    for(i=n-1;i>=0;i--)
    {
        if(diag_array[1]==1)break;
        if(matrix[i][n-1-i]==-1&&diag_array[1]==0)
        {
            diag2=1;        
        }
        else if(matrix[i][n-1-i]!=-1)
        {
            diag2=0;
            break;
        }
    }
    if(diag2==1)
    {
        diag_array[1]=1;
        bingo_count++;
    }
}

int main(int argc, char const *argv[])
{
    if (argv[1] == NULL)
    {
        printf("Provide the ip of the server\n");
        return 0;
    }
    sock = socket(AF_INET,SOCK_STREAM,0);
    char it[23],msg[100];
    char tmpmsg[100];
    //char *token;
    // char sp[10] = "\n";	
    // it = (char*)malloc(sizeof(char)*23);
    // msg = (char*)malloc(sizeof(char)*100);
    // tmpmsg = (char*)malloc(sizeof(char)*100);

    struct sockaddr_in sa;
    sa.sin_port = htons(9095);
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(argv[1]);
    
    if (connect(sock,(struct sockaddr * )&sa,sizeof(sa)) == -1)
    {
        perror("error in connecting\n");
        return -1;
    }
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////



   char writemsg[100],content[100];
    long unsigned int lom;
    int matrix[4][4];
    int noOfBytes,p,code_no,t1=0,flag=0,score[10],winner=-1;
    char num_arr[3];
    while(1)
    {
        p = readn(sock,it,8);//reading the length of the message from socket
        if (p == 0)
        {
            printf("Server Closed the Connection\n");
            return 0;
        }
        noOfBytes = atoi(it);
        p = readn(sock,msg,noOfBytes);//reading the message
        //printf("Server1: %s\n", msg );
        msg[p] = '\0';
        int code = decodeMessage(msg,content,p);
        if (code == rule_)
        {
            printf("%s\n",content);
	        memset(msg,0,noOfBytes);
	        memset(content,0,noOfBytes);
	        strcpy(msg,"\0");
	        strcpy(content,"\0");
        }
        else if (code == mat_)
        {
//        	printf("msg = %s, content = %s\n",msg,content);
            string_to_matrix(content,4,matrix);            

        //level_1:
	        while(winner == -1)
	        {
                if(pause_flag==1)
                {
                    pause_flag=0;
                    char resume[10];
                    printf("Enter 'r' to resume:\n");
                    scanf(" %s",resume);
                    if(!strcmp(resume,"r") || !strcmp(resume,"R"))
                    {
                        send_fnf(sock,pause_);
                    }
                }
			    memset(msg,0,sizeof(msg));
	            memset(content,0,sizeof(content)); 
	            strcpy(msg,"\0");
	            strcpy(content,"\0");   	
		       	p = readn(sock,it,8);//reading the length of the message from socket
		//        	printf("Message : %d\n",p );
		        if (p == 0)
		        {
                    printf("hello\n");
		            printf("Server Closed the Connection\n");
		            return 0;
		        }
		        noOfBytes = atoi(it);
		        p = readn(sock,msg,noOfBytes);//reading the message
//		        printf("Server: %s\n", msg);
		        code = decodeMessage(msg,content,p);
		        //printf("code= %d\n",code );
	        	if (code == num_)
		        {
    //		            printf("Msg : %s, content: %s\n",msg,content);
		            int res = search(1,atoi(content),4,matrix);
                    if(res==-1)
                    {
                        printf("Client has paused the game\n");
                        pause_flag=1;
                    }
                    else
                    {
    		            send_fnf(sock,res);
                    }
		        }
		        else if(code == score_)
		        {
//		        	printf("content\n%s\n",content );
		        	int z=0;
		        	char *token;
		        	char sp[2] = " ";
		        	token = strtok(content,sp);
		        	while(token!=NULL)
		        	{
		        		score[z++] = atoi(token);
		        		token = strtok(NULL,sp);
		        	}
		        	system("clear");
                    printf("\n******************    WHITE BOARD   ************************\n\n");
		        	for (z = 0; z < 10; z++)
		        	{
		        		printf("Player[%d] = %d : ",z,score[z]);
		        		if(z==4)
		        		{
		        			printf("\n");
		        		}
		        	}
		        	printf("\n\n\n" );
		        }
		        else if(code == winner_)
		        {
		        	winner = atoi(content);
//	                printf("Game's Winner is Player %d\n", winner );
		        	printf("Level 1 Finished. Moving on to Level 2!\n");
  
	                //goto level2;
		        }
	        }
	        break;
	    }

    }
        ///////////////////     LEVEL 2     ////////////////////////////////////////////
    printf("\n\n---------------------STARTING LEVEL 2------------------------\n");
    //char writemsg[100],content[100];
    //long unsigned int lom;
    int matrix2[5][5];
    lifeline=0;
    t1=0;flag=0;winner=-1;
    pause_flag=0;
    //char num_arr[3];
    while(1)
    {
        p = readn(sock,it,8);//reading the length of the message from socket
        if (p == 0)
        {
            printf("Server Closed the Connection\n");
            return 0;
        }
        noOfBytes = atoi(it);
        p = readn(sock,msg,noOfBytes);//reading the message
        //printf("Server1: %s\n", msg );
        msg[p] = '\0';
        int code = decodeMessage(msg,content,p);
        if (code == rule_)
        {
            printf("%s\n",content);
            memset(msg,0,noOfBytes);
            memset(content,0,noOfBytes);
            strcpy(msg,"\0");
            strcpy(content,"\0");
    //         goto last;
        }
        else if (code == mat_)
        {
//            printf("msg = %s, content = %s\n",msg,content);
            string_to_matrix(content,5,matrix2);            

        //level_1:
            while(winner == -1)
            {
                if(pause_flag==1)
                {
                    pause_flag=0;
                    char resume[10];
                    printf("Enter 'r' to resume:\n");
                    scanf(" %s",resume);
                    if(!strcmp(resume,"r") || !strcmp(resume,"R"))
                    {
                        send_fnf(sock,pause_);
                    }
                }

                memset(msg,0,sizeof(msg));
                memset(content,0,sizeof(content)); 
                strcpy(msg,"\0");
                strcpy(content,"\0");       
                p = readn(sock,it,8);//reading the length of the message from socket
        //          printf("Message : %d\n",p );
                if (p == 0)
                {
                    printf("    \n");
                    printf("Server Closed the Connection\n");
                    return 0;
                }
                noOfBytes = atoi(it);
                p = readn(sock,msg,noOfBytes);//reading the message
                //printf("Server: %s\n", msg);
                code = decodeMessage(msg,content,p);
                //printf("code= %d\n",code );
                if (code == num_)
                {
                    //printf("Msg : %s, content: %s\n",msg,content);
		    		int res = search(2,atoi(content),5,matrix);
                    if(res==-1)
                    {
                        printf("Client has paused the game\n");
                        pause_flag=1;
                    }
                    else
                    {
    		            send_fnf(sock,res);
                    }                
				}
                else if(code == score_)
                {
                    printf("content\n%s\n",content );
                    int z=0;
                    char *token;
                    char sp[2] = " ";
                    token = strtok(content,sp);
                    while(token!=NULL)
                    {
                        score[z++] = atoi(token);
                        token = strtok(NULL,sp);
                    }
		        	system("clear");
                    printf("\n******************    WHITE BOARD   ************************\n\n");
		        	for (z = 0; z < 10; z++)
		        	{
		        		printf("Player[%d] = %d : ",z,score[z]);
		        		if(z==4)
		        		{
		        			printf("\n");
		        		}
		        	}
		        	printf("\n\n\n" );
                }
                else if(code == winner_)
                {
                    winner = atoi(content);
//                  printf("Game's Winner is Player %d\n", winner );
                    printf("Level 2 finished. Moving on to Level 3!\n");

                    //goto level2;
                }
            }
            break;
        }



    }

    /////////////////////////////////////////LEVEL 3/////////////////////////////////////////////

    printf("\n\n---------------------STARTING LEVEL 3------------------------\n");
    //char writemsg[100],content[100];
    //long unsigned int lom;
    //int matrix2[5][5];
    lifeline=0;
    t1=0;flag=0;winner=-1;
    pause_flag=0;
    //char num_arr[3];
    while(1)
    {
        p = readn(sock,it,8);//reading the length of the message from socket
        if (p == 0)
        {
            printf("Server Closed the Connection\n");
            return 0;
        }
        noOfBytes = atoi(it);
        p = readn(sock,msg,noOfBytes);//reading the message
        //printf("Server1: %s\n", msg );
        msg[p] = '\0';
        int code = decodeMessage(msg,content,p);
        if (code == rule_)
        {
            printf("%s\n",content);
            memset(msg,0,noOfBytes);
            memset(content,0,noOfBytes);
            strcpy(msg,"\0");
            strcpy(content,"\0");
    //         goto last;
        }
        else if (code == mat_)
        {
//            printf("msg = %s, content = %s\n",msg,content);
            string_to_matrix(content,5,matrix2);            

        //level_3:
            while(winner == -1)
            {
                if(pause_flag==1)
                {
                    pause_flag=0;
                    char resume[10];
                    printf("Enter 'r' to resume:\n");
                    scanf(" %s",resume);
                    if(!strcmp(resume,"r") || !strcmp(resume,"R"))
                    {
                        send_fnf(sock,pause_);
                    }
                }
                memset(msg,0,sizeof(msg));
                memset(content,0,sizeof(content)); 
                strcpy(msg,"\0");
                strcpy(content,"\0");       
                p = readn(sock,it,8);//reading the length of the message from socket
        //          printf("Message : %d\n",p );
                if (p == 0)
                {
                    printf("    \n");
                    printf("Server Closed the Connection\n");
                    return 0;
                }
                noOfBytes = atoi(it);
                p = readn(sock,msg,noOfBytes);//reading the message
                //printf("Server: %s\n", msg);
                code = decodeMessage(msg,content,p);
                //printf("code= %d\n",code );
                if (code == num_)
                {

                    //printf("Msg : %s, content: %s\n",msg,content);
                    int result=search2(atoi(content),5,matrix2);
                    if(result==-1)
                    {
                        printf("Client has paused the game\n");
                        pause_flag=1;
                    }
                    else
                    {
	                    bingo(5,matrix2);
	                    int res;
	                    printf("BINGO COUNT = %d\n",bingo_count);
	                    if(bingo_count!=5)
	                    {
	                        res=1;
	                    }
	                    else
	                    {
	                        res = 5;
	                    }
	                    send_fnf(sock,res);                    	
                    }
                }
                else if(code == score_)
                {
                    printf("content\n%s\n",content );
                    int z=0;
                    char *token;
                    char sp[2] = " ";
                    token = strtok(content,sp);
                    while(token!=NULL)
                    {
                        score[z++] = atoi(token);
                        token = strtok(NULL,sp);
                    }
		        	system("clear");
                    printf("\n******************    WHITE BOARD  ************************\n\n");
		        	for (z = 0; z < 10; z++)
		        	{
		        		printf("Player[%d] = %d : ",z,score[z]);
		        		if(z==4)
		        		{
		        			printf("\n");
		        		}
		        	}
		        	printf("\n\n\n" );
                }
                else if(code == winner_)
                {
                	int max=0,i;
                	for(i=0;i<10;i++)
                	{
                		if(max<score[i])
                		{
                			max=score[i];
                		}

                	}
                	printf("The game ends here!\n\n");
                	printf("\nWinners of the game: ");
                    winner = atoi(content);
                    for(i=0;i<10;i++)
                    {
                    	if(score[i]==max)
                    	{
                    		printf(" %d ",i );
                    	}
                    }

                    printf("\n");
//                    printf("Game's Winner is Player %d\n", winner );
//                    printf("Inside winner:\n");

                    //goto level2;
                }
            }
            break;
        }
    }
}
