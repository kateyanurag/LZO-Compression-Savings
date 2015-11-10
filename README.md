======================================================
LZO algorithm to process a given file/directory path to determine compression savings
======================================================

Algorithm : LZO 1X
int lzo1x_1_compress ( const lzo_bytep src, lzo_uint  src_len,
                             lzo_bytep dst, lzo_uintp dst_len,
                             lzo_voidp wrkmem );

  Algorithm:            LZO1X
  Compression level:    LZO1X-1
  Memory requirements:  LZO1X_1_MEM_COMPRESS    (64 KiB on 32-bit machines)

 This compressor is pretty fast without multi threading

  Return value: LZO_E_OK


  ======
  USAGE
  ======
  1) go to root directory of the workspace
  2 run the following commands
  	$ make clean
  	$ make or $ make lzo
  	$  ./lzo_compress [file path or dir path] [-f]

* path is the path of file or directory. 
* -f is to enable threading for fast processing
* Both the parameters are optional. 
* If path not provided, all the files in the current directory will be considered.
* By default threading is disabled

