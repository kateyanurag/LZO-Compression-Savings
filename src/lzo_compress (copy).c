#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include "mylzo.h"

/**
* This function determines the compression savings of the File.
* The compression is done using LZO1X algorithm
* @param fp	 The file pointer of the file whose compression savings is to be determined 
* @param size  The size in bytes  
*/
int 
compression_savings_util(lzo_bytep input_block, int input_block_size) {

	lzo_bytep output_block = (lzo_bytep) malloc(input_block_size + input_block_size / 16 + 64 + 3);
	lzo_bytep wrkmem = (lzo_bytep) malloc(LZO1X_1_MEM_COMPRESS);
	lzo_uint output_block_size = 0;

	int ret_val = lzo1x_1_compress(input_block, input_block_size, output_block, &output_block_size, wrkmem);
	if (ret_val != LZO_E_OK || output_block_size > input_block_size + input_block_size / 16 + 64 + 3) {
	           printf("internal error - compression failed");
	          free(output_block);
	          free(wrkmem);
	           return -1;
       	}
       	free(output_block);
	free(wrkmem);
	return output_block_size;
}  

void
compression_savings(char *file_name) {

	struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
	stat(file_name, buf);
	if (buf->st_mode & S_IFDIR) {
		printf("\n Ignoring directory %s", file_name);
		free(buf);
		return;
	}
	free(buf);

	/* Open the file */
	FILE *fp = fopen(file_name, "rb");
	if (fp == NULL) {
		printf("\n Cannot open file %s", file_name);
		return -1;	
	}

	/* Find the size of the file*/
	fseek(fp,0,SEEK_END);
	long long int file_size=ftell(fp); 
	printf("\n SIZE OF FILE IN BYTES : %ld ", file_size);

	/* If file size = 0, then ignore */
	if (file_size == 0 || file_size < 0) {
		printf("\n File is Empty. Ignoring file %s", file_name);
		return;
	}


	fseek(fp, SEEK_SET, 0);

	/*  Find the number of processors currently available.
	    This is useful to decide on number of threads to spawn for increasing
	    parallelism and getting optimal performance */
	int cpus = sysconf( _SC_NPROCESSORS_ONLN );
	printf("\n CPUS: %d", cpus);

	/* 
	* Reference : Some Results on Compressibility Using LZO Algorithm, K. Kant, Intel Research
	* http://www.kkant.net/papers/lzo_results.pdf
	* The above paper says that the compression block size for resonable latencies and good performance should be 256 bytes. 
	* Assuming this is optimal, we will stick to this.
	*/
	long number_of_blocks = file_size / COMPRESSION_BLOCK_SIZE;
	int last_block_bytes = file_size % COMPRESSION_BLOCK_SIZE;

	printf("\n number_of_blocks = %d", number_of_blocks);
	printf("\n last_block_bytes = %d", last_block_bytes);

	if (lzo_init() != LZO_E_OK) {
        		printf("internal error - lzo_init() failed !!!\n");
        		printf("(this usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable `-DLZO_DEBUG' for diagnostics)\n");
        		exit(1);
    	}


	/*
	* We can have 'cpus' number of threads running where each thread will 
	* compress  256 bytes of block in parallel.
	*/
	int thread_number = 1;
	char buffer[257];
	lzo_bytep read_buffer = (lzo_bytep)malloc(COMPRESSION_BLOCK_SIZE);
	int bytes_read = 0;
	int compressed_size = 0;
	for (thread_number = 1; thread_number <= number_of_blocks; thread_number++) {
		bytes_read = fread((void *)read_buffer, (size_t)1, (size_t)COMPRESSION_BLOCK_SIZE, fp);
		if (bytes_read == 0) {
			printf("\n READ 0 FROM FILE");
			break;
		}
		compressed_size += compression_savings_util(read_buffer, COMPRESSION_BLOCK_SIZE);
	}

	// Read last block from the file
	bytes_read = fread(read_buffer, 1, last_block_bytes, fp);
	if (bytes_read != 0) {
		compressed_size += compression_savings_util(read_buffer, last_block_bytes); 
		printf("\n BEFORE: %d    AFTER: %d", last_block_bytes, compressed_size);	
	}
	free(read_buffer);
	fclose(fp);

	printf("\n ORIGINAL SIZE: %d    COMPRESSED SIZE: %d   COMPRESSION SAVINGS: %2.2f%%\n", file_size, compressed_size, (file_size - compressed_size)*100.0/file_size);
}

void 
get_files(char *path) {

	DIR *dir = opendir (path);
	struct dirent *ent;
	if (dir != NULL) {
		// print all the files and directories within directory 
		while ((ent = readdir (dir)) != NULL) {
			char * file_name = (char *) malloc(1 + strlen(path)+ strlen(ent->d_name) );
			strcpy(file_name, path);
			strcat(file_name, "/");
			strcat(file_name, ent->d_name);
			compression_savings(file_name);
			free(file_name);
		}
		closedir (dir); 
	}  
} 





int 
main(int argc, char *argv[]) {

	char *file_name = "/home/anurag/LZO compression/src"; 

	// Ignore the last 
	if (file_name[strlen(file_name) - 1 ] == '/') {
		printf("\n Invalid Path - Do not add / at the end");
		return;
	}

	struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
	stat(file_name, buf);
	if (buf->st_mode & S_IFDIR) {
		get_files(file_name);
	}
	else if(buf->st_mode & S_IFREG) {
		printf("\n COMPRESSING FILE");
		compression_savings(file_name);
	}
	free(buf);
	return 0;
}