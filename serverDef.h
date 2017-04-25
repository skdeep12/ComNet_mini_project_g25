/*
 * serverDef.h
 *
 *  Created on: 24-Apr-2017
 *      Author: drogon
 */

#ifndef SERVERDEF_H_
#define SERVERDEF_H_
#include <ctype.h>
#include<stdlib.h>
#include<string.h>
//#include <strstr.h>
ssize_t readn(int fd,char *vptr,size_t n);
ssize_t writen(int fd,char *vptr,size_t n);

int decodeMessage(char* input,char output[],int readBytes)
{
	int x=0,i=0;
	char num[10];
	//output=(char*)malloc(readBytes*sizeof(char));
	strcpy(output,"");
	while(input[i]!=':')
		i++;
	i++;
	int k=0;
	while(input[i]!='\n')
		num[k++] = input[i++];
	num[k] = '\0';
	x = atoi(num);
	i++;
	int j=0;
	while(i<readBytes)
		output[j++]=input[i++];
	output[j]='\0';
	return x;
  /*  int i=16,k=0;
    while(input[i]!='\0')
    {
        output[k++]=input[i++];
    }
    output[k]='\0';
    return input[14]-48;
*/
}
ssize_t writen(int fd,char *vptr,size_t n)
{
    size_t nleft = n;
    ssize_t nwritten ;
    char *ptr = vptr;
    while(nleft > 0)
    {
        if ((nwritten = write(fd,ptr,nleft))  < 0)
        {
            if(errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft-=nwritten;
        ptr+=nwritten;
    }
    return n;
}
ssize_t readn(int fd,char* vptr,size_t n)
{
    size_t nleft;
    ssize_t nread;
    char* ptr;
    ptr = vptr;
    nleft=n;
    while(nleft > 0)
    {
        nread = read(fd,ptr,nleft);
        if (nread  < 0 )
        {
            if(errno == EINTR)
                nread = 0; //call read again
            else
                return -1;
        }
        else if (nread == 0)
        {
            return n-nleft;
        }
        nleft-=nread;
        ptr+=nread;
    }
    return (n - nleft);
}


#endif /* SERVERDEF_H_ */
