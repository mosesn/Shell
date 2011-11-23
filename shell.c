#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#define MAX_LEN_COMMAND 10
#define MAX_ARGS 100
#define START 20
#define MAX_PATH_ATTR_LEN 2

struct node {
	struct node *myNode;
	char *string;
};

int changeDir(char *string)
{
	if (string != NULL) {
		if (chdir(string) == 0)
			return 0;
		else { /*-1 has been returned*/
			fprintf(stderr, "You got an error: %s!\n",
				strerror(errno));
			return 1;
		}
	} else
		return 1;
}

char *getArg(char **savePtr, char *delim)
{
	char *string = strtok_r(NULL, delim, savePtr);
	if (string != NULL)
		return string;
	else
		return NULL;
}

struct node makeNode(struct node *myNode, char *string)
{
	struct node tmp =  {myNode, string};
	return tmp;
}

struct node *push(struct node *oldHead, char *newStr)
{
	struct node *newHead = malloc(sizeof(struct node));
	if (newHead == NULL) {
		fprintf(stderr, "You got an error: %s!\n", strerror(errno));
		return NULL;
	}
	*newHead = makeNode(oldHead, newStr);
	return newHead;
}

size_t max(size_t first, size_t second)
{
	return (first > second) ? first : second;
}

struct node *remove_node(struct node *oldHead, char *newStr)
{
	if ((*oldHead).myNode == NULL)
		return NULL;
	else {
		int maxlen = max(sizeof(newStr), sizeof((*oldHead).string));
		if (strncmp(newStr, (*oldHead).string, maxlen) == 0) {
			struct node *temp = (*oldHead).myNode;
			free((*oldHead).string);
			free(oldHead);
			return temp;
		} else {
			struct node *temp =
				remove_node((*oldHead).myNode, newStr);
			if (temp == NULL)
				return NULL;
			else if (temp != (*oldHead).myNode)
				(*oldHead).myNode = temp;
			return oldHead;
		}
	}
}

char *copy(char *oldStr)
{
	size_t num = START;
	size_t len;
	while ((len = strnlen(oldStr, num)) == num)
		num *= 2;

	char *newStr = malloc(num * sizeof(char));
	if (newStr == NULL) {
		fprintf(stderr, "You got an error: %s!\n", strerror(errno));
		exit(1);
	}
	strncpy(newStr, oldStr, num);
	return newStr;
}

char *getCurDir()
{
	size_t num = START;
	char *string = malloc(sizeof(char) * num);
	while (getcwd(string, num) == NULL) {
		if (string == NULL) {
			fprintf(stderr, "You got an error: %s!\n",
				strerror(errno));
			exit(1);
		}
		num *= 2;
		free(string);
		string = malloc(sizeof(char) * num);
	}
	return string;
}

char *pop(struct node **oldHead)
{
	if ((**oldHead).myNode != NULL) {
		struct node *freeable = *oldHead;
		char *string = (**oldHead).string;
		*oldHead = ((**oldHead).myNode);
		free(freeable);
		return string;
	} else
		return NULL;
}

char *popd(struct node **oldHead)
{
	char *string = pop(oldHead);
	if (string != NULL)
		if (changeDir(string) != 0)
			return NULL;
	return string;
}

struct node *pushd(struct node **oldHead, char *newStr)
{
	if (newStr != NULL) {
		char *string = getCurDir();
		if (changeDir(newStr) != 0) {
			free(string);
			return NULL;
		}
		struct node *tmp = push(*oldHead, copy(string));
		if (tmp != NULL)
			*oldHead = tmp;
		free(string);
		return *oldHead;
	} else
		return NULL;
}

int traverseFree(struct node *cur)
{
	if (cur != NULL) {
		if ((*cur).myNode != NULL) {
			traverseFree((*cur).myNode);
			free((*cur).string);
			free(cur);
		} else {
			traverseFree((*cur).myNode);
			free(cur);
		}
	}
	return 0;
}

int traversePrint(struct node *head, char *delim)
{
	struct node *curNode = head;
	int bool = 0;
	while (curNode != NULL && (*curNode).myNode != NULL) {
		if (bool != 0)
			printf("%s", delim);
		else
			bool++;
		printf("%s", (*curNode).string);
		curNode = (*curNode).myNode;
	}
	printf("%s", "\n");
	return 0;
}

int nextWordLength(char *str, int start)
{
	size_t x = start;
	char c;
	while ((c = *(str + sizeof(char) * x)) != '\0' &&
	       (c != ' ') && (c != '\n') && (c != '\t'))
		x++;
	return x;
}

int nextWord(char *in, char *out, int start, int end)
{
	int x;
	for (x = start; x < end; x++)
		*(out + sizeof(char) * x) = *(in + sizeof(char) * x);
	*(out + sizeof(char) * (x + 1)) = '\0';
	return 0;
}

int whichCommand(char *command)
{
	if (strncmp(command, "exit", MAX_LEN_COMMAND) == 0)
		return 0;
	else if (strncmp(command, "pushd", MAX_LEN_COMMAND) == 0)
		return 1;
	else if (strncmp(command, "popd", MAX_LEN_COMMAND) == 0)
		return 2;
	else if (strncmp(command, "dirs", MAX_LEN_COMMAND) == 0)
		return 3;
	else if (strncmp(command, "cd", MAX_LEN_COMMAND) == 0)
		return 4;
	else if (strncmp(command, "path", MAX_LEN_COMMAND) == 0)
		return 5;
	else
		return 6;
}

int pathFunc(struct node **path, char *savePtr, char *delim)
{
	char *first = getArg(&savePtr, delim);
	if (first == NULL) /*just PATH*/
		traversePrint(*path, ":");
	else {
		char *second = getArg(&savePtr, delim);
		if (second == NULL)
			return 1;
		if (strncmp("+", first, MAX_PATH_ATTR_LEN) == 0) {
			char *newStr = copy(second);
			struct node *tmp = push(*path, newStr);
			if (tmp != NULL)
				*path = tmp;
		} else if (strncmp("-", first, MAX_PATH_ATTR_LEN) == 0) {
			if ((**path).myNode != NULL)
				*path = remove_node(*path, second);
			else
				return 1;
		} else
			return 1;

	}
	return 0;
}

char *filegen(struct node *path, char *curPtr)
{
	size_t presize = sizeof((*path).string) + sizeof("/") + 1;
	char *tmp = strncat((*path).string, "/", presize);
	size_t postsize =  sizeof(tmp) + sizeof(curPtr) + 1;
	char *retVal = strncat(tmp, curPtr, postsize);

	return retVal;
}

int recursePath(struct node *path, char *curPtr, char *strings[])
{
	if ((path != NULL) && ((*path).myNode != NULL)) {
		if (execv(filegen(path, curPtr), strings) == -1)
			return recursePath((*path).myNode, curPtr, strings);
		else
			return 0;
	}
	return 1;
}

int go(char *savePtr, struct node *path, char *delim, char *curPtr)
{
	size_t numArgs = 1;
	char *strings[MAX_ARGS + 1];
	strings[0] = curPtr;
	while ((strings[numArgs] = strtok_r(NULL, delim, &savePtr))
	       != NULL && numArgs < (MAX_ARGS - 1))
		numArgs++;


	pid_t pid = fork();
	if (pid == (pid_t) 0) {
		if (execv(curPtr, strings) == -1 && *curPtr != '/')
			if (recursePath(path, curPtr, strings) == 1)
				perror("ERROR");
		exit(0);
	} else {
		int ReturnCode;
		while (pid != wait(&ReturnCode))
			;
		return 0;
	}
}

int switcher(int switchee, char *savePtr, char *delim, char *curPtr,
	     struct node **head, struct node **path)
{
	switch (switchee) {
	case 0:
		return 0;
	case 1:
		if (pushd(head, getArg(&savePtr, delim)) != NULL)
			return 0;
		return 1;
	case 2:
		if (popd(head) != NULL)
			return 0;
		return 1;
	case 3:
		traversePrint(*head, "\n");
		return 0;
	case 4:
		return changeDir(getArg(&savePtr, delim));
	case 5:
		return pathFunc(path, savePtr, delim);
	case 6:
		/*execute*/
		go(savePtr, *path, delim, curPtr);
		return 0;
	default:
		return 1;
	}

}

int mainProcess(struct node **head, struct node **path)
{
	char *ptr = NULL;
	char *delim = " \t\n";
	char *curPtr;
	char *savePtr;
	ssize_t line;
	size_t bytes = 0;
	int retVal = 0;

	/*getline should let the user get lines safely*/
	line = getline(&ptr, &bytes, stdin);

	if (line == -1) {
		fprintf(stderr, "You got an error!\n");
		exit(1);
	}

	curPtr = strtok_r(ptr, delim, &savePtr);
	if (curPtr == NULL){
		return 1;
	}
	retVal = whichCommand(curPtr);
	if (switcher(retVal, savePtr,
		     delim, curPtr, head, path) == 1)
		printf("You are doing something you should not be doing.\n");

	free(ptr);

	return retVal;
}

int main(int argc, char **argv)
{
	int stop = 1;
	struct node *head = malloc(sizeof(struct node));
	if (head == NULL) {
		fprintf(stderr, "You got an error: %s!\n", strerror(errno));
		exit(1);
	}
	*head = makeNode(NULL, "");

	struct node *path = malloc(sizeof(struct node));
	if (path == NULL) {
		fprintf(stderr, "You got an error: %s!\n", strerror(errno));
		exit(1);
	}
	*path = makeNode(NULL, "");

	while (stop)
		stop = mainProcess(&head, &path);

	traverseFree(path);
	traverseFree(head);

	return EXIT_SUCCESS;
}
