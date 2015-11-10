#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>

void get_files(char *path) {

	printf("\n DIRECTORY PATH : %s", path);
	sleep(1);
	DIR *dir = opendir (path);
	struct dirent *ent;
	if (dir != NULL) {
		// print all the files and directories within directory 
		while ((ent = readdir (dir)) != NULL) {
			struct stat buf;
			stat(ent->d_name, &buf);
			if (buf.st_mode & S_IFDIR) {
				printf("\n DIRECTORY NAME: %s", ent->d_name);
				//get_files(ent->d_name);
			}
			else if (buf.st_mode & S_IFREG) {
				printf ("\n FILE NAME: %s", ent->d_name);
			}
		}
		closedir (dir); 
	}  
}




void main() {
	char *file_name = "/home/anurag/a/"; 

	int enabling_thread = 0;


	struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
	stat(file_name, buf);
	if (buf->st_mode & S_IFDIR) {
		DIR *dir = opendir (file_name);
		struct dirent *ent;
		if (dir != NULL) {
			// print all the files and directories within directory 
			while ((ent = readdir (dir)) != NULL) {
				lstat(ent->d_name, buf);
				if (S_ISDIR(buf->st_mode)) {
					printf("\n DIRECTORY NAME: %s", ent->d_name);
				}
				else if (buf->st_mode & S_IFREG) {
					printf ("\n FILE NAME: %s", ent->d_name);
				}
			}
			closedir (dir); 
		}  
	}
	else if(buf->st_mode & S_IFREG) {
		printf("\n REGULAR !!!!!!!!!!!!!!!!!!!");
	}
	free(buf);
}