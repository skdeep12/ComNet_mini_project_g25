#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


void cut(int n,int number,int matrix[][n]){
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
int bingo_count=0;

void bingo(int n,int matrix[][n],int row_array[],int col_array[],int diag_array[]){
	int i,j;
		int row=0,col=0,diag1=0,diag2=0;
		for(i=0;i<n;i++){
			for(j=0;j<n;j++){
				if(row_array[i]==1){
					break;
				}
				if(matrix[i][j]==-1&&row_array[i]==0){
					row=1;
			}
			else if(matrix[i][j]!=-1){
					row=0;
					break;
			}
		}
		if(row==1){
			row_array[i] = 1;
			bingo_count++;
			break;
	}	
}
		for(j=0;j<n;j++){
			for(i=0;i<n;i++){
				if(col_array[j]==1)break;
				if(matrix[i][j]==-1&&col_array[j]==0){
					col=1;
			}
			else if(matrix[i][j]!=-1){
					col=0;
					break;
			}
		}
		if(col==1){
			col_array[j] = 1;
			bingo_count++;
			break;
	}
}
	for(i=0;i<n;i++){
		if(diag_array[0]==1)break;
		if(matrix[i][i]==-1&&diag_array[0]==0){
			diag1=1;		
		}
		else if(matrix[i][i]!=-1){
			diag1=0;
			break;
		}
	}
	if(diag1==1){
		diag_array[0]=1;
		bingo_count++;
	}
	for(i=n-1;i>=0;i--){
		if(diag_array[1]==1)break;
		if(matrix[i][n-1-i]==-1&&diag_array[1]==0){
			diag2=1;		
		}
		else if(matrix[i][n-1-i]!=-1){
			diag2=0;
			break;
		}
	}
	if(diag2==1){
		diag_array[1]=1;
		bingo_count++;
	}
}
int main(void){
	int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
	int t = 0;
	int row_array[]={0,0,0};
	int col_array[]={0,0,0};
	int diag_array[2]={0,0};
	int num;
	while(t!=5){
		scanf("%d",&num);
		cut(3,num,matrix);
		bingo(3,matrix,row_array,col_array,diag_array);
		int i,j;
		for(i=0;i<3;i++){
			for(j=0;j<3;j++){
				printf("%d ",matrix[i][j] );
			}
			printf("\n");
		}
		t++;
	}
	printf("%d\n",bingo_count );


}