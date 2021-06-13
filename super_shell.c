/*
 * Super_Shell

 *
 *  Created on: Mar 26, 2021
 *      Authors: Ori Malka & Alexander Martinov
 */

#define _CRT_SECURE_NO_WARNINGS
#define N 256
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

 //a struct for sorting letter frequency.
typedef struct intandchar {
	char c;
	int n;
} intandchar;

//a struct to manage command locking.
typedef struct Lock {
	char Name[N];
	int LockTime;
	struct timeval TimeWhenLocked;
} Lock;

//the assignment functions
int scalls(char* path, char args[][N], int n);
int encryptFile(char* filename, int n, char* newfilename);
int decryptFile(char* filename, int n, char* newfilename);
int letterFreq(char* filename);
int uppercaseByIndex(char* filename, char* newfilename, int I);
int lowercaseByIndex(char* filename, char* newfilename, int I);
int randomFile(int x, char* newfilename);
int compressFile(char* filename, char* newfilename);

int lockCmdForTime(char* filename, int t);
//help functions to implement lockCmdForTime
int isLocked(char* filename); //checks if a command is locked, returns 1 if locked else 0.
void emptyLocks(); //removes unlocked commands from the global help array.

Lock LockedCommands[N]; //global array to store and control locked commands.

int main() {
	int i;
	srand(time(NULL)); //for random function to implement randomFile function.

	//initialize LockedCommands array

	for (i = 0; i < N; i++) {
		strcpy(LockedCommands[i].Name, "");
	}

	//Shell loop to receive new commands.
	do {

		//to get the commands and the paramaters.
		char s[2] = " ", * temp, args[6][N], path[2 * N] = "/bin/";
		char buffer[N];
		int counter = 0, flag = 0, fd_out, fd_screen, i, newoutflag = 0;

		printf("SuperShell>");
		fgets(buffer, N, stdin);

		temp = strtok(buffer, s); //we divide the buffer into tokens to be able to control the input.
		while ((temp != NULL) && (counter < 6)) {

			strcpy(args[counter], temp);
			counter++;
			temp = strtok(NULL, s);

		}

		//create the path
		strcat(path, args[0]);

		//to remove the "Enter"
		args[counter - 1][strlen(args[counter - 1]) - 1] = '\0';

		//check the case when ">" received to change the output.
		for (i = 0; i < counter; i++) {
			if (!(strcmp(args[i], ">"))) {
				if ((fd_out = open(args[i + 1], O_WRONLY | O_CREAT, 0764))
					== -1) {
					perror("open to");
					return (-1);
				}
				fd_screen = dup(1); //saves the default screen fd
				dup2(fd_out, 1); //replace the default output with the opened file.
				newoutflag = 1; //indicate that the output has been changed.
				counter -= 2; //for ">" and "new stdout"
			}
		}

		//flag=0 means that it will be a system call (regular) or a not supported call.

		if (isLocked(args[0])) {
			printf("%s is Locked!!\n", args[0]);
			flag = 1; //in this case we also wont to avoid system call.
		}
		else if (!(strcmp("encryptFile", args[0]))) {
			int n = atoi(args[2]);
			if (n > 100 && n < 1) {
				printf("The given number is out of range");
				return -1;
			}

			encryptFile(args[1], n, args[3]);
			flag = 1;

		}
		else if (!(strcmp("decryptFile", args[0]))) {
			int n = atoi(args[2]);
			if (n > 100 && n < 1) {
				printf("The given number is out of range");
				return -1;
			}
			decryptFile(args[1], n, args[3]);
			flag = 1;

		}
		else if (!(strcmp("lockCmdForTime", args[0]))) {
			int t = atoi(args[2]);
			lockCmdForTime(args[1], t);
			flag = 1;

		}
		else if (!(strcmp("letterFreq", args[0]))) {
			letterFreq(args[1]);
			flag = 1;

		}
		else if (!(strcmp("uppercaseByIndex", args[0]))) {
			int n = atoi(args[3]);
			uppercaseByIndex(args[1], args[2], n);
			flag = 1;

		}
		else if (!(strcmp("lowercaseByIndex", args[0]))) {
			int n = atoi(args[3]);
			lowercaseByIndex(args[1], args[2], n);
			flag = 1;

		}
		else if (!(strcmp("randomFile", args[0]))) {
			int x = atoi(args[1]);
			randomFile(x, args[2]);
			flag = 1;

		}
		else if (!(strcmp("compressFile", args[0]))) {
			compressFile(args[1], args[2]);
			flag = 1;
		}
		else if (!(strcmp("byebye", args[0]))) {
			flag = 1;
			exit(0);
		}

		if (!flag) {
			if (scalls(path, args, counter - 1) == -1) {
				printf("Error! too many/few arguments");
			}
		}

		if (newoutflag == 1) {
			dup2(fd_screen, 1); //to make sure that stdout default is the screen.
			close(fd_out);
		}

		emptyLocks();

	} while (1);

	return 0;

}

int scalls(char* path, char args[][N], int n) {

	int pid;
	if ((pid = fork()) == -1) {
		perror("fork() failed.");
		exit(EXIT_FAILURE);
	}

	if (pid == 0) { //this is to make sure its the son pid.

		switch (n) {
			//case for different amount of arguments
		case 0:
			execlp(args[0], args[0], NULL);
			printf("Not Supported\n");
			exit(0);
			break;

		case 1:
			execl(path, args[0], args[1], NULL);
			printf("Not Supported\n");
			exit(0);
			break;

		case 2:
			execl(path, args[0], args[1], args[2], NULL);
			printf("Not Supported\n");
			exit(0);
			break;

		case 3:
			execl(path, args[0], args[1], args[2], args[3], NULL);
			printf("Not Supported\n");
			exit(0);
			break;

		default:
			exit(0);

			break;

		}

	}

	wait();
	return 0;

}

int encryptFile(char* filename, int n, char* newfilename) {
	int fd_from, fd_to, r_bytes, w_bytes, i;
	char buff[N];

	if ((fd_from = open(filename, O_RDONLY)) == -1) //open source file to read.
	{
		perror("open from");
		return (-1);
	}

	if ((fd_to = open(newfilename, O_WRONLY | O_CREAT, 0764)) == -1) { //open destination file for write.
		perror("open to");
		return (-1);
	}

	while ((r_bytes = read(fd_from, buff, N)) != 0) { //read N bytes to the buffer from the source file.

		if (r_bytes == -1) {
			perror("read");
			return (-1);
		}

		for (i = 0; i < r_bytes - 1; i++) { //change the buffer chars as asked.
			int temp = buff[i] + n;
			buff[i] = temp % N;

		}

		if ((w_bytes = write(fd_to, buff, r_bytes)) == -1) { //write r_y bytes from the buffer to the destination file.
			perror("write");
			return (-1);
		}

		if (w_bytes != r_bytes) { //in case there was an Error writing the same text that we read, we abort.
			perror("bad write");
			return (-1);
		}

	}

	close(fd_from);
	close(fd_to);

	return 0;

}

int decryptFile(char* filename, int n, char* newfilename) {
	int fd_from, fd_to, r_bytes, w_bytes, i;
	char buff[N];
	//same comments as in encryptFile (expect the algorithm to change the buffer chars)

	if ((fd_from = open(filename, O_RDONLY)) == -1) {
		perror("open from");
		return (-1);
	}

	if ((fd_to = open(newfilename, O_WRONLY | O_CREAT, 0764)) == -1) {
		perror("open to");
		return (-1);
	}

	while ((r_bytes = read(fd_from, buff, N)) != 0) {

		if (r_bytes == -1) {
			perror("read");
			return (-1);
		}

		for (i = 0; i < r_bytes - 1; i++) {
			int temp = buff[i] - n;
			buff[i] = temp % N;

		}

		if ((w_bytes = write(fd_to, buff, r_bytes)) == -1) {
			perror("write");
			return (-1);
		}

		if (w_bytes != r_bytes) {
			perror("bad write");
			return (-1);
		}

	}

	close(fd_from);
	close(fd_to);

	return 0;

}

int letterFreq(char* filename) {
	int fd_from, r_bytes;
	char buff[N];
	intandchar a[26]; //count struct array that holds a letter with an int counter.
	int i, j;
	double totalcounter = 0.0; //to save the total amount of actual alphabet letters in the file.

	for (i = 0; i < 26; i++) { //initialize the struct array.
		a[i].n = 0;
		a[i].c = 'a' + i;
	}

	if ((fd_from = open(filename, O_RDONLY)) == -1) //open the source file for reading.
	{
		perror("open from");
		return (-1);
	}

	while ((r_bytes = read(fd_from, buff, N)) != 0) { //read N bytes (or less) from the source file to the buffer.

		if (r_bytes == -1) {
			perror("read");
			return (-1);
		}

		for (i = 0; i < r_bytes; i++) { //count appearance of letters in the source file.

			if ((buff[i] >= 'a') && (buff[i] <= 'z')) {
				a[buff[i] - 'a'].n++;
				totalcounter++;
			}

			if ((buff[i] >= 'A') && (buff[i] <= 'Z')) {
				a[buff[i] - 'A'].n++;
				totalcounter++;
			}

		}

	}

	//find the top 3

	intandchar temp;
	for (i = 1; i < 26; i++) {
		temp = a[i];
		j = i - 1;

		//Insert sort by key of the counter, the swap is for each struct.
		while (j >= 0 && a[j].n > temp.n) {
			a[j + 1] = a[j];
			j = j - 1;
		}
		a[j + 1] = temp;
	}

	double sum; //calculate the sums and prints in format as shown in the example.
	sum = (((a[25].n) / (totalcounter)) * 100);
	printf("%c-%.1lf%%\n", a[25].c, sum);
	sum = (((a[24].n) / (totalcounter)) * 100);
	printf("%c-%.1lf%%\n", a[24].c, sum);
	sum = (((a[23].n) / (totalcounter)) * 100);
	printf("%c-%.1lf%%\n", a[23].c, sum);
	if (a[25].c == 'e') {
		if (a[24].c == 'a') {
			if ((a[23].c == 'o') || (a[23].c == 'i'))
				printf("Good Letter Frequency\n");

		}
	}

	close(fd_from);
	return 0;
}

int uppercaseByIndex(char* filename, char* newfilename, int I) {
	int fd_from, fd_to, r_bytes, w_bytes, i;
	char buff[N], buffout[N] = { 0 }, * temp, s[2] = " ";
	/*
	 1.read to a buffer and split it into tokens.
	 2.check if length of the token is greater than I.
	 3.check that the token[I] is a lower case letter.
	 4.reduce 32 from the lower case to change it to upper case letter.
	 */

	if ((fd_from = open(filename, O_RDONLY)) == -1) //we opened the from file for reading.
	{
		perror("open from");
		return (-1);
	}

	if ((fd_to = open(newfilename, O_WRONLY | O_CREAT, 0764)) == -1) {
		perror("open to");
		return (-1);
	}

	while ((r_bytes = read(fd_from, buff, N)) != 0) {

		if (r_bytes == -1) {
			perror("read");
			return (-1);
		}

		temp = strtok(buff, s);

		while (temp != NULL) {
			char t[N]; //to hold a token
			strcpy(t, temp);
			if ((strlen(t)) > I) {
				if ((t[I] >= 'a') && (t[I] <= 'z')) {
					t[I] = t[I] - 32;
				}
			}
			strcat(t, s); //to add space
			strcat(buffout, t);
			temp = strtok(NULL, s);

		}

		if ((w_bytes = write(fd_to, buffout, r_bytes)) == -1) {
			perror("write");
			return (-1);
		}

		if (w_bytes != r_bytes) { //in case there was an Error writing the same text that we read, we abort.
			perror("bad write");
			return (-1);
		}

	}

	close(fd_from);
	close(fd_to);

	return 0;
}

int lowercaseByIndex(char* filename, char* newfilename, int I) {
	int fd_from, fd_to, r_bytes, w_bytes;
	char buff[N], buffout[N] = { 0 }, * temp, s[2] = " ";

	//same as uppercaseByIndex except that we check if it is upper case and change it to lower case letter.

	if ((fd_from = open(filename, O_RDONLY)) == -1) //we opened the from file for reading.
	{
		perror("open from");
		return (-1);
	}

	if ((fd_to = open(newfilename, O_WRONLY | O_CREAT, 0764)) == -1) {
		perror("open to");
		return (-1);
	}

	while ((r_bytes = read(fd_from, buff, N)) != 0) {

		if (r_bytes == -1) {
			perror("read");
			return (-1);
		}

		temp = strtok(buff, s);

		while (temp != NULL) {
			char t[N]; //to hold a token
			strcpy(t, temp);
			if ((strlen(t)) > I) {
				if ((t[I] >= 'A') && (t[I] <= 'Z')) {
					t[I] = t[I] + 32;
				}
			}
			strcat(t, s); //to add space
			strcat(buffout, t);
			temp = strtok(NULL, s);

		}

		if ((w_bytes = write(fd_to, buffout, r_bytes)) == -1) {
			perror("write");
			return (-1);
		}

		if (w_bytes != r_bytes) { //in case there was an Error writing the same text that we read, we abort.
			perror("bad write");
			return (-1);
		}

	}

	close(fd_from);
	close(fd_to);

	return 0;
}

int randomFile(int x, char* newfilename) {
	int fd_to, w_bytes = 1, i;
	char c[2] = { 0 };
	if ((fd_to = open(newfilename, O_WRONLY | O_CREAT, 0764)) == -1) {
		perror("open to");
		return (-1);
	}
	/*
	 1.open the new file.
	 2.generate a random letter.
	 3.write the letter into the new file.
	 4.does it x times.
	 */

	for (i = 0; i < x; i++) {
		c[0] = rand() % 26;
		if (rand() % 2)
			c[0] += 'a';
		else
			c[0] += 'A';

		if (!(w_bytes = write(fd_to, c, 1))) {
			perror("write");
			return (-1);
		}
	}

	close(fd_to);
	return 0;

}

int compressFile(char* filename, char* newfilename) {
	int fd_from, fd_to, r_bytes, w_bytes, i;
	char buff[N];
	/*
	 1.open the files.
	 2.read from the source file.
	 3.go over the buffer with a loop on each letter.
	 4.write letter buffer[i] if !=buffer[i+1].
	 5.count letter buffer[i] if ==buffer[i+1].
	 6.if counter greater than 4 write letter buffer[i] and counter (as a char);
	 7.else write letter buffer[i] counter<5 times to the dest file.
	 */

	if ((fd_from = open(filename, O_RDONLY)) == -1) //we opened the from file for reading.
	{
		perror("open from");
		return (-1);
	}

	if ((fd_to = open(newfilename, O_WRONLY | O_CREAT, 0764)) == -1) {
		perror("open to");
		return (-1);
	}

	while ((r_bytes = read(fd_from, buff, N)) != 0) {

		if (r_bytes == -1) {
			perror("read");
			return (-1);
		}

		for (i = 0; i < r_bytes - 1; i++) {
			int counter = 1;
			if (buff[i] != buff[i + 1]) {
				char temp[2] = { 0 };
				temp[0] = buff[i];
				if ((w_bytes = write(fd_to, temp, 1)) == -1) {
					perror("write");
					return (-1);
				}
			}
			else {
				while ((buff[i] == buff[i + 1]) && (i < r_bytes)) {
					counter++;
					i++;
				}
				if (counter > 4) {
					char temp[3];
					temp[0] = buff[i];
					temp[1] = counter + '0';
					if ((w_bytes = write(fd_to, temp, 2)) == -1) {
						perror("write");
						return (-1);
					}

				}

				else {
					char temp[2] = { 0 };
					int j;
					temp[0] = buff[i];
					for (j = 0; j < counter; j++) {
						if ((w_bytes = write(fd_to, temp, 1)) == -1) {
							perror("write");
							return (-1);
						}
					}

				}

			}

		}

	}

	close(fd_from);
	close(fd_to);

	return 0;

}
int lockCmdForTime(char* filename, int t) {
	int i;
	/*
	 'filename' command need to be locked t seconds.
	 1.check if the command name is already in the locked array.
	 2.if so: check if its already locked
	 2.1. locked: add t seconds to the locking time.
	 2.2. unlocked: lock (updating everything except the name).
	 3.not in the array: find an empty spot in the locked array and lock the command.
	 */

	for (i = 0; i < N; i++) {
		if (!strcmp(filename, LockedCommands[i].Name)) {
			if (isLocked(filename)) {
				LockedCommands[i].LockTime += t;
			}
			else {
				LockedCommands[i].LockTime = t;
				gettimeofday(&(LockedCommands[i].TimeWhenLocked), NULL);
			}
			return 0;
		}
	}

	for (i = 0; i < N; i++) {
		if (!strcmp("", LockedCommands[i].Name)) {
			strcpy(LockedCommands[i].Name, filename);
			LockedCommands[i].LockTime = t;
			gettimeofday(&(LockedCommands[i].TimeWhenLocked), NULL);
			return 0;
		}
	}

	return -1;
}

int isLocked(char* filename) {
	double elapsedtime;
	struct timeval timenow;
	int i;
	/*
	 1. save the time now.
	 2. search the command in the locked commands array.
	 2.1 if found: calc elapsed time and compare with lock time. (greater its not locked, else locked).
	 2.2 if not found: its not locked.
	 */
	gettimeofday(&(timenow), NULL);

	for (i = 0; i < N; i++) {
		if (!strcmp(filename, LockedCommands[i].Name)) {
			elapsedtime = (timenow.tv_sec
				- LockedCommands[i].TimeWhenLocked.tv_sec);
			if (elapsedtime >= LockedCommands[i].LockTime)
				return 0;
			return 1;
		}
	}
	return 0;
}

void emptyLocks() { //run all over the locked array and remove unlocked commands from the array.
	int i;
	for (i = 0; i < N; i++) {
		if (strcmp("", LockedCommands[i].Name)) {
			if (!isLocked(LockedCommands[i].Name)) {
				strcpy(LockedCommands[i].Name, "");
			}
		}
	}
}
