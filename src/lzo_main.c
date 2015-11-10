#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include "mylzo.h"

// File pointer for writing to CSV file
FILE *csv_fp;
pthread_mutex_t csv_lock = PTHREAD_MUTEX_INITIALIZER;

/**
* This method extracts all the files from the given path and calls the compression_savings()
* method defined in lzo_compress.c.
* If enabled_threading is 1, then the compression factor is calculated using efficient multithreading.
*/
void 
get_files(char *path, int enabled_threading) {

	if (path == NULL || enabled_threading < 0) {
		return;
	}

	/*  Find the number of processors currently available.
	This is useful to decide on number of threads to spawn for increasing
	parallelism and getting optimal performance */
	int cpus = sysconf( _SC_NPROCESSORS_ONLN );

	// Threads and mutex variables
	pthread_t *pthreads[cpus] ;
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	void **value_ptr = NULL;
	int threads_running = 0;	

	DIR *dir = opendir (path);
	struct dirent *ent;

	if (dir != NULL) {
		// Extract all the files and directories within directory 
		while (1) {
			if (enabled_threading) {
				pthread_mutex_lock(&lock);
			}	
			ent = readdir(dir);
			if (ent == NULL) {
				break;
			}

			// make each file name absolute
			char * file_name = (char *) malloc(1 + strlen(path)+ strlen(ent->d_name));
			strcpy(file_name, path);
			strcat(file_name, "/");
			strcat(file_name, ent->d_name);

			if (enabled_threading) {

				// Limit the threads to be spawned to the number of virtual CPU's
				if (threads_running < cpus) {
					pthreads[threads_running] = (pthread_t *)malloc(sizeof(pthread_t));
					threads_running++;
					pthread_mutex_unlock(&lock);

					// Each time a new thread gets forked and will run the compression_savings() method with file_name as parameter
					pthread_create(pthreads[threads_running - 1], 0, (void *)&compression_savings, file_name);

				} else {
					pthread_mutex_unlock(&lock);
				}
			} else { 
				// Only the main thread calls this method.
				compression_savings(file_name);
			}

			if (enabled_threading) {

				// Join the spawned threads
				while(threads_running != 1) {
					pthread_join(*pthreads[threads_running - 1], value_ptr);
					free(pthreads[threads_running - 1]);
					threads_running--;
				}
			}
			free(file_name);
		}
		// Close the directory.
		closedir (dir); 
	}  
} 

int 
main(int argc, char *argv[]) {

	// File nae or Path name
	char *file_name = ""; 

	// By default multi threading is disabled 
	int enabled_threading = 0;

	/* Parsing Command Line arguments */

	// Default command
	if (argc == 1) {
		// Compress files in current directory 
		file_name = ".";
		// Disable threading
		enabled_threading = 0;
	} else if (argc == 2) {
		if (strcmp(argv[argc - 1], "-f") == 0) {
			enabled_threading = 1; 
			file_name = ".";
		} else if (strcmp(argv[argc - 1], "--help") == 0) {
			printf("\n  How to execute the command");
			printf("\n * path is the path of file or directory. \n * -f is to enable threading for fast processing");
			printf("\n * Both the parameters are optional.\n * If path not provided, all the files in the current directory will be considered.");
			printf("\n * By default threading is disabled\n");
			return -1;
		} else {
			file_name = argv[argc - 1];
		}
	} else if (argc == 3) {
		if (strcmp(argv[argc - 1], "-f") == 0 ) {
			enabled_threading = 1;
			file_name = argv[argc - 2];
		} else if (strcmp(argv[argc - 2], "-f") == 0) {
			enabled_threading = 1;
			file_name = argv[argc - 1];
		}else {
			printf("\n Invalid command !\n Usage : ./lzo_compress [file path or dir path] [-f]");
			printf("\n * For help type ./lzo_compress --help\n\n");
			return -1;
		}

	}else {
		printf("\n Invalid command !\n Usage : ./lzo_compress [file path or dir path] [-f]");
		printf("\n * For help type ./lzo_compress --help\n\n");
		return -1;
	}

	// Dont allow user to find compression factor of root directory "/"
	if (strcmp(file_name, "/") == 0) {
		printf("\n Cannot compress root '/'  directory");;
		return -1; 
	}

	// Ask user to input the path which ends with 
	if (file_name[strlen(file_name) - 1 ] == '/') {
		printf("\n Invalid Path - Do not add / at the end");
		return -1; 
	}


	/* Open the output file in write binary mode */
	csv_fp = fopen("./compression_results.csv", "wb");
	if (csv_fp == NULL) {
		printf("\n Cannot open file for writing compression results");
		return -1;	
	}
	// Write column names in csv files
	fprintf(csv_fp, "File Name (path) \t Initial size(bytes) \t Amount compressible (percent) \t After compression size(bytes)\n");


	/* Check if the input path is a file_name or a directory path */
	struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
	stat(file_name, buf);
	if (buf->st_mode & S_IFDIR) {

		// If directory path, then find compression factor of all files in this path. 
		//The directories will be ignored.
		get_files(file_name, enabled_threading);
	}
	else if(buf->st_mode & S_IFREG) {
		compression_savings(file_name);
	} else {
		// If invalid path, the display appropriate message
		printf("\n File Doesnt exists!\n");
		return -1;
		
	}
	free(buf);
	fclose(csv_fp);

	printf("\n\n Compression Results stored in current_directory/compression_results.csv \n");
	return 0;
}
