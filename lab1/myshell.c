#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 100
#define SUB_COMMAND_SIZE 30

typedef struct Node* nodePtr;
typedef struct Node{
	char command[BUFFER_SIZE];
	nodePtr next;
}Node;

/* Function to create a new node for the linked list */
nodePtr createNode(const char* command)
{
    nodePtr newNode = (nodePtr)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Memory allocation failed\n");
        exit(1);
    }
    strncpy(newNode->command, command, BUFFER_SIZE);
    newNode->next = NULL;
    return newNode;
}

/* Function to delete the entire linked list */
void deleteList(nodePtr head) {
    nodePtr current = head;
    nodePtr next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    head = NULL;
}

/* Function to print the commands in the linked list */
void printList(nodePtr head , int size) {
    nodePtr current = head;
    while (current != NULL)
	{
		fprintf(stdout,"%d %s",size--,current->command);
        current = current->next;
    }
}

int main(void)
{
    close(2); // Close stderr
    dup(1); // Duplicate stdout to stderr
    char command[BUFFER_SIZE];  // Input command buffer

	nodePtr history_list=NULL; // Linked list for storing command history
	int background_run = 0; // regular run: 0 , in background: 1 -> &
	int i;
	int counter=0; // Command counter for history
	char* token;
	const char* delim =" \n"; // Delimiters for tokenizing command
	char arguments[SUB_COMMAND_SIZE][BUFFER_SIZE]; // Arguments array
	char* arg_ptrs[SUB_COMMAND_SIZE]; // Array of argument pointers
	int pid;


    while (1)
    {
        // Prompt for command
        fprintf(stdout, "my-shell> ");
        memset(command, 0, BUFFER_SIZE);
        fgets(command, BUFFER_SIZE, stdin);

        // Exit condition
        if (strncmp(command, "exit", 4) == 0)
        {
			deleteList(history_list);
            break;
        }

		// Add command to history
		counter++;
		nodePtr new_command = createNode(command);
		if(history_list != NULL)
			new_command->next = history_list;
		history_list = new_command;

		// Print command history
		if(strncmp(command, "history", 7) == 0)
		{
			printList(history_list, counter);
			continue;
		}

		background_run = 0; // Reset background run flag
		i = 0;
		token = strtok(command,delim); // Tokenize command

		// Store tokens in arguments array
		while(token !=NULL)
		{
			strcpy(arguments[i],token);
			i++;
			token = strtok(NULL,delim);
		}

		// Skip empty commands
		if(i==0)
		{
			continue;
		}

		// Check for background execution
		if(!strncmp("&",arguments[i-1],1))
		{
			background_run = 1;
		}

		// Fork a new process
		pid = fork();
		if(pid > 0)
		{
			int stutus = 0;
			// Wait for the child process if not running in the background
			if(!background_run)
				wait(&stutus);
		}
		else if (pid == 0)
		{
			int result;
			arg_ptrs[i] = NULL;
			if(background_run)
				i--;
            // Copy arguments to argument pointers array
			for(i-=1 ; i>=0 ; i--)
			{
				arg_ptrs[i] = arguments[i];
			}
			// Execute the command
			result = execvp(arguments[0],arg_ptrs);
			exit(result); // Exit with the result of execvp
		}
		else
			perror("error");
    }
    return 0;
}
