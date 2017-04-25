/*
 * server.c
 *
 *  Created on: 24-Apr-2017
 *      Author: drogon
 */
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
#include <signal.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "serverDef.h"



#define MAX_PLAYERS 8
#define MIN_REQ 1
#define listenPort 9095
#define rule_ 0
#define mat_ 1
#define num_ 2
#define winner_ 3
#define fnf_ 4
#define score_ 5
#define pause_ 9
#define MAX_NUMBER 25
#define ALARM_TIME 15
int YO_flag=0;
//#define struct sockaddr_in * sa_inP;
//#define printIP() printf("Client ip is %s\n", inet_ntoa(p->sin_addr));
int winner = -1;

void handler(int sig)
{

	printf("in sig handler\n");
}

void encryptMatrixToMessage(char message[],int n,int matrix[][n][n],int playerNo)
{
	strcpy(message,"");
	//strcat(message,"Message-Type: 1\n");
	int i,j;
	char *it = (char *)malloc(sizeof(char)*10);
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			sprintf(it,"%d",matrix[playerNo][i][j]);
			strcat(message,it);
			if(i!=n-1 || j!=n-1)
			{
				strcat(message," ");
			}
		}
	}
	//strcat(message,"\0");
}

int checkCount(int input_from_clients[],int playerNo)
{
	int i;
	for(i=0;i<playerNo;i++)
	{
		if(input_from_clients[i]==0)
		{
			return 0;
		}
	}
	return 1;
}

int printMatrices(int matrix[][4][4], int no_of_players)
{
	int i,j,k;
	for(i=0;i<no_of_players;i++)
	{
		printf("Player %d Matrix\n******************\n\n",i);
		for(j=0;j<4;j++)
		{
			for(k=0;k<4;k++)
				printf("%d\t",matrix[i][j][k]);
			printf("\n");
		}
		printf("\n\n\n");
	}
	return 0;
}

int generateRandomNumber(int a,int b)   //This function generates a random number between a and b (both inclusive)
{
	int diff = b-a;
    return (int) (((double)(diff+1)/RAND_MAX) * rand() + a);        //Special function that uses floating-point math to reduce bias
}

void sendRules(int sock)
{

	char *it = (char *)malloc(sizeof(char)*10);
	char rules[] = "Message-Type: 0\nWelcome to the Game Tambola!!";
	//char readMsg[100];
	sprintf(it,"%lu",sizeof(rules));
	printf("size of message is %s and length of it is %lu\n",it,strlen(it));
	writen(sock,it,sizeof(it));
	writen(sock,rules,sizeof(rules));
}

void sendMessage(int sock,char rules[],int type)
{
	char *it = (char *)malloc(sizeof(char)*10);
	char readmsg[100];
//	strcpy(readmsg,)
	sprintf(readmsg,"Message-Type: %d\n",type);
	strcat(readmsg,rules);
	sprintf(it,"%lu",strlen(readmsg));
	writen(sock,it,sizeof(it));
	writen(sock,readmsg,strlen(readmsg));
}

int check_winner(int score[],int playerNo)
{
	int i;
	for(i=0;i<playerNo;i++)
	{
		if(score[i]==10)return i;	//this function checks the score of players 0 to playerNo and if it
		//gets a player with score 10, it returns its index. However, it is possible that after one iteration
		//two players win together. So, both the players need to be returned.

	}
	return -1;
}
int ifexhausted(int arr[],int n)
{
	int i;
	for(i=1;i<n;i++)
	{
		if(arr[i]==-1)
		{
			return 0;
		}
	}
	return 1;
}

int find_max(int scores[],int playerNo)
{
	int i;
	int max=-1;
	int max_index=0;
	for(i=0;i<playerNo;i++)
	{
		if(scores[i]>max)
		{
			max=scores[i];
			max_index=i;
		}
	}
	return max_index;

}
int max_fd(int cl[],int dis[],int pl)
{
	int max = -1,i;
	for(i=0;i<pl;i++)
	{
		if(dis[i] == 0)
		{
			if(cl[i] > max)
				max = cl[i];
		}
	}
	return max;
}
int main()
{
	srand(time(NULL));
	struct sigaction act;
	memset(&act, '\0', sizeof(act));

	/* Use the sa_sigaction field because the handles has two additional parameters */
	act.sa_handler = handler;
 	sigemptyset (&act.sa_mask);
	/* The SA_SIGINFO flag tells sigaction() to use the 	sa_sigaction field, not sa_handler. */
	act.sa_flags = 0;
	sigaction (SIGALRM, &act,NULL);
	int listenSock;
	int clientSockets[10];
	struct sockaddr_in sa;
	struct sockaddr_in cl_sockInfo[10];
	listenSock = socket(AF_INET,SOCK_STREAM,0);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(listenPort);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
   		perror("setsockopt(SO_REUSEADDR) failed");
	int flags = fcntl(listenSock,F_GETFL,0);
	flags = flags | O_NONBLOCK;
	fcntl(listenSock,F_SETFL,flags);
	if (bind(listenSock,(struct sockaddr *) &sa,sizeof(sa)) == -1)
	{
		printf("bind error for listening socket\n");
		return -1;
	}
	if (listen(listenSock,10) == -1)
	{
		printf("error in listening state\n");
		return -1;
	}
	printf("Server is Up and listening on port number %d\n",listenPort );


/*//////////////////////  We need to call all the players and connect them here  ////////////////////////*/

	fd_set fs;
	int playerNo=0,len_cl = sizeof(struct sockaddr_in);
	int s_err;
	FD_SET(listenSock,&fs);
	struct timeval time_out;
	time_out.tv_sec = ALARM_TIME;
	time_out.tv_usec = 0;
	int time_flag=0;
	while(time_flag==0)
	{
		printf("playerNo = %d,MIN_REQ = %d",playerNo,MIN_REQ);
		s_err = select(listenSock+1,&fs,NULL,NULL,&time_out);
		if(s_err == 0)
		{
			//timer expires
			if(playerNo>=MIN_REQ)
			{
				time_flag=1;
			}
			printf("timer expired\n");
			FD_SET(listenSock,&fs);
			time_out.tv_sec = ALARM_TIME;
			time_out.tv_usec = 0;
		}
		else if(s_err == 1)
		{
			printf("Clients are available\n");
			while((clientSockets[playerNo] = accept(listenSock,(struct sockaddr *)&cl_sockInfo[playerNo],&len_cl)) > 0)
			{
				printf("returned %d\n",clientSockets[playerNo]);
				sendRules(clientSockets[playerNo]);
				playerNo++;
			}
			printf("remaining time = %ld.%ld",time_out.tv_sec,time_out.tv_usec);
		}
		else if(s_err < 0)
		{
			printf("error in select() call\n");
		}
	}

	int pause_cl;
	printf("Number of players is %d. Starting the game\n",playerNo);
	printf("client ip is %s\n",inet_ntoa(cl_sockInfo[0].sin_addr));
	int exhaust_flag=0;
	int no_of_players=10;              ////////////  Find out the number_of_players   /////////////////////////
	int matrix[no_of_players][4][4];   ////////////  Create player matrix             /////////////////////////
	memset(matrix,0,sizeof(matrix));   ////////////  Initialize player matrix with 0  /////////////////////////
	int i,j,ag;                           ////////////  Initialize loop variables        /////////////////////////
	int disconnected[10];
	int num_of_disClients=0;
	memset(disconnected,0,10*sizeof(int));
	//////////////////////  Create matrices for all players and deploy them to each player  ///////////////////
	for(i=0;i<no_of_players;i++)
	{
		int temporary_matrix[MAX_NUMBER+1]={0};
		int random_number,row_number,column_number;
		for(j=0;j<10;j++)
		{
			while(1)        // This loop generates unique random numbers
			{
				random_number = generateRandomNumber (1,MAX_NUMBER-1);
				if(temporary_matrix [random_number] == 0)       // If the number has not been generated.....break
				{
					temporary_matrix[random_number]=1;
					break;
				}
			}
			while(1)       // This loop generates unique position for the generated random number
			{

				row_number = generateRandomNumber (0,3);
				column_number = generateRandomNumber (0,3);
				if(matrix[i][row_number][column_number] == 0)
				{
					matrix[i][row_number][column_number] = random_number;
					break;
				}
			}

		}
	}

	char message[100];
	char randomNoMessage[100];
	for(i=0;i<playerNo;i++)
	{
		encryptMatrixToMessage(message,4,matrix,i);
		printf("%s\n",message );
		sendMessage(clientSockets[i],message,mat_);
	}
	printf("Matrices sent to clients\n");

	//int input_from_clients[playerNo];
	int count,p,noOfBytes;
	fd_set readfds,tempfds;
	char *readmsg = (char *)malloc(sizeof(char)*100);
	char *content = (char *)malloc(sizeof(char)*100);
	char it[10];
	int score[10]={0};
	memset(score,0,sizeof(score));

	int check_occurrence[26];
	memset(check_occurrence,-1,sizeof(check_occurrence));
	while(winner == -1)
	{
		count=0;
		memset(message,0,100);
		while(1)
		{
			int rand_number=rand()%MAX_NUMBER;
			rand_number+=1;
			if(check_occurrence[rand_number]==-1)
			{
				check_occurrence[rand_number]=1;
				sprintf(message,"%d",rand_number);
				break;
			}
			else
			{
				if(ifexhausted(check_occurrence,26))
				{
					printf("All the numbers are exhausted. Level 1 finished.\n");
					exhaust_flag=1;
					break;
				}
			}
		}
		if(exhaust_flag==1)
		{
			winner=find_max(score,playerNo);
		}
		else
		{
			printf("Random no generated: %s\n",message );
			strcpy(randomNoMessage,message);

			for(i=0;i<playerNo;i++)
			{
				if(disconnected[i] == 0)
				{

					sendMessage(clientSockets[i],message,num_);//num is for specifying the type
					FD_SET(clientSockets[i],&readfds);
					FD_SET(clientSockets[i],&tempfds);
					count++;
					printf("added %s to set\n",inet_ntoa(cl_sockInfo[i].sin_addr) );
				}
			}
			//		memset(input_from_clients,0,sizeof(input_from_clients));
			//count = playerNo;
			while(1)//checks the
			{
				//DISPLAY SCORES HERE
				if(count == 0)
				{
	                memset(message,0,100);
	                for(i=0;i<10;i++)
	                {
	                    sprintf(it,"%d",score[i]);
	                    strcat(message,it);
	                    strcat(message," ");
	                }
	                printf("Score string is :: %s\n",message);
	                for(i=0;i<playerNo;i++)
	                {
	                    sendMessage(clientSockets[i],message,score_);//score_ is for specifying the score type of message
	                }
					break;
				}
				printf("waiting for data on the sockets\n");
				int max_f = max_fd(clientSockets,disconnected,playerNo);
				//printf();
				select(max_f+1,&readfds,NULL,NULL,NULL);	//////DOUBT
				int i;
				for ( i= 0; i < playerNo; i++)
				{
					if (disconnected[i] == 0 && FD_ISSET(clientSockets[i],&readfds))
					{
						FD_CLR(clientSockets[i],&tempfds);//clear the fd from temp set
						p = readn(clientSockets[i],it,8);
						printf("p ========== %d\n",p );
						if(p == 0 || p == -1)
				        {
				            perror("Client got disconnected\n");
				            disconnected[i]  = 1;
				            num_of_disClients++;
				            continue;
				        }
//				        count--;
				        noOfBytes = atoi((char *)it);
				        int rb = readn(clientSockets[i],readmsg,noOfBytes);//reading the message
				        printf("from %s",inet_ntoa(cl_sockInfo[i].sin_addr));
				        int code = decodeMessage(readmsg,content,rb);
				        memset(readmsg,0,rb);
				        if(code == fnf_)
				        {
				        	if(atoi(content)==1)
				        	{
				        		score[i] += atoi(content);
				        	}
				        	count--;
				        	
				        	continue;
				        }
						else if(code == pause_)
				        {
				        	printf("entered in pause handling mechanism\n");
				        	fd_set temp;
				        	pause_cl = clientSockets[i];
				        	FD_SET(pause_cl,&temp);
				        	//send message to all the clients
				        	for(ag=0;ag<playerNo;ag++)
				        	{
				        		char rules[]="Game Paused";
				        		if(disconnected[ag] == 0)
				        			sendMessage(clientSockets[ag],rules,rule_);//send the message to all the clients that game is paused.
				        	}
				        	printf("pause_cl == %d\n",pause_cl );
				        	while(pause_cl)
				        	{
				        		printf("inside pause_cl\n");
				        		//select(pause_cl+1,&temp,NULL,NULL,NULL);
				        		printf("after select\n");
				        		p = readn(pause_cl,it,8);
				        		printf("p ==== in === %d\n",p );
				        		perror("error: \n");
				        		if(p == 0 || p == -1)
				        		{
				        			printf("Client got disconnected\n");
									disconnected[i] = 1;
									//count--;
									for(ag=0;ag<playerNo;ag++)
									{
										char rules[]="Game Resumed.";
										if(disconnected[ag] == 0)
											sendMessage(clientSockets[ag],rules,rule_);//send the message to all the clients that game is paused.
									}
									num_of_disClients++;
									YO_flag = 1;
				        		}
				        		else 
				        		{
				        			FD_SET(pause_cl,&tempfds);
				        			rb = readn(pause_cl,readmsg,noOfBytes);
				        			for(ag=0;ag<playerNo;ag++)
									{
										char rules[]="Game Resumed.";
										if(disconnected[ag] == 0)
											sendMessage(clientSockets[ag],rules,rule_);//send the message to all the clients that game is paused.
									}
									sendMessage(pause_cl,randomNoMessage,num_);
				        		}
				        		
								pause_cl = 0;
				        	}
				        }				        
					}
				}
				readfds = tempfds;//remaining descriptors are again reassigned
				if(num_of_disClients > 0)
				{
					int t,zp,flag = 0;
					fd_set abn;
					struct timeval timeout;
					timeout.tv_sec = 5;
					timeout.tv_usec = 0;
					FD_SET(listenSock,&abn);
					printf("will wait for 5 seconds\n");
					while(num_of_disClients > 0 && timeout.tv_sec > 0)
					{
						for(zp = 0;zp<10;zp++)
						{
							if(disconnected[zp] == 1)
								flag =1;
						}
						if(flag != 1)
							break;
						printf("before select\n");
						int sr = select(listenSock+1,&abn,NULL,NULL,&timeout);
						if(sr == 0)
						{
							for(zp = 0;zp<10;zp++)
							{
								if(disconnected[zp] == 1)
								{

									disconnected[zp] = 2;
									if(YO_flag==1)
									{
										YO_flag=0;
									}
									else
									{
										count--;
									}
									close(clientSockets[zp]);
								}
								num_of_disClients =0;
								
							}
						}
						else if(sr == 1)
						{
							if(FD_ISSET(listenSock,&abn))
							{

								struct sockaddr_in temp;
								printf("got data\n");
								while((t = accept(listenSock,(struct sockaddr *)&temp,&len_cl)) > 0)
								{
									for(zp = 0;zp<10;zp++)
									{
										if(disconnected[zp]  == 1)
										{
											if(strcmp(inet_ntoa(cl_sockInfo[zp].sin_addr),inet_ntoa(temp.sin_addr)) == 0)
											{
												printf("ip is %s\n",inet_ntoa(cl_sockInfo[zp].sin_addr));
												clientSockets[zp] = t;
												cl_sockInfo[zp] = temp;
												disconnected[zp] = 0;
												num_of_disClients--;
												if(YO_flag==1)
												{
													YO_flag=0;
												}
												else
												{
													count--;
												}
												encryptMatrixToMessage(message,4,matrix,zp);
												sendMessage(clientSockets[zp],message,mat_);
												sendMessage(clientSockets[zp],randomNoMessage,num_);//num is for specifying the type

											}
										}
									}
								}
							}
						}
						else
						{
							printf("select error\n");
						}
					}
				}
				printf("count = %d\n",count);
			}

			winner = check_winner(score,playerNo);

		}
		if(winner!=-1)
		{
			strcpy(message,"");
			sprintf(message,"%d",winner);
			for(i = 0; i < playerNo; ++i)
				sendMessage(clientSockets[i],message,winner_);

//			printf("Winner of level-1 is: %d\n",winner );
		}		
	}


////////////////////////////////////////////////////////////////////////////////////////////////////////

		//LEVEL 2

	printf("\n\n---------------------STARTING LEVEL 2------------------------\n");
	printf("Number of players is %d. Starting the game\n",playerNo);
	printf("client ip is %s\n",inet_ntoa(cl_sockInfo[0].sin_addr));
	exhaust_flag=0;
	winner=-1;
	YO_flag=0;
	//int no_of_players=10;              ////////////  Find out the number_of_players   /////////////////////////
	int matrix2[no_of_players][5][5];   ////////////  Create player matrix             /////////////////////////
	memset(matrix2,0,sizeof(matrix2));   ////////////  Initialize player matrix with 0  /////////////////////////
	//int i,j;                           ////////////  Initialize loop variables        /////////////////////////
	//int disconnected[10];
	num_of_disClients=0;
	memset(disconnected,0,10*sizeof(int));
	//////////////////////  Create matrices for all players and deploy them to each player  ///////////////////
	for(i=0;i<no_of_players;i++)
	{
		int temporary_matrix[MAX_NUMBER+1]={0};
		int random_number,row_number,column_number;
		for(j=0;j<15;j++)
		{
			while(1)        // This loop generates unique random numbers
			{
				random_number = generateRandomNumber (1,MAX_NUMBER);
				if(temporary_matrix [random_number] == 0)       // If the number has not been generated.....break
				{
					temporary_matrix[random_number]=1;
					break;
				}
			}
			while(1)       // This loop generates unique position for the generated random number
			{

				row_number = generateRandomNumber (0,4);
				column_number = generateRandomNumber (0,4);
				if(matrix2[i][row_number][column_number] == 0)
				{
					matrix2[i][row_number][column_number] = random_number;
					break;
				}
			}

		}
	}

	//char message[100];
	//char randomNoMessage[100];
	for(i=0;i<playerNo;i++)
	{
		encryptMatrixToMessage(message,5,matrix2,i);
		printf("%s\n",message );
		sendMessage(clientSockets[i],message,mat_);
	}
	printf("Matrices sent to clients\n");

	//int input_from_clients[playerNo];
	//int count,p,noOfBytes;
	//fd_set readfds,tempfds;
	//char *readmsg = (char *)malloc(sizeof(char)*100);
	//char *content = (char *)malloc(sizeof(char)*100);
	//char it[10];
	//int score[10]={0};
	//memset(score,0,sizeof(score));

	//int check_occurrence[26];
	memset(check_occurrence,-1,sizeof(check_occurrence));
	while(winner == -1)
	{
		count=0;
		memset(message,0,100);
		while(1)
		{
			int rand_number=rand()%MAX_NUMBER;
			rand_number+=1;
			if(check_occurrence[rand_number]==-1)
			{
				check_occurrence[rand_number]=1;
				sprintf(message,"%d",rand_number);
				break;
			}
			else
			{
				if(ifexhausted(check_occurrence,26))
				{
					printf("All the numbers are exhausted. Level 1 finished.\n");
					exhaust_flag=1;
					break;
				}
			}
		}
		if(exhaust_flag==1)
		{
			winner=find_max(score,playerNo);
		}
		else
		{
			printf("Random no generated: %s\n",message );
			strcpy(randomNoMessage,message);

			for(i=0;i<playerNo;i++)
			{
				if(disconnected[i] == 0)
				{

					sendMessage(clientSockets[i],message,num_);//num is for specifying the type
					FD_SET(clientSockets[i],&readfds);
					FD_SET(clientSockets[i],&tempfds);
					count++;
					printf("added %s to set\n",inet_ntoa(cl_sockInfo[i].sin_addr) );
				}
			}
			//		memset(input_from_clients,0,sizeof(input_from_clients));
			//count = playerNo;
			while(1)//checks the
			{
				//DISPLAY SCORES HERE
				if(count == 0)
				{
	                memset(message,0,100);
	                for(i=0;i<10;i++)
	                {
	                    sprintf(it,"%d",score[i]);
	                    strcat(message,it);
	                    strcat(message," ");
	                }
	                printf("Score string is :: %s\n",message);
	                for(i=0;i<playerNo;i++)
	                {
	                    sendMessage(clientSockets[i],message,score_);//score_ is for specifying the score type of message
	                }
					break;
				}
				printf("waiting for data on the sockets\n");
				int max_f = max_fd(clientSockets,disconnected,playerNo);
				//printf();
				select(max_f+1,&readfds,NULL,NULL,NULL);	//////DOUBT
				int i;
				for ( i= 0; i < playerNo; i++)
				{
					if (disconnected[i] == 0 && FD_ISSET(clientSockets[i],&readfds))
					{
						FD_CLR(clientSockets[i],&tempfds);//clear the fd from temp set
						p = readn(clientSockets[i],it,8);
						printf("p ========== %d\n",p );
						if(p == 0 || p == -1)
				        {
				            perror("Client got disconnected\n");
				            disconnected[i]  = 1;
				            num_of_disClients++;
				            continue;
				        }
//				        count--;
				        noOfBytes = atoi((char *)it);
				        int rb = readn(clientSockets[i],readmsg,noOfBytes);//reading the message
				        printf("from %s",inet_ntoa(cl_sockInfo[i].sin_addr));
				        int code = decodeMessage(readmsg,content,rb);
				        memset(readmsg,0,rb);
			        	printf("Lifeline String: %s\n",content );

				        if(code == fnf_)
				        {
//				        	printf("Lifeline: %d\n",atoi(content) );
				        	if(atoi(content)==1)
				        	{
				        		printf("Increment!\n");
				        		score[i] += atoi(content);
				        	}
				        	else if(atoi(content)==5)
				        	{
				        		score[i]=(int)(score[i]*1.5);
				        	}
				        	else if(atoi(content)==6)
				        	{
				        		score[i]=(int)(score[i]/1.5);
				        	}
				        	else if(atoi(content)==7)
				        	{
				        		score[i]+=2;
				        	}

				        	count--;
				        	continue;
				        }
						else if(code == pause_)
				        {
				        	printf("entered in pause handling mechanism\n");
				        	fd_set temp;
				        	pause_cl = clientSockets[i];
				        	FD_SET(pause_cl,&temp);
				        	//send message to all the clients
				        	for(ag=0;ag<playerNo;ag++)
				        	{
				        		char rules[]="Game Paused";
				        		if(disconnected[ag] == 0)
				        			sendMessage(clientSockets[ag],rules,rule_);//send the message to all the clients that game is paused.
				        	}
				        	printf("pause_cl == %d\n",pause_cl );
				        	while(pause_cl)
				        	{
				        		printf("inside pause_cl\n");
				        		//select(pause_cl+1,&temp,NULL,NULL,NULL);
				        		printf("after select\n");
				        		p = readn(pause_cl,it,8);
				        		printf("p ==== in === %d\n",p );
				        		perror("error: \n");
				        		if(p == 0 || p == -1)
				        		{
				        			printf("Client got disconnected\n");
									disconnected[i] = 1;
									//count--;
									for(ag=0;ag<playerNo;ag++)
									{
										char rules[]="Game Resumed.";
										if(disconnected[ag] == 0)
											sendMessage(clientSockets[ag],rules,rule_);//send the message to all the clients that game is paused.
									}
									num_of_disClients++;
									YO_flag = 1;
				        		}
				        		else 
				        		{
				        			FD_SET(pause_cl,&tempfds);
				        			rb = readn(pause_cl,readmsg,noOfBytes);
				        			for(ag=0;ag<playerNo;ag++)
									{
										char rules[]="Game Resumed.";
										if(disconnected[ag] == 0)
											sendMessage(clientSockets[ag],rules,rule_);//send the message to all the clients that game is paused.
									}
									sendMessage(pause_cl,randomNoMessage,num_);
				        		}
				        		
								pause_cl = 0;
				        	}
				        }				        
					}
				}
				readfds = tempfds;//remaining descriptors are again reassigned
				if(num_of_disClients > 0)
				{
					int t,zp,flag = 0;
					fd_set abn;
					struct timeval timeout;
					timeout.tv_sec = 5;
					timeout.tv_usec = 0;
					FD_SET(listenSock,&abn);
					printf("will wait for 5 seconds\n");
					while(num_of_disClients > 0 && timeout.tv_sec > 0)
					{
						for(zp = 0;zp<10;zp++)
						{
							if(disconnected[zp] == 1)
								flag =1;
						}
						if(flag != 1)
							break;
						printf("before select\n");
						int sr = select(listenSock+1,&abn,NULL,NULL,&timeout);
						if(sr == 0)
						{
							for(zp = 0;zp<10;zp++)
							{
								if(disconnected[zp] == 1)
								{

									disconnected[zp] = 2;
									if(YO_flag==1)
									{
										YO_flag=0;
									}
									else
									{
										count--;
									}
									close(clientSockets[zp]);
								}
								num_of_disClients =0;
								
							}
						}
						else if(sr == 1)
						{
							if(FD_ISSET(listenSock,&abn))
							{

								struct sockaddr_in temp;
								printf("got data\n");
								while((t = accept(listenSock,(struct sockaddr *)&temp,&len_cl)) > 0)
								{
									for(zp = 0;zp<10;zp++)
									{
										if(disconnected[zp]  == 1)
										{
											if(strcmp(inet_ntoa(cl_sockInfo[zp].sin_addr),inet_ntoa(temp.sin_addr)) == 0)
											{
												printf("ip is %s\n",inet_ntoa(cl_sockInfo[zp].sin_addr));
												clientSockets[zp] = t;
												cl_sockInfo[zp] = temp;
												disconnected[zp] = 0;
												num_of_disClients--;

												if(YO_flag==1)
												{
													YO_flag=0;
												}
												else
												{
													count--;
												}
												encryptMatrixToMessage(message,5,matrix2,zp);
												sendMessage(clientSockets[zp],message,mat_);
												sendMessage(clientSockets[zp],randomNoMessage,num_);//num is for specifying the type
											}
										}
									}
								}
							}
						}
						else
						{
							printf("select error\n");
						}
					}
				}
				printf("count = %d\n",count);
			}

			winner = check_winner(score,playerNo);

			}

		if(winner!=-1)
		{
			strcpy(message,"");
			sprintf(message,"%d",winner);
			for(i = 0; i < playerNo; ++i)
				sendMessage(clientSockets[i],message,winner_);

//			printf("Winner is %d\n",winner );
		}
	}


////////////////////////////////////////////	LEVEL 3 	//////////////////////////////////////////////////
	

	printf("\n\n---------------------STARTING LEVEL 3------------------------\n");
	printf("Number of players is %d. Starting the game\n",playerNo);
	printf("client ip is %s\n",inet_ntoa(cl_sockInfo[0].sin_addr));
	exhaust_flag=0;
	winner=-1;
	YO_flag=0;
	//int no_of_players=10;              ////////////  Find out the number_of_players   /////////////////////////
	//int matrix2[no_of_players][5][5];    ////////////  Create player matrix             /////////////////////////
	memset(matrix2,0,sizeof(matrix2));   ////////////  Initialize player matrix with 0  /////////////////////////
	//int i,j;                           ////////////  Initialize loop variables        /////////////////////////
	//int disconnected[10];
	num_of_disClients=0;
	memset(disconnected,0,10*sizeof(int));
	//////////////////////  Create matrices for all players and deploy them to each player  ///////////////////
	for(i=0;i<no_of_players;i++)
	{
		int temporary_matrix[MAX_NUMBER+1]={0};
		int random_number,row_number,column_number;
		for(j=0;j<25;j++)
		{
			while(1)        // This loop generates unique random numbers
			{
				random_number = generateRandomNumber (1,MAX_NUMBER);
				if(temporary_matrix [random_number] == 0)       // If the number has not been generated.....break
				{
					temporary_matrix[random_number]=1;
					break;
				}
			}
			while(1)       // This loop generates unique position for the generated random number
			{

				row_number = generateRandomNumber (0,4);
				column_number = generateRandomNumber (0,4);
				if(matrix2[i][row_number][column_number] == 0)
				{
					matrix2[i][row_number][column_number] = random_number;
					break;
				}
			}

		}
	}

	//char message[100];
	//char randomNoMessage[100];
	for(i=0;i<playerNo;i++)
	{
		encryptMatrixToMessage(message,5,matrix2,i);
		printf("%s\n",message );
		sendMessage(clientSockets[i],message,mat_);
	}
	printf("Matrices sent to clients\n");

	//int input_from_clients[playerNo];
	//int count,p,noOfBytes;
	//fd_set readfds,tempfds;
	//char *readmsg = (char *)malloc(sizeof(char)*100);
	//char *content = (char *)malloc(sizeof(char)*100);
	//char it[10];
	//int score[10]={0};
	//memset(score,0,sizeof(score));

	//int check_occurrence[26];
	memset(check_occurrence,-1,sizeof(check_occurrence));
	while(winner == -1)
	{
		count=0;
		memset(message,0,100);
		while(1)
		{
			int rand_number=rand()%MAX_NUMBER;
			rand_number+=1;
			if(check_occurrence[rand_number]==-1)
			{
				check_occurrence[rand_number]=1;
				sprintf(message,"%d",rand_number);
				break;
			}
			else
			{
				if(ifexhausted(check_occurrence,26))
				{
					printf("All the numbers are exhausted. Level 1 finished.\n");
					exhaust_flag=1;
					break;
				}
			}
		}
		if(exhaust_flag==1)
		{
			winner=find_max(score,playerNo);
		}
		else
		{
			printf("Random no generated: %s\n",message );
			strcpy(randomNoMessage,message);

			for(i=0;i<playerNo;i++)
			{
				if(disconnected[i] == 0)
				{

					sendMessage(clientSockets[i],message,num_);//num is for specifying the type
					FD_SET(clientSockets[i],&readfds);
					FD_SET(clientSockets[i],&tempfds);
					count++;
					printf("added %s to set\n",inet_ntoa(cl_sockInfo[i].sin_addr) );
				}
			}
			//		memset(input_from_clients,0,sizeof(input_from_clients));
			//count = playerNo;
			while(1)//checks the
			{
				//DISPLAY SCORES HERE
				if(count == 0)
				{
	                memset(message,0,100);
	                for(i=0;i<10;i++)
	                {
	                    sprintf(it,"%d",score[i]);
	                    strcat(message,it);
	                    strcat(message," ");
	                }
	                printf("Score string is :: %s\n",message);
	                for(i=0;i<playerNo;i++)
	                {
	                    sendMessage(clientSockets[i],message,score_);//score_ is for specifying the score type of message
	                }
					break;
				}
				printf("waiting for data on the sockets\n");
				int max_f = max_fd(clientSockets,disconnected,playerNo);
				//printf();
				select(max_f+1,&readfds,NULL,NULL,NULL);	//////DOUBT
				int i;
				for ( i= 0; i < playerNo; i++)
				{
					if (disconnected[i] == 0 && FD_ISSET(clientSockets[i],&readfds))
					{
						FD_CLR(clientSockets[i],&tempfds);//clear the fd from temp set
						p = readn(clientSockets[i],it,8);
						printf("p ========== %d\n",p );
						if(p == 0 || p == -1)
				        {
				            perror("Client got disconnected\n");
				            disconnected[i]  = 1;
				            num_of_disClients++;
				            continue;
				        }
//				        count--;
				        noOfBytes = atoi((char *)it);
				        int rb = readn(clientSockets[i],readmsg,noOfBytes);//reading the message
				        printf("from %s",inet_ntoa(cl_sockInfo[i].sin_addr));
				        int code = decodeMessage(readmsg,content,rb);
				        memset(readmsg,0,rb);
			        	//printf("Lifeline String: %s\n",content );

				        if(code == fnf_)
				        {
//				        	printf("Lifeline: %d\n",atoi(content) );
				        	if(atoi(content)==1)
				        	{
				        		//printf("Increment!\n");
				        		score[i] += atoi(content);
				        	}
				        	else if(atoi(content)==5)
				        	{
				        		score[i] += 5;
				        		winner = i;
				        	}
				        	count--;
				        	continue;
				        }
						else if(code == pause_)
				        {
				        	printf("entered in pause handling mechanism\n");
				        	fd_set temp;
				        	pause_cl = clientSockets[i];
				        	FD_SET(pause_cl,&temp);
				        	//send message to all the clients
				        	for(ag=0;ag<playerNo;ag++)
				        	{
				        		char rules[]="Game Paused";
				        		if(disconnected[ag] == 0)
				        			sendMessage(clientSockets[ag],rules,rule_);//send the message to all the clients that game is paused.
				        	}
				        	printf("pause_cl == %d\n",pause_cl );
				        	while(pause_cl)
				        	{
				        		printf("inside pause_cl\n");
				        		//select(pause_cl+1,&temp,NULL,NULL,NULL);
				        		printf("after select\n");
				        		p = readn(pause_cl,it,8);
				        		printf("p ==== in === %d\n",p );
				        		perror("error: \n");
				        		if(p == 0 || p == -1)
				        		{
				        			printf("Client got disconnected\n");
									disconnected[i] = 1;
									//count--;
									for(ag=0;ag<playerNo;ag++)
									{
										char rules[]="Game Resumed.";
										if(disconnected[ag] == 0)
											sendMessage(clientSockets[ag],rules,rule_);//send the message to all the clients that game is paused.
									}
									num_of_disClients++;
									YO_flag = 1;
				        		}
				        		else 
				        		{
				        			FD_SET(pause_cl,&tempfds);
				        			rb = readn(pause_cl,readmsg,noOfBytes);
				        			for(ag=0;ag<playerNo;ag++)
									{
										char rules[]="Game Resumed.";
										if(disconnected[ag] == 0)
											sendMessage(clientSockets[ag],rules,rule_);//send the message to all the clients that game is paused.
									}
									sendMessage(pause_cl,randomNoMessage,num_);
				        		}
				        		
								pause_cl = 0;
				        	}
				        }					
				    }
				}
				readfds = tempfds;//remaining descriptors are again reassigned
				if(num_of_disClients > 0)
				{
					int t,zp,flag = 0;
					fd_set abn;
					struct timeval timeout;
					timeout.tv_sec = 5;
					timeout.tv_usec = 0;
					FD_SET(listenSock,&abn);
					printf("will wait for 5 seconds\n");
					while(num_of_disClients > 0 && timeout.tv_sec > 0)
					{
						for(zp = 0;zp<10;zp++)
						{
							if(disconnected[zp] == 1)
								flag =1;
						}
						if(flag != 1)
							break;
						printf("before select\n");
						int sr = select(listenSock+1,&abn,NULL,NULL,&timeout);
						if(sr == 0)
						{
							for(zp = 0;zp<10;zp++)
							{
								if(disconnected[zp] == 1)
								{

									disconnected[zp] = 2;
									if(YO_flag==1)
									{
										YO_flag=0;
									}
									else
									{
										count--;
									}
									close(clientSockets[zp]);
								}
								num_of_disClients =0;
								
							}
						}
						else if(sr == 1)
						{
							if(FD_ISSET(listenSock,&abn))
							{

								struct sockaddr_in temp;
								printf("got data\n");
								while((t = accept(listenSock,(struct sockaddr *)&temp,&len_cl)) > 0)
								{
									for(zp = 0;zp<10;zp++)
									{
										if(disconnected[zp]  == 1)
										{
											if(strcmp(inet_ntoa(cl_sockInfo[zp].sin_addr),inet_ntoa(temp.sin_addr)) == 0)
											{
												printf("ip is %s\n",inet_ntoa(cl_sockInfo[zp].sin_addr));
												clientSockets[zp] = t;
												cl_sockInfo[zp] = temp;
												disconnected[zp] = 0;
												num_of_disClients--;
												if(YO_flag==1)
												{
													YO_flag=0;
												}
												else
												{
													count--;
												}
												encryptMatrixToMessage(message,5,matrix2,zp);
												sendMessage(clientSockets[zp],message,mat_);
												sendMessage(clientSockets[zp],randomNoMessage,num_);//num is for specifying the type
											}
										}
									}
								}
							}
						}
						else
						{
							printf("select error\n");
						}
					}
				}
				printf("count = %d\n",count);
			}

//			winner = check_winner(score,playerNo);

			}

		if(winner!=-1)
		{
			strcpy(message,"");
			sprintf(message,"%d",winner);
			for(i = 0; i < playerNo; ++i)
				sendMessage(clientSockets[i],message,winner_);


			int max=0;
			for(i=0;i<playerNo;i++)
			{
				if(max<score[i])
				{
					max=score[i];
				}
			}

			printf("Winners of the game: ");
			for(i=0;i<playerNo;i++)
			{
				if(score[i]==max)
				{
					printf("%d ",i);
				}
			}
			printf("\n");

//			printf("Winner is %d\n",winner );
		}
	}

	for(i=0;i<playerNo;i++)
		close(clientSockets[i]);


	
	return 0;

}



