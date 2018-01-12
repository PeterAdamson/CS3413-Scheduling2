//Author Peter Adamson
//CS3413
//Assignment 1 Question 2
//command line format for usage of this program: ./a.out [int] < [input.txt] 
//where [int] is the desired number of processors (1 or more) and [input.txt] is the input file you wish to hand to stdin

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLEN 100

//define the node structure for the queue
typedef struct qNode Node;
struct qNode
{
	char *user;
	char *process;
	int arrival;
	int duration;
	Node *next;	
};

//defines a user summary structure for use upon job completion
typedef struct userSummary Summary;
struct userSummary
{
	char *user;
	int lastCompleted;
};

//defines a cpu structure
typedef struct cpuStruct CPU;
struct cpuStruct
{
	int cpuNum;
	int availableAt;
	int startAt;
	int busy;  //0 if free, 1 if busy
};

//function declarations
void initialize();
void enqueue(Node *newJob);
void dequeue();
void schedule(int cpus);
void printResult();
int length();
int front();
Node *readJob();
Node* getFirstNode();

//global pointers to the start(head) and end(tail) of the queue
Node *head;
Node *tail;

int main(int argc, char **argv)
{
	//check the number of cpus given
	int cpus;
	if(argc == 2)
	{
		cpus = atoi(argv[1]);
	}
	else
	{
		printf("invalid number of arguments\n");
		return;
	}

	//chew up and ignore the header
	char header[MAXLEN];
	fgets(header, sizeof(header), stdin);

	//initialize queue pointers
	initialize();

	//load the jobs into the queue
	Node *newJob;
	newJob = readJob();
	while(newJob != NULL)
	{
		enqueue(newJob);
		newJob = readJob();
	}
	
	//schedule the jobs
	schedule(cpus);

	//produce a report
	printResult();
}

//sets the head and tail pointers to null and indicates to the user that the pointers are ready to use
void initialize()
{
	head = tail = NULL;
	printf("queue initialized\n");
}

//loads a job into the end of the queue
void enqueue(Node *newJob)
{
	//set up the job to be added
	char user[MAXLEN];
	char process[MAXLEN];
	int arrival;
	int duration;
	Node *temp = NULL;
	temp = (Node*)malloc(sizeof(Node));
	temp->user = newJob->user;
	temp->process = newJob->process;
	temp->arrival = newJob->arrival;
	temp->duration = newJob->duration;
	temp->next = NULL;

	if(tail == NULL)	//the queue must be empty, so set both head and tail to temp
	{
		head = temp;
		tail = temp;
	}
	else			//the queue is not empty, so add the job to the end of the queue
	{
		tail->next = temp;
		tail = temp;	
	}
}

//removes a job from the front of the queue
void dequeue()
{
	//set up the job to be removed
	Node *temp;
	temp = head;
	if(head == NULL)	//the queue is empty
	{ 
		tail = NULL;
	}
	else			//queue is not empty, so remove the job at the start of the queue
	{
		head = head->next;
		free(temp);
	}
}

//returns the first job in the queue
Node* getFirstNode()
{
	//set up the job to be returned
	Node *firstNode;
	firstNode = head;
	if(head == NULL)	//the queue is empty
	{
		printf("queue is empty\n");
	}
	else			//the queue is not empty, so return the first job in the queue
	{
		return firstNode;
	}
}

//return the length of the queue
int length()
{
	int length = 0;
	Node *temp = head;

	if(tail == NULL)	//the queue must be empty
	{
		return length;
	}
	else			//the queue is not empty, so increment length and continue until the end is reached
	{
		while(temp)
		{
			length++;
			temp = temp->next;
		}
	}
	return length;
}

//reads in a job from standard input
Node *readJob()
{
	//set up the job to be added
	char user[MAXLEN];
	char process[MAXLEN];
	char a[MAXLEN];
	char b[MAXLEN];
	int arrival;
	int duration;
	Node *newJob = NULL;

	/*check that we are not at the end of the file and that the format is correct.
	if so, set up and return the job*/
	if(!feof(stdin) && (4 == scanf("%s %s %d %d", user, process, &arrival, &duration)))
	{
	newJob = (Node*)malloc(sizeof(Node));
	newJob->user = malloc(strlen(user)+1);
	strcpy(newJob->user, user);
	newJob->process = malloc(strlen(process)+1);
	strcpy(newJob->process, process);
	newJob->arrival = arrival;
	newJob->duration = duration;
	newJob->next = NULL;
	}

	return newJob;
}

//performs the scheduling of the arrived jobs
void schedule(int cpus)
{
	//set up a log file for auditing or reporting purposes (e.g. to produce a summary)
	FILE *file = fopen("TaskLog.txt","w");
	int time = 0;
	int maxUsers = length();
	int i;
	int j;
	int found = 0;

	//an array of Summary structs
	Summary *summaries[maxUsers];

	//an array of cpu structs
	CPU *cpuList[cpus];

	//memory allocation and array initialization for summaries
	for(i = 0; i < maxUsers; i++)
	{
		summaries[i] = malloc(sizeof(Summary));
		summaries[i] = NULL;
	}

	//memory allocation and array initialization for summaries
	for(i = 0; i < cpus; i++)
	{
		cpuList[i] = malloc(sizeof(CPU));
		cpuList[i]->busy = 0;
		cpuList[i]->availableAt = 0;
		cpuList[i]->cpuNum = i+1;
	}

	//reset i to 0 for next section
	i = 0;

	//scheduling loop as long as queue has jobs remaining	
	while(head)
	{
		//check if any cpu's have been freed up since last interval
		for(j = 0; j < cpus; j++)
		{
			if(cpuList[j]->availableAt <= time)
			{
				cpuList[j]->busy = 0;
			}
		}

		int runItAgain = 0;
		while(runItAgain == 0)
		{
	
			//check if there are any free cpus
			int allbusy = 1;
			for(j = 0; j < cpus; j++)
			{
				if(cpuList[j]->busy == 0)
				{
					allbusy = 0;
				}
			}
		
			//if all cpu's are busy, we can't do anything so increase time
			if(allbusy == 1)	//all cpus are busy
			{
				time++;
				runItAgain = 1;
			}
			else			//at least one cpu is free
			{
				Node *temp = getFirstNode();
				if(temp == NULL)	//the queue has emptied
				{
					runItAgain = 1;
					continue;
				}
				else if(temp->arrival > time) //the job has not arrived yet
				{
					runItAgain = 1;
					time++;
				}
				else			//we can schedule the job
				{
					//find the first available cpu
					int cpuCount = 0;
					while(cpuList[cpuCount]->busy == 1)
					{
						cpuCount++;
					}

					cpuList[cpuCount]->availableAt = time + temp->duration;
					cpuList[cpuCount]->startAt = time;
					cpuList[cpuCount]->busy = 1;
					fprintf(file,"%d %s %s %d\n",cpuList[cpuCount]->startAt,temp->process,temp->user,cpuList[cpuCount]->availableAt);
					Summary *tempUser = NULL;
					tempUser = (Summary*)malloc(sizeof(Summary));
					tempUser->user = temp->user;
					tempUser->lastCompleted = cpuList[cpuCount]->availableAt;
					summaries[i] = tempUser;
					i++;

					if(temp->next)	//there is at least one more job still in the queueu
					{
						if(temp->arrival != temp->next->arrival)	//the next job does not arrive simultaneously
						{
							runItAgain = 1;
						}
					}
					else		//there are no jobs left in the queueu
					{
						runItAgain = 1;
					}

					dequeue();
				}
			}
		}
	}

	//check for duplicates 
	for(i = 0; i < maxUsers - 1; i++)
	{
		for(j = i +1; j < maxUsers; j++)
		{
			if(summaries[j] != NULL) //we have data present
			{
				if(strcmp(summaries[i]->user,summaries[j]->user) == 0)	//we have a duplicate
				{
					if(summaries[i]->lastCompleted > summaries[j]->lastCompleted)  //summaries[i] is the more recently completed job
					{
						summaries[j] = NULL;
					}
					else								//summaries[j] is the more recently completed job
					{
						summaries[i] = summaries[j];
						summaries[j] = NULL;
						}
				}
			}
		}
	}
	
	//sort cpuList
	for(i = 0; i < cpus -1; i++)
	{
		for(j=i+1; j< cpus; j++)
		{
			if(cpuList[i]->availableAt > cpuList[j]->availableAt) //swap
			{
				CPU* temp = cpuList[i];
				cpuList[i] = cpuList[j];
				cpuList[j] = temp;
			}
		}
	}

	//indicate that the job scheduling has completed
	for(i = 0; i < cpus; i++)
	{
		fprintf(file,"%d CPU%dIDLE CPU%dIDLE 0\n",cpuList[i]->availableAt,cpuList[i]->cpuNum,cpuList[i]->cpuNum);
	}

	//print out summary to audit/log file
	for(i = 0; i < maxUsers; i++)
	{
		if(summaries[i] != NULL)	//we have data
		{
			fprintf(file,"%s %d\n",summaries[i]->user, summaries[i]->lastCompleted);
		}
	}

	//free summaries
	for(i = 0; i < maxUsers; i++)
	{
		summaries[i] = NULL;
		free(summaries[i]);
	}

	//free cpus
	for(i = 0; i < cpus; i++)
	{
		cpuList[i] = NULL;
		free(cpuList[i]);
	}

	//close the audit/log file
	fclose(file);
}

//produces a formatted report for the scheduler
void printResult()
{
	//open and read the audit/log
	FILE *file = fopen("TaskLog.txt","r");
	int time;
	char job[MAXLEN];
	char user[MAXLEN];
	int completed;

	//set up a header
	printf("Time\tJob\n");

	//read and print loop
	while(1)
	{
		if(!feof(file) && (4 == fscanf(file, "%d %s %s %d", &time, job, user, &completed))) //format is correct
		{
			printf("%d\t%s\n",time, job);
		}
		else										   //we are done with this section of the report
		{
			break;
		}
	}
	
	//set up the next header
	printf("\n");
	printf("Summary\n");

	//read and print loop
	while(1)
	{
		if(!feof(file) && (2 == fscanf(file, "%s %d", user, &time)))	//format is correct
		{
			printf("%s\t%d\n",user,time);
		}
		else								//we are done
		{
			break;
		}
	}	

	//close the audit/log file
	fclose(file);
}
