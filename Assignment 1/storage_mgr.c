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

