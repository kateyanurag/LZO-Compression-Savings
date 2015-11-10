#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include "mylzo.h"

/**
* This function determines the compression savings of the File.
* The compression is done using LZO1X algorithm
* @param fp	 The file pointer of the file whose compression savings is to be determined 
* @param size  The size in bytes  
*/
int 
compression_savings_util(lzo_bytep input_block, int input_block_size) {

	/* 
	* Allocate memory to store compressd data.
	* LZO will expand incompressible data by a little amount. From the library documentation, 
	* I found this formula for a worst-case expansion calculation : output_block_size = input_block_size + (input_block_size / 16) + 64 + 3
	*  [This is about 106% for a large block size.]
	*/
	lzo_bytep output_block = (lzo_bytep) malloc(input_block_size + input_block_size / 16 + 64 + 3);
	lzo_bytep wrkmem = (lzo_bytep) malloc(LZO1X_1_MEM_COMPRESS);
	lzo_uint output_block_size = 0;

	/*
	* Algorithm: LZO1X
  	* Compression level: LZO1X-1
	*/
	int ret_val = lzo1x_1_compress(input_block, input_block_size, output_block, &output_block_size, wrkmem);
	if (ret_val != LZO_E_OK || output_block_size > input_block_size + input_block_size / 16 + 64 + 3) {
	           printf("internal error - compression failed");
	          free(output_block);
	          free(wrkmem);
	           return -1;
       	}
       	free(output_block);
	free(wrkmem);

	// Return size of compressed block.
	return output_block_size;
}  

/**
* This function determines the compression savings of the File.
* It splits the file into appropriate chunks of block and the calls the lzo compression api
* @param file_name Name of the file whose compression factor is to be determined. Cannot be null.
*/
void
compression_savings(char *file_name) {

	if (file_name == NULL) {
		return;
	}

	// If the file_name is a directory, then ignore. LZO doensn't compress directory
	struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
	stat(file_name, buf);
	if (buf->st_mode & S_IFDIR) {
		printf("\n Ignoring directory  %s ", file_name);
		free(buf);
		return;
	}
	free(buf);

	/* Open the file */
	FILE *fp = fopen(file_name, "rb");
	if (fp == NULL) {
		printf("\n Cannot open file %s", file_name);
		return;	
	}

	/* Find the size of the file*/
	fseek(fp,0,SEEK_END);
	long long int file_size=ftell(fp); 
	//printf("\n SIZE OF FILE IN BYTES : %ld ", file_size);

	/* If file size = 0, then ignore */
	if (file_size == 0 || file_size < 0) {
		printf("\n File is Empty. Ignoring file %s", file_name);
		return;
	}

	// Set the file pointer to the start
	fseek(fp, SEEK_SET, 0);

	/* 
	* Reference : Some Results on Compressibility Using LZO Algorithm, K. Kant, Intel Research
	* http://www.kkant.net/papers/lzo_results.pdf
	* The above paper says that the compression block size for resonable latencies and good performance should be 256 bytes. 
	* Assuming this is optimal, we will stick to this.
	*/
	long number_of_blocks = file_size / COMPRESSION_BLOCK_SIZE;
	int last_block_bytes = file_size % COMPRESSION_BLOCK_SIZE;

	if (lzo_init() != LZO_E_OK) {
        		printf("internal error - lzo_init() failed !!!\n");
        		return;
    	}

	/*
	* Call compression_savings_util() and compress each block.
	* Store the compression size to determine compression factor
	*/
	lzo_bytep read_buffer = (lzo_bytep)malloc(COMPRESSION_BLOCK_SIZE);
	int bytes_read = 0;
	int compressed_size = 0;
	int blk_number = 0;
	for (blk_number = 1; blk_number <= number_of_blocks; blk_number++) {
		bytes_read = fread((void *)read_buffer, (size_t)1, (size_t)COMPRESSION_BLOCK_SIZE, fp);
		if (bytes_read == 0) {
			break;
		}
		compressed_size += compression_savings_util(read_buffer, COMPRESSION_BLOCK_SIZE);
	}

	// Read last block from the file
	bytes_read = fread(read_buffer, 1, last_block_bytes, fp);
	if (bytes_read != 0) {
		compressed_size += compression_savings_util(read_buffer, last_block_bytes); 
	}
	free(read_buffer);
	fclose(fp);

	// Write to CSV file
	pthread_mutex_lock(&csv_lock);
	if (csv_fp == NULL) {
		printf("\n CSV FP is NULL\n");
	}
	fprintf(csv_fp, "%s \t\t\t  %lld \t\t\t %2.2f%% \t\t\t %d \n", file_name, file_size, (file_size - compressed_size)*100.0/file_size, compressed_size);
	printf("\n%s    %lld    %2.2f%%    %d", file_name, file_size, (file_size - compressed_size)*100.0/file_size, compressed_size);
	pthread_mutex_unlock(&csv_lock);
}
