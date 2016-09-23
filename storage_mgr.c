#include "storage_mgr.h"
#include <stdio.h>
#include <malloc.h>

#ifdef __LINUXm__
#include <unistd.h>
#else
#include <io.h>
#endif

/*it seems there is no need to implement a header on each page based on the test_assign file...
typedef struct PageHeader{
    int index;
    char* pageName;
    void* addtionInfo;
    int sizeofHeader;
}PageHeader;

PageHeader* p_pageHeader=0;

void initPageHeader()
{
    p_pageHeader=(PageHeader*)malloc(sizeof(PageHeader));
    p_pageHeader->index=0;
    p_pageHeader->sizeofHeader=64;
}

void writePageHeader(FILE* fp)
{
    fwrite(&p_pageHeader->index,sizeof(int),1,fp);
    char ch='\0';
    fwrite(&ch,1,p_pageHeader->sizeofHeader-sizeof(p_pageHeader->index),fp);
}*/


/*  this file header contains basic file information, 
 *   and stored in the beginning of file     */
typedef struct DataBaseHeader{
	FILE* filePointer;
	int currentPage;
	int maxPageCount;
	char* additionalInfo;
	int sizeofHeader;
}DataBaseHeader;

//this is a databaseheader used in program to help read a page file
DataBaseHeader* p_dataBaseHeader = 0;

/*********************************************************************************
  *Function:        initDataBaseHeader
  *Description:     intial a file header 
  *Input:           None
  *Output:          None
  *Return:          None
**********************************************************************************/
void initDataBaseHeader()
{
    p_dataBaseHeader=(DataBaseHeader*)malloc(sizeof(DataBaseHeader));
    p_dataBaseHeader->currentPage=0;
    p_dataBaseHeader->maxPageCount=1;
    p_dataBaseHeader->filePointer=0;
    //reserve some space for future use, record the esstienal data
    p_dataBaseHeader->sizeofHeader=128;
}

/*********************************************************************************
 * Function:        writeDataBaseHeader
 * Description:     Write file header into the beginning of a file
 * Input:           FILE* fp: file pointer
 * Output:          None
 * Return:          None
 **********************************************************************************/
void writeDataBaseHeader(FILE* fp)
{
    fwrite(&p_dataBaseHeader->currentPage,sizeof(int),1,fp);
    fwrite(&p_dataBaseHeader->maxPageCount,sizeof(int),1,fp);
    // allocate memory for other info that may be used except currentPage and maxPageCount
    char* data=(char*)calloc(p_dataBaseHeader->sizeofHeader-sizeof(int)*2,1);
    fwrite(data,1,p_dataBaseHeader->sizeofHeader-sizeof(int)*2,fp);
	fflush(fp);
	free(data);
}

/*********************************************************************************
 * Function:        readDataBaseHeader
 * Description:     Read file header from the beginning of a file
 * Input:           FILE* fp: file pointer
 * Output:          None
 * Return:          None
 **********************************************************************************/
void readDataBaseHeader(FILE* fp)
{
    fread(&p_dataBaseHeader->currentPage,sizeof(int),1,fp);
    fread(&p_dataBaseHeader->maxPageCount,sizeof(int),1,fp);
    int length=p_dataBaseHeader->sizeofHeader-sizeof(int)*2;
    p_dataBaseHeader->additionalInfo=(char*)malloc(length);
    fread(p_dataBaseHeader->additionalInfo,1,length,fp);
}

/*********************************************************************************
 * Function:        initStorageManager
 * Description:     initial storageManager
 * Input:           None
 * Output:          None
 * Return:          None
 **********************************************************************************/
void initStorageManager()
{
    p_dataBaseHeader=0;
}

/*********************************************************************************
 * Function:        createPageFile
 * Description:     create a new page file with one page filled with '\0'
 * Input:           char* fileName: file name
 * Output:          None
 * Return:          RC: return code
 **********************************************************************************/
RC createPageFile(char* fileName)
{
    //check whether the file exsit, since we want to create, the file shouldn't exsit
	int ret = _access(fileName,0);
    
    //if the file already exit, do not create and return error.
    if(ret==0)
    {
        printf("The file %s already exsit! Do you mean to open the file?",fileName);
        return RC_FILE_ALREADY_EXIST;
    }

    //create the file with mode "wb", and check if it has been successfully created
    FILE* fp=fopen(fileName,"wb");
    if(fp==0)
    {
        printf("Can not create the file %s!!",fileName);
        return RC_FILE_OPEN_FAILED;
    }

    //record the file header, and write it into the beginning of the file
	initDataBaseHeader();
    p_dataBaseHeader->maxPageCount=1;
    p_dataBaseHeader->currentPage=0;
    p_dataBaseHeader->filePointer=fp;
    writeDataBaseHeader(fp);

    // write the page with '\0' into the file
	char* data=(char*)calloc(PAGE_SIZE,1);
	fwrite(data, 1, PAGE_SIZE, fp);
    fflush(fp);

    fclose(fp);

    // free data
    free(data);
    
    return RC_OK;
}

/*********************************************************************************
 * Function:        openPageFile
 * Description:     open an exist page file, read the file header to get the information 
 *                  and save these into handle
 * Input:           char* fileName: file name
 * Output:          SM_FileHandle *fHandle: file handle
 * Return:          RC: return code
 **********************************************************************************/
RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{
    //This function is almost the same as createPageFile

    //check whether the file exsit.
    int ret=_access(fileName,0|2);

    // Return error if the file doesn't exit.
    if(ret!=0)
    {
        printf("The file %s does not exsit or have no permit to write!",fileName);
        return RC_FILE_NOT_FOUND;
    }

    //open the file with mode 'rb', and check if it has been successfully created
    FILE* fp=fopen(fileName,"rb+");
    if(fp==0)
    {
        printf("Can not open the file %s!!",fileName);
        return RC_FILE_OPEN_FAILED;
    }

    //create a new dataBaseHeader,record the pointer
    initDataBaseHeader();
	p_dataBaseHeader->filePointer = fp;

    //read file header from file, and save the information in file handle
    readDataBaseHeader(fp);
    fHandle->mgmtInfo=p_dataBaseHeader;
    fHandle->fileName=fileName;
    fHandle->curPagePos=0;
    fHandle->totalNumPages=p_dataBaseHeader->maxPageCount;

    return RC_OK;
}

/*********************************************************************************
 * Function:        closePageFile
 * Description:     close a file
 * Input:           SM_FileHandle *fHandle: file handle
 * Output:          None
 * Return:          RC: return code
 **********************************************************************************/
RC closePageFile(SM_FileHandle *fHandle)
{
    // check if the handle is valid
    if(fHandle==0||fHandle->mgmtInfo==0)
    {
        printf("The fileHandle is Empty!!!");
        return RC_FILE_HANDLE_NOT_INIT;
    }

    // update the information in header
    p_dataBaseHeader=fHandle->mgmtInfo;

    // close file
    fclose(p_dataBaseHeader->filePointer);

    // write header to the file
    peipei1

    //we should delete the dataBaseHeader stored in mgmtInfo and then delete the fHandle
    free(p_dataBaseHeader->additionalInfo);
    free(p_dataBaseHeader);
    p_dataBaseHeader=0;
	fHandle->mgmtInfo = 0;

    return RC_OK;
}

/*********************************************************************************
 * Function:        destroyPageFile
 * Description:     destroy a file
 * Input:           char *fileName: file name
 * Output:          None
 * Return:          RC: return code
 **********************************************************************************/
RC destroyPageFile(char *fileName)
{
    //check whether the file is exsit. 
    int ret=_access(fileName,0);

    // Return error if it doesn't exit.
    if(ret!=0)
    {
        printf("The file %s does not exsit!",fileName);
        return RC_FILE_NOT_FOUND;
    }

    // remove file and check if it is successfully.
    ret=remove(fileName);
    if(ret!=0)
    {
        printf("Error in remove the file! Please check the permission!");
        return RC_FILE_REMOVE_FAILED;
    }

    return RC_OK;
}

/*********************************************************************************
 * Function:        check_readBlock_commonError
 * Description:     check if handle is valid
 * Input:           SM_FileHandle* fHandle: file handle
 * Output:          None
 * Return:          RC: return code
 **********************************************************************************/
RC check_readBlock_commonError(SM_FileHandle* fHandle)
{
    // check if handle is valid
	if (fHandle == 0 || fHandle->mgmtInfo == 0)
	{
		printf("The fileHandle is Empty!!!");
		return RC_FILE_HANDLE_NOT_INIT;
	}

	return RC_OK;
}

/*********************************************************************************
 * Function:        readBlock
 * Description:     read the pageNumth block from a file into memPage. 
 * Called By:       readCurrentBlock
                    readFirstBlock
                    readLastBlock
                    readNextBlock
                    readPreviousBlock
 * Input:           int pageNum: the sequence number of page that need to be read
                    SM_FileHandle* fHandle: file handle
 * Output:          SM_PageHandle memPage: the page handle that will be written
 * Return:          RC: return code
 **********************************************************************************/
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //check if handle given is valid
	RC check = check_readBlock_commonError(fHandle);
	if (check != RC_OK) return check;

    //get information from handle
	p_dataBaseHeader = fHandle->mgmtInfo;

    // check if pageNumber is valid.
	if (pageNum >= p_dataBaseHeader->maxPageCount)
	{
		printf("PAGENUM exceed MAXPAGECOUNT");
		return RC_READ_NON_EXISTING_PAGE;
	}

	fHandle->curPagePos = pageNum;

    // reset the file pointer to the beginning
    rewind(p_dataBaseHeader->filePointer);

    // get the position of pageNumth block
    fseek(p_dataBaseHeader->filePointer,PAGE_SIZE*pageNum + p_dataBaseHeader->sizeofHeader,SEEK_SET);

    //read the pageNumth block
    int readsize=fread(memPage,1,PAGE_SIZE,p_dataBaseHeader->filePointer);
	if (readsize != PAGE_SIZE)
	{
		return RC_ERROR;
	}

    return RC_OK;
}
/*********************************************************************************
 * Function:        getBlockPos
 * Description:     get the block position in a file. 
 * Input:           SM_FileHandle *fHandle: file handle
 * Output:          None
 * Return:          int: the block position
 **********************************************************************************/
peipei3

/*********************************************************************************
 * Function:        readCurrentBlock
 * Description:     read the current block from a file into memPage. 
 * Calls:           readBlock
 * Input:           SM_FileHandle* fHandle: file handle
 * Output:          SM_PageHandle memPage: the page handle that will be written
 * Return:          RC: return code
 **********************************************************************************/
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //check if handle given is valid
	RC check = check_readBlock_commonError(fHandle);
	if (check != RC_OK) return check;

    // check if current page position is valid.
	if (fHandle->curPagePos >= fHandle->totalNumPages)
	{
		printf("CurrentPagePos not in TotalPageNum");
		return RC_READ_NON_EXISTING_PAGE;
	}

    //read current block
    return readBlock(fHandle->curPagePos,fHandle,memPage);
}


/*********************************************************************************
 * Function:        readFirstBlock
 * Description:     read the first block from a file into memPage. 
 * Calls:           readBlock
 * Input:           SM_FileHandle* fHandle: file handle
 * Output:          SM_PageHandle memPage: the page handle that will be written
 * Return:          RC: return code
 **********************************************************************************/
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //check if handle given is valid
	RC check = check_readBlock_commonError(fHandle);
	if (check != RC_OK) return check;

    //read the first block
    return readBlock(0,fHandle,memPage);
}

/*********************************************************************************
 * Function:        readLastBlock
 * Description:     read the last block from a file into memPage. 
 * Calls:           readBlock
 * Input:           SM_FileHandle* fHandle: file handle
 * Output:          SM_PageHandle memPage: the page handle that will be written
 * Return:          RC: return code
 **********************************************************************************/
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //check if handle given is valid
	RC check = check_readBlock_commonError(fHandle);
	if (check != RC_OK) return check;

    // read last block
    return readBlock(fHandle->totalNumPages-1,fHandle,memPage);
}

/*********************************************************************************
 * Function:        readNextBlock
 * Description:     read the next block from a file into memPage. 
 * Calls:           readBlock
 * Input:           SM_FileHandle* fHandle: file handle
 * Output:          SM_PageHandle memPage: the page handle that will be written
 * Return:          RC: return code
 **********************************************************************************/
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //check if handle given is valid
	RC check = check_readBlock_commonError(fHandle);
	if (check != RC_OK) return check;

    // check if current page is the last one. Return error if it is.
    if(fHandle->curPagePos>=(fHandle->totalNumPages-1))
    {
		printf("Current Page is the last Block!");
        return RC_READ_NON_EXISTING_PAGE;
    }

    // read the next block
    return readBlock(fHandle->curPagePos+1,fHandle,memPage);
}

/*********************************************************************************
 * Function:        readPreviousBlock
 * Description:     read the previous block from a file into memPage. 
 * Calls:           readBlock
 * Input:           SM_FileHandle* fHandle: file handle
 * Output:          SM_PageHandle memPage: the page handle that will be written
 * Return:          RC: return code
 **********************************************************************************/
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //check if handle given is valid
	RC check = check_readBlock_commonError(fHandle);
	if (check != RC_OK) return check;

    // check if current page is the first one or invalid. Return error if it is.
	if (fHandle->curPagePos > fHandle->totalNumPages || fHandle->curPagePos==0)
	{
		printf("Current Page is the first Block or current Page exceed last Block!");
		return RC_READ_NON_EXISTING_PAGE;
	}

    // read the previous block
    return readBlock(fHandle->curPagePos-1,fHandle,memPage);
}

/*********************************************************************************
 * Function:        writeBlock
 * Description:     write the pageNumth block from a memPage into file. 
 * Called By:       writeCurrentBlock
 * Input:           int pageNum: the sequence number of page that need to be written
                    SM_FileHandle* fHandle: file handle
                    SM_PageHandle memPage: the page handle that will be written
 * Output:          None
 * Return:          RC: return code
 **********************************************************************************/
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //check if handle given is valid
	RC check = check_readBlock_commonError(fHandle);
	if (check != RC_OK) return check;

    //get information from handle
	p_dataBaseHeader = fHandle->mgmtInfo;

    // check if pageNum is valid
    if(pageNum>=p_dataBaseHeader->maxPageCount)
    {
		printf("The PageNum Exceed the MaxPageCount, Can not Write to Invalid Page!");
        return RC_WRITE_NON_EXISTING_PAGE;
    }

    //reset the file pointer to the beginning
    rewind(p_dataBaseHeader->filePointer);

    //get to the pageNumth position
    fseek(p_dataBaseHeader->filePointer,PAGE_SIZE*pageNum+p_dataBaseHeader->sizeofHeader,SEEK_SET);

    // write data from memPage to file
    fwrite(memPage,1,PAGE_SIZE,p_dataBaseHeader->filePointer);
	fflush(p_dataBaseHeader->filePointer);

    return RC_OK;
}

/*********************************************************************************
 * Function:        writeCurrentBlock
 * Description:     write the current block from a memPage into file. 
 * Calls:           writeBlock
 * Input:           SM_FileHandle* fHandle: file handle
                    SM_PageHandle memPage: the page handle that write from
 * Output:          None
 * Return:          RC: return code
 **********************************************************************************/
RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //check if handle given is valid
	RC check = check_readBlock_commonError(fHandle);
	if (check != RC_OK) return check;

    //chekc if current page position is valid
	if (fHandle->curPagePos >= fHandle->totalNumPages||fHandle->curPagePos<0)
	{
		printf("the CurrentPage is not Exist! Have you modified the FileHandle outside the Program???");
		return RC_WRITE_NON_EXISTING_PAGE;
	}

    //write block
    return writeBlock(fHandle->curPagePos,fHandle,memPage);
}

/*********************************************************************************
 * Function:        appendEmptyBlock
 * Description:     append an new empty block filled with zero bytes at the end of the file.
 * Called By:       ensureCapacity
 * Input:           SM_FileHandle* fHandle: file handle
 * Output:          None
 * Return:          RC: return code
 **********************************************************************************/
RC appendEmptyBlock(SM_FileHandle *fHandle)
{
    //check if handle given is valid
    if(fHandle==0)
    {
        printf("The fileHandle is NULL!!!, Function will exit.");
        return RC_FILE_HANDLE_NOT_INIT;
    }

    //get information from handle
    p_dataBaseHeader=(DataBaseHeader*)fHandle->mgmtInfo;
    FILE* fp=p_dataBaseHeader->filePointer;

    // get the last position of file
    rewind(fp);
    fseek(fp,0,SEEK_END);

    // increase the number of pages in file
    int num=p_dataBaseHeader->maxPageCount+1;

    // write a page of zero bytes at the end of the file
	char* data = (char*)calloc(PAGE_SIZE, 1);
    fwrite(data,1,PAGE_SIZE,fp);

    //update the handle information
    fHandle->totalNumPages=num;
    p_dataBaseHeader->maxPageCount=num;
	fHandle->curPagePos = num - 1;
	p_dataBaseHeader->currentPage = num - 1;

    return RC_OK;
}

/*********************************************************************************
 * Function:        ensureCapacity
 * Description:     increase the number of pages to numberOfPages if it is less than that.
 * Calls:           appendEmptyBlock
 * Input:           SM_FileHandle* fHandle: file handle
 * Output:          None
 * Return:          RC: return code
 **********************************************************************************/
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)
{
    // use a loop to check and increase the number of pages
    while(fHandle->totalNumPages<numberOfPages)
    {
        RC ret=appendEmptyBlock(fHandle);
		if (ret != RC_OK) return ret;
    }
    
	return RC_OK;
}



























