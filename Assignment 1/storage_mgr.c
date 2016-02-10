/**************************************************************************/
/*                       Storage Manager                                  */
/*                    =====================                               */
/*        Implementation of a storage manager that allows                 */ 
/*        read/writing of blocks to/from a file on disk                   */
/*          								  */
/**************************************************************************/

/* Headers */
#include "storage_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

/*=========================================================================*/
/*		Karthik K :: Implementation of Page Functions		   */
/*	     ===================================================	   */	 
/*			 Manipulating Page Files			   */
/*   			-------------------------                          */
/*              Functions Implemented :: => initStorageManager             */
/*             -----------------------   => createPageFile                 */
/*           			         => openPageFile                   */
/*           			         => closePageFile                  */
/*           			         => destroyPageFile 	           */
/*    				                                           */
/*=========================================================================*/

extern void initStorageManager (void)
{
	printf("\n<===== Storage Manager Initialization =====>\n");
}

/* Creates a Page File with the given fileName */
extern RC createPageFile(char *fileName)
{
	/* Declarations */
	char *buffer;                                
	int write_status;
	FILE *fp;

	buffer=(char *)calloc(PAGE_SIZE,sizeof(char));      // Allocates Memory initially and returns a pointer to the allocated Memory

	fp = fopen(fileName,"wb+");  			    // Opens the file in Write/Update Mode                       

	if(fp == NULL)                                      // To check if the File was succesfully opened
	{
		return RC_FILE_NOT_FOUND;	
	}
	else
	{
		write_status = fwrite(buffer,1,PAGE_SIZE,fp);  // Writes data of size 1 byte and the result of fwrite is stored in write_status
		fclose(fp);          			       // Closing the FilePointer
		free(buffer);        			       // Freeing the Allocated Memory

		if(write_status)
		{
			return RC_OK;
		}
		else
		{
			return RC_WRITE_FAILED;
		}
	}	

}

/* Opens an Existing Page File and Initializes the File Handler */
extern RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{
	/* Declarations and Initializations */
	int fileSize = 0;
	int Num_Pages;
	FILE *fp;

	fp = fopen(fileName,"rb+");    		     // Opens the File in Read/Update Mode for input/output Operations 

	if(fp == NULL) 				     // To check if the File was succesfully opened
	{
		return RC_FILE_NOT_FOUND;
	}
	else
	{
		fseek(fp,0,SEEK_END); 		     // Sets the file position indicator pointed to by fp and the offset is EOF
		fileSize = ftell(fp);  		     // Returns the Current value of the file-position indicator for the file pointed to by fp
		fseek(fp,0,SEEK_SET); 	             // Sets the file position indicator pointed to by fp with the offset to the start of file 

		Num_Pages = fileSize%PAGE_SIZE;      // Calculating the Number of Pages

		if(Num_Pages != 0)
		{
			return RC_READ_NON_EXISTING_PAGE;
		}
		else
		{
			/* Assigning values to SM_FileHandle structure components */
			fHandle->curPagePos = 0;
			fHandle->mgmtInfo = fp;
			fHandle->fileName = fileName;
			fHandle->totalNumPages = fileSize/PAGE_SIZE;
		}
	}
	return RC_OK;
}

/* Closing an Open Page File */
extern RC closePageFile(SM_FileHandle *fHandle){

	int close_status;
	close_status = fclose(fHandle->mgmtInfo);      // Flushes the stream, closes the File descriptor & result of fclose is stored in close_status

	if(close_status)			       // To check if the File was succesfully closed
	{
		return RC_FILE_NOT_FOUND;          
	}
	return RC_OK;

}

/* Deletes an Open Page File */
extern RC destroyPageFile(char *fileName){

	if((remove(fileName) != 0))                    //Removing the File
	{           
		return RC_FILE_NOT_FOUND;        
	}
	else
	{
		return RC_OK;
	}
}

/*=========================================================================*/
/*		Akshay Shanbhag :: Implementation of File Read Function	   */
/*	     ===================================================	   */	 
/*			 						   */
/*   			------------------------                           */
/*              Functions Implemented :: => readFirstBlock                 */
/*             -----------------------   => readBlock                      */
/*           			         => readPreviousBlock              */
/*           			         => readNextBlock                  */
/*           			         => readLastBlock                  */
/*					 => readCurrentBlock               */
/*					 => getBlockPos			   */
/*    				                                           */   
/*=========================================================================*/

//Generic Error Messages defined for File Read operations.
#define RC_NO_PREVIOUS_BLOCK 5
#define RC_NO_NEXT_BLOCK 6
#define RC_NO_FIRST_BLOCK 7
#define RC_NO_LAST_BLOCK 8
#define RC_NO_CURRENT_BLOCK 9


/*readFirstBlock method takes filehandle and memPage as input parameters and reads the first block of memory of 4096 bytes from the file and returns RC_OK if operation is successful.*/

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	printf("*** Inside readFirstBlock method ***\n");
	int rc = RC_NO_FIRST_BLOCK;//Error message if no first block exists.

	if(fHandle != NULL)
	{
		rc = readBlock(0,fHandle,memPage);// calling readblock method from begining
	}
	printf("*** Exiting readFirstBlock method ***\n");
	return rc;
}


/* readBlock method takes pageNum,fhandle and character array as input parameters and reads the block of memory with the given pagenumber from the file and returns RC_OK if operation is completed. */

RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{	
	printf("*** Inside readBlock method ***\n");

	FILE *fp;	

	int rc = RC_FILE_HANDLE_NOT_INIT; //generic error message for file handle not initialized.

	if(fHandle != NULL)//check if fHandle exists
	{


		fp = fopen(fHandle->fileName, "r"); // opens the file to read.


		if(fp != NULL) // check if file exists.
		{	
			if(pageNum > fHandle->totalNumPages)
			{
				rc= RC_READ_NON_EXISTING_PAGE;
			}
			else
			{


				fseek(fp,pageNum*PAGE_SIZE,SEEK_SET);	//sets the file position of the stream i.e. fp to the given offset i.e. pageNum*PAGE_SIZE from begining.	

				int size = fread(memPage,1,PAGE_SIZE,fp); // reads 4096 elements each of the size of 1 byte from fp into the array pointed by memPage.

				fHandle->curPagePos = pageNum;//set the current page position to the current page number.

				if(size==PAGE_SIZE)//checking for successful reading
				{
					rc = RC_OK; //success message
					printf("*** File read successfully ***\n");
				}

			}
			fclose(fp); //close the file.
		}	
		else
		{
			rc = RC_FILE_NOT_FOUND; 
		}

	}
	printf("*** Exiting readBlock method ***\n");
	return rc;
}



/* readPrevBlock method takes fhandle and character array as input parameters and reads the previous block of 4096 bytes of memory page from the current postion of the file and returns RC_OK if operation is completed. */

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	printf("*** Inside readPreviousBlock method ***\n");	
	int rc = RC_NO_PREVIOUS_BLOCK;//Error message if no previous block exists.
	if(fHandle != NULL)//check if fHandle exists
	{		
		int prevBlock = getBlockPos(fHandle) - 1; //get current block position and subtract 1 block
		rc = readBlock(prevBlock,fHandle,memPage);//calling readblock method
	}
	printf("*** Exiting readPreviousBlock method ***\n");
	return rc;	
}



/* readNextBlock method takes fhandle and character array as input parameters and reads the nextblock of 4096 bytes of memory page from the current postion of the file and returns RC_OK if operation is completed. */
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	printf("*** Inside readNextBlock method ***\n");
	int rc = RC_NO_NEXT_BLOCK;//Error message if no next block exists.
	if(fHandle != NULL)//check if fHandle exists
	{		
		int nextBlock = getBlockPos(fHandle) + 1; //get current block position and add 1 block;
		rc = readBlock(nextBlock,fHandle,memPage);//calling readblock method
	}
	printf("*** Exiting readNextBlock method ***\n");
	return rc;
}



/* readLastBlock method takes fhandle and character array as arguments and returns RC_OK if operation is completed. */
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	printf("*** Inside readLastBlock method ***\n");
	int rc = RC_NO_LAST_BLOCK;//Error message if no last block exists.
	if(fHandle != NULL)//check if fHandle exists
	{		
		int lastBlock = fHandle->totalNumPages - 1; //get the last page number
		rc = readBlock(lastBlock,fHandle,memPage);//calling readblock method
	}
	printf("*** Exiting readLastBlock method ***\n");
	return rc;
}



/* readCurrentBlock method reads data from the file and returns RC_OK if execution is successful. */
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	printf("*** Inside readCurrentBlock method ***\n");
	int rc = RC_NO_CURRENT_BLOCK;//Error message if no current block exists.
	if(fHandle != NULL)//check if fHandle exists
	{		
		int currentBlock = getBlockPos(fHandle);// get current block/page position.
		rc = readBlock(currentBlock,fHandle,memPage);// calling readBlock method.
	}
	printf("*** Exiting readCurrentBlock method ***\n");
	return rc;

}



/* getBlockPos method prints and and returns current page position of the block in the file. */
int getBlockPos (SM_FileHandle *fHandle)
{
	int rc = RC_FILE_HANDLE_NOT_INIT; //generic error message for file handle not initialized.
	printf("*** Inside getBlockPos method ***\n");
	if (fHandle != NULL)//check if fhandle exists
	{
		return fHandle->curPagePos; // return block/page position
	}


}

