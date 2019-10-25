#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<string.h>
#include<ctype.h>
#include<math.h>
#include<stdlib.h>


#define MAX 40
#define BUFFER_SIZE 4096
#define READ_END 0
#define WRITE_END 1
int N = 0; 

void pipeExec(char **first, char **second){
	int fdpipe1[2]; //file desc. pipe1 of the first child
	int fdpipe2[2]; //fd pipe2 of the second child
	pid_t pid1,pid2; //child processes
	char read_msg[N];

	//creating pipes
	if(pipe(fdpipe1) == -1 ){fprintf(stderr,"Pipe1 Failed"); }

	if(pipe(fdpipe2) == -1 ){fprintf(stderr,"Pipe2 Failed"); }

	//creating child1 and executing first command
	pid1 = fork();
	if(pid1 < 0){fprintf(stderr,"Fork Failed(Child-1)"); }

	//write end is open while read and is closed?
	if (pid1 == 0){
	dup2(fdpipe1[1],1);

	close(fdpipe1[READ_END]);

	
		execvp(first[0],first);
		
	//close(fdpipe1[WRITE_END]);					//SEE HERE
	} 
	else { //parent process
	//wait(NULL);
	close(fdpipe1[WRITE_END]); //close write end (index 1) - it is unused here
	//close(fdpipe2[READ_END]);


	//int n = read(fdpipe1[READ_END], read_msg, N);
	//printf("Number of characters read %d",n);

	int n = 0;
	int m = 0;
	while( n = read(fdpipe1[READ_END], read_msg, N)){
		m = m + n;
		write(fdpipe2[WRITE_END], read_msg ,(int) strlen(read_msg) + 1);
	}
	
	printf("Number of characters read %d",m);
	
	//int m = write(fdpipe2[WRITE_END], read_msg ,(int) strlen(read_msg) + 1);
	//printf("Number of characters wrote %d",m);


	//create second child
	pid2 = fork();
	if(pid2 == 0){ //second child process
		//close(fdpipe2[WRITE_END]); //close write end (index 1) - it is unused here
		//close(fdpipe1[READ_END]);
		//close(fdpipe1[WRITE_END]);
		//printf("second child read \n");
		dup2(fdpipe2[0],0);
		close(fdpipe2[WRITE_END]);
		/*
		char *argex2[3];
		argex2[0] = "sort";
		argex2[1] = NULL;

		printf("*****check2******** \n");
		*/
		execvp(second[0],second);
		//close(fdpipe2[READ_END]);
	}
	//else
		//wait(NULL);

}}


/**
 *  buffer arguments delimited by space " "
 * 	assuming given argument has redundant character at the end
 *  comp is 0 if it is non compound
 */
void divideArgs(char *buff, char **args, int comp){
	if(comp == 0){
	char divide[] = " ";
	char *strptr = strtok(buff,divide);
	args[0] = strptr;
	char temp[MAX];
	strcpy(temp,"/bin/");
	strcat(temp,args[0]);
	
	
	args[0] = temp;
	//args[0][strlen(args[0]) - 1] = '\0';
	int count = 1;
	while(strptr != NULL ){	
		strptr = strtok(NULL,divide);
		args[count] = strptr;
		count++;
	}
	count--;
	//count--;
	count--;
    args[count][strlen(args[count]) - 1] = '\0';
}
	else if(comp == 1){
	char divide2[] = "|";
	char *strptr2 = strtok(buff,divide2);
	args[0] = strptr2;
	char temp2[MAX];
	strcpy(temp2,"");
	strcat(temp2,args[0]);
	
	
	args[0] = temp2;
	int count2 = 1;
	while(strptr2 != NULL ){	
		strptr2 = strtok(NULL,divide2);
		args[count2] = strptr2;
		count2++;
	}
	count2--;
	//count--;
	count2--;
    args[count2][strlen(args[count2]) - 1] = '\0';
	}
	else{
	char divide[] = " ";
	char *strptr = strtok(buff,divide);
	args[0] = strptr;
	char temp[MAX];
	strcpy(temp,"");
	strcat(temp,args[0]);
	
	//args[0] = temp;
	//args[0][strlen(args[0]) - 1] = '\0';
	int count = 1;
	while(strptr != NULL ){	
		strptr = strtok(NULL,divide);
		args[count] = strptr;
		count++;
	}
	count--;
	//count--;
	count--;
    //args[count][strlen(args[count]) - 1] = '\0';
	}
}


/**
 * return 1 if it is compound statement
 */
int checkCompound(char *args){
	int comp = 0;
	int i = 0;
	while(args[i] != '\0'){
		if(args[i] == '|')
			comp = 1;
		i++;
	}
	return comp;
}

/**
 * return 1 if it is batch statement
 */
int checkBatch(char *args){
	int comp = 0;
	int temp = 0;
	
	while(isdigit(args[temp]) || (args[temp] == ' ') ){
	temp++;
	}
	
	//char tempBuff2[MAX];
	int count = 0;
	while(args[temp] != ' ' && args[temp] != '\0'){
		//tempBuff2[count] = args[temp];
		if(args[temp] == '.')
			comp = 1;
		count++;
		temp++;
	}	
	
	return comp;
}

void execute(char *buff){
	if(checkCompound(buff) == 0){
	char *args[MAX];
	divideArgs(buff,args,0);


	
	pid_t exec;
	exec = fork();
	if(exec < 0){
		printf("Exec Fork Failed \n");	
	}
	
	if(exec == 0){
		execve(args[0], args, NULL);
	}
	}
	else{
		char *args2[MAX];
		char *firstCommand[MAX];
		char *secCommand[MAX];
		divideArgs(buff,args2,1);			//may be no need for &&

		char temparg[MAX];
		memcpy(temparg,args2[0],MAX);
		divideArgs(temparg,firstCommand,3);
		
		char temparg2[MAX];
		memcpy(temparg2,args2[1],MAX);
		divideArgs(temparg2,secCommand,3);	
		

		
		
		pipeExec(firstCommand,secCommand);
		
	}
		
}

void readFile(char *str){
	char ch[50];
	FILE *txtFile; 
	str[(int)strlen(str) - 1] = '\0';
	txtFile = fopen(str,"r");
	if(txtFile == NULL){
		printf("No such file \n");
		return;
		
	}
	while (fgets(ch, 50, txtFile) != NULL ) {
			execute(ch);
    }
    fclose(txtFile);
}



unsigned long toint(char* str)
{
unsigned long m = 1;
unsigned long r = 0;
//length of string
int length = strlen(str);
//for loop to convert each char
for(int i = length -1 ; i >= 0 ; i--)
{
r = r + ((int)str[i] -48) * m;
m = m * 10;
}
return r;
}
int main(){

	/*while(1){
		pid_t theWhile;
		theWhile = fork();
		if(theWhile == 0){*/
	char buffer[MAX];
	//printf("bilshell$:",fgets(buffer,MAX,stdin);
	if(fgets(buffer,MAX,stdin) != NULL){
		printf("Input is: %s \n", buffer);
	}
	int index = 0;
	while(buffer[index] == ' ')
		index++;	
	if(checkBatch(buffer) == 1){ //the command is batch
		printf("Batch: %s \n", buffer);
		int temp = index;
		//string which contains N;
		char str[MAX] = "";
		while(isdigit(buffer[temp])){
		char tempAr[4] = "b";
		tempAr[0] = buffer[temp];
		strcat(str,tempAr);
		temp++;
		}
		index = temp;
		printf("My string %s \n", str);
		//finding the number N
		N = (int) toint(str);
		printf("N is **** %d \n", N);
		//taking name of the file
		index++;
		while(buffer[index] == ' ')
		index++;
		
		char str2[MAX] = "";
		while(buffer[index] != ' ' && buffer[index] != '\0'){
			char tempAr2[4] = "b";
			tempAr2[0] = buffer[index];
			strcat(str2,tempAr2);			
			index++;
		}
		printf("My file is %s \n", str2);	
		readFile(str2);
		
	}
	else
	{
		printf("Interactive %s \n", buffer);
		
		int temp = index;
		//string which contains N;
		char str[MAX] = "";
		while(isdigit(buffer[temp])){
		char tempAr[4] = "b";
		tempAr[0] = buffer[temp];
		strcat(str,tempAr);
		temp++;
		}
		index = temp;
		//finding the number N
		N = (int) toint(str);
		printf("N is **** %d \n", N);
		//taking name of the file
		index++;
		while(buffer[index] == ' ')
		index++;
		
		char str2[MAX] = "";
		while( buffer[index] != '\0'){
			char tempAr2[4] = "b";
			tempAr2[0] = buffer[index];
			strcat(str2,tempAr2);			
			index++;
		}	
		execute(str2);

	}
/*	exit(0);
}
	else
		wait(NULL);
}*/
	return 0;
}
