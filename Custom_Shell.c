#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/wait.h>
#include <fcntl.h>
int main()
{
	//1. Character Array
	int i;
	int total_commands=10;
	int buffer_size=4096; // here the reason for choosing for 4096 is that a command line can take 4096 bytes from terminal in a single go
	int iterator=0;
	char history[total_commands][buffer_size];
	char buffer[buffer_size]; 
	for(i=0; i<buffer_size ;i++)
	{
	    buffer[i]='\0'; // initializing from \0 character otherwise buffer has garbage 
	}
	bool check=true;
	int n=0; // for nth command after !
	printf("--------Custom Shell----------\n");
	char cwd[256];
	while(check) //5. Repeatition of the same process
	{	
		if(getcwd(cwd,sizeof(cwd))==NULL)
		{
				perror("Getcwd error\n");
		}
		//printf("Enter Command: \n");
		printf("hafsah@hafsah-VirtualBox: ~ %s $: ",cwd);
		fflush(stdout);
		int sze=read(0,buffer,buffer_size);
		buffer[sze-1]='\0';
		strcpy(history[iterator% total_commands], buffer); //copying from buffer every command and overwriting incase exceeds 10
		//For Exiting
		if (strcmp(buffer, "exit") == 0)
      		{
		    exit(0);
		}
		//For checking History
		if(strcmp(buffer, "history") == 0)
		{
			iterator--;
			for(int j=iterator;j>=0;j--)   
			{
				printf("%d . %s\n",(j+1),history[(j)%total_commands]);
			}
			
		}
		if(strcmp(buffer, "!!") == 0)
		{
			if(iterator>0) //Basic Error Checking
			{
				strcpy(buffer,history[iterator-1]);
				strcpy(history[iterator],history[iterator-1]);
			}
			else
			{
				printf("No Commands in HIstory\n");
			}
		}
		if(buffer[0]=='!' && buffer[1]>='1' && buffer[1]<= '9' &&buffer[2]=='\0' )
		{
			n=(buffer[1])-48;
			if((iterator%total_commands)>=n) //Basic Error Checking
			{
				strcpy(buffer,history[n-1]);
				strcpy(history[iterator],history[n-1]);
			}
			else
			{
				printf("No such Commands in HIstory\n");
			}
		}
		    	
		//2. Tokenization : Seprating the commands and it's arguments
		char *command[10];
		
		command[0]=strtok(buffer," "); //tokenizing using built-in function -->strtok
		if(command[0]!=NULL)
		{
			i=0;
			int cmds=0;
			while(command[i]!=NULL)
			{
				i++;
				command[i]=strtok(NULL," ");
				cmds++;
			}
			command[i]=NULL;
			if(strcmp(command[0], "cd") == 0)
			{
			    if(chdir(command[1]) != 0)
			    {
				perror("cd");
			    }
			    continue;
			}
			// checking if pipe exists
			bool if_pipe_exists = false;
			bool if_output_exists = false;
			bool if_input_exists = false;
			bool if_error_exists = false;
			int positions_pipe[10] = {-1};// this will be used later for combination of < | >
			int positions_output[10] = {-1};// this will be used later for combination of < | >
			int positions_input[10] = {-1};// this will be used later for combination of < | >
			int k=0; // pipe count
			int e=0; // output redirection count
			int o=0; // input redirection count
			int err=0; // error redirection count
			for(i=0; i<cmds; i++)
			{
			    if(strcmp(command[i], "|") == 0)
			    {
				if_pipe_exists = true;
				positions_pipe[k] = i; //the index where pipe exists
				k++; //pipe count
				
			    }
			    
			    if(strcmp(command[i], ">") == 0)
			    {
				if_output_exists = true;
				positions_output[e] = i; //the index where > exists
				e++; //output redirection count
				
			    }
		
			    if(strcmp(command[i], "<") == 0)
			    {
				if_input_exists = true;
				positions_input[o] = i; //the index where pipe exists
				o++; //input redirection count
				
			    }
			    
			    if(strcmp(command[i], "2>") == 0)
			    {
				if_error_exists = true;
				positions_input[err] = i; //the index where pipe exists
				err++; //input redirection count
				
			    }
			}
			if(if_output_exists || if_input_exists || if_error_exists && !if_pipe_exists)
			{
				i=cmds; 
				char *input_file = NULL;
				char *output_file = NULL;
				char *error_file = NULL;
				for (int j = 0; j < i; j++) 
				{
				    if (strcmp(command[j], "<") == 0 && command[j+1] != NULL) 
				    {
				      input_file = command[j+1];
				      // Removing the "<" and the input filename from the command
				      for (int k = j; k < i-2; k++) 
				      {
					command[k] = command[k+2];
				      }
				      command[i-2] = NULL;
				      command[i-1] = NULL;
				      i -= 2;
				      j--;
				    } 
				    else if (strcmp(command[j], ">") == 0 && command[j+1] != NULL) 
				    {
				      output_file = command[j+1];
				      // Removing the ">" and the output filename from the command
				      for (int k = j; k < i-2; k++) 
				      {
					command[k] = command[k+2];
				      }
				      command[i-2] = NULL;
				      command[i-1] = NULL;
				      i -= 2;
				      j--;
				    }
					   
					else if (strcmp(command[j], "2>") == 0 && command[j+1] != NULL)
					{
					    error_file = command[j+1];
					    // Removing the "2>" and the error filename from the command
					    for (int k = j; k < i-2; k++)
					    {
						command[k] = command[k+2];
					    }
					    command[i-2] = NULL;
					    command[i-1] = NULL;
					    i -= 2;
					    j--;
					}
				 }
				  
				pid_t r = fork();
				if (r == 0) 
				{
				  // Input redirection
				  if (input_file != NULL) 
				  {
				    int fd = open(input_file, O_RDONLY);
				    dup2(fd, 0);
				    close(fd);
				  }
				  //Error redirection
				  if (error_file != NULL) 
				  {
				    int fd = open(error_file, O_WRONLY | O_CREAT );
				    dup2(fd, 1);
				    close(fd);
				  }
				  
				  // Output redirection
				  if (output_file != NULL) 
				  {
				    int fd = open(output_file, O_WRONLY | O_CREAT );
				    dup2(fd, 1);
				    close(fd);
				  }
				  
				  
				  execvp(command[0], command);
				} 
				else if (r > 0) 
				{
				
				    // Shell waits for the child to finish execution
				    waitpid(r, NULL, 0);
				}
			   
			}
			if (if_pipe_exists)
			{
			    	    
				k++;
				//printf("%d",k);
				int max_args = 10; // maximum number of arguments per command
				char *cmd[k][max_args + 1]; // add an extra null terminator to each command
				int m = 0;

				for (int f = 0; f < k; f++) 
				{
				    for (int u = 0; u < max_args; u++) 
				    {
					if (command[m] == NULL) 
					{
					    cmd[f][u] = NULL;
					    break;
					} 
					else if (strcmp(command[m], "|") == 0) 
					{
					    cmd[f][u] = NULL;
					    break;
					} 
					else 
					{
					    cmd[f][u] = command[m];
					    m++;
					}
				    }
				    m++;
				}
						
				/*
				for (int f = 0; f < k; f++) 
				{
				    printf("%d \t", f);
				    for (int u = 0; cmd[f][u] != NULL; u++) 
				    {
					printf("%s \t", cmd[f][u]);
				    }
				    printf("\n");
				}*/
			    	    int fd[k][2];  // array to hold the pipes
				    pid_t pid1;
				    
				    // create all the pipes
				    for (int i = 0; i < k; i++) 
				    {
					if (pipe(fd[i]) < 0) 
					{
					    perror("pipe error");
					    return 1;
					}
				    }
				    
				    // child processes for each command created repeatedly
				    for (int i = 0; i < k; i++) 
				    {
					pid1 = fork();
					if (pid1 == -1) 
					{
					    perror("fork error");
					    // close all pipes before exiting
					    for (int j = 0; j < i; j++) 
					    {
						close(fd[j][0]);
						close(fd[j][1]);
					    }
					    return 1;
					}
					
					if (pid1 == 0) 
					{
					    // child process
					    if (i == 0) 
					    {
						// 1- first command, write to pipe
						close(fd[0][0]);
						dup2(fd[0][1], 1);
						close(fd[0][1]);
					    } 
					    else if (i == k - 1) 
					    {
						// k-1 last command, read from pipe
						close(fd[k-2][1]);
						dup2(fd[k-2][0], 0);
						close(fd[k-2][0]);
					    } 
					    else 
					    {
						// (2---k-2) middle command, read from previous pipe and write to next pipe
						close(fd[i-1][1]);
						dup2(fd[i-1][0], 0);
						close(fd[i-1][0]);
						
						close(fd[i][0]);
						dup2(fd[i][1], 1);
						close(fd[i][1]);
					    }
					    
					    execvp(cmd[i][0], cmd[i]); // here as commands are parsed above only the ones for particular iteration are executed
					    //ie ifthe original command was command[i]={"ls","|","grep",".txt","|","wc","-l"};
					    //for i=0  cmd[0][0]="ls" will be passed as first argument and cmd[0][1] as NULL
					    //for i=1  cmd[1][0]="grep" will be passed as first argument,cmd[1][1]=".txt" as second and cmd[1][2] as NULL
					    //for i=2  cmd[2][0]="wc" will be passed as first argument ,cmd[2][1]="-l" as secondand cmd[2][2] as NULL
					    // if execvp returns, there was an error
					    perror("execvp error");
					    exit(1);
					}
					else if(pid1>0)
					{
						if (i > 0) 
				    		{
							close(fd[i-1][0]);
							close(fd[i-1][1]);
						}
						waitpid(pid1, NULL, 0);
					}
				    }
            		}
			if(!if_output_exists && !if_input_exists && !if_error_exists && !if_pipe_exists)
			{
				bool check2=false;
				int status=0;
				// ask TA why this was not working if(command[i-1]=="&")
				if(strcmp(command[i-1],"&")==0)  // adding "&" functionality ,if i-1 is equal & then the parent process won't wait for completion of child 
				{
					// PartB: use of & operator to make it a background process to run it concurrently with shell
					check2=true;
					command[i-1]=NULL;  
				}
				//3. Forking a child and then using execvp to execute the command
				pid_t r=fork();
				if(r==0)
				{	
					execvp(command[0],command);
				
				}
				else if(r>0)
				{
					if(check2==false)
					{
						// 4. shell waiting for the child to finish execution
						waitpid(r,&status,0);	
					}
					// If check2 is true the parent doesn't wait for completetion of execution
				}
			}
		}
		iterator++;
		for(i=0; i<buffer_size ;i++)
		{
			buffer[i]='\0';
		}
	
	
	}
	return 0;
}

