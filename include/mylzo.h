/*
* Custom header file which includes the header from liblzo.
*/

#include "minilzo.h"

#define size_t long int

// Compression block size is kept 256 for optimal performance
#define COMPRESSION_BLOCK_SIZE  256

extern int compression_savings_util(lzo_bytep input_block, int input_block_size);
void compression_savings(char *file_name);
extern void get_files(char *path, int enabled_threading);

/* This is the file pointer for the output file */
extern FILE *csv_fp;

/* Lock for csv output file  since it will be accessed by multiple threads*/
extern pthread_mutex_t csv_lock;