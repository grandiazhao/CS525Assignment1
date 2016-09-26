#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// test name
char *testName;

/* test output files */
#define TESTPF "test_pagefile.bin"

/* prototypes for test functions */
static void testCreate(void);
static void testReadWriteMethod(void);
static void testAppendPage(void);
static void testMultiPageContent(void);
static void testEnsureCapacity(void);

/* main function running all tests */
int
main (void)
{
  testName = "";
  
  initStorageManager();
  // test function
  testCreate();
  testReadWriteMethod();
  testAppendPage();
  testMultiPageContent();
  testEnsureCapacity();

  return 0;
}

/*  Function Name: testCreate
 *  Test:  Open a file before creation should return error
 *         The new file create contains one page filled with '\0'
 *         Cannot open a file after destroyed
 */
void testCreate(void) {
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i = 0;
  testName = "test things about create new file ";
  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // before create, we can not open a non-existing file
  ASSERT_ERROR((openPageFile(TESTPF, &fh)), "opening non-existing file should return an error.");

  // create and open a file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("Create and open file \n");
  //read page from file to ph
  TEST_CHECK(readCurrentBlock(&fh, ph));  

  // check if the new file page is filled with '\0'
  for (i = 0; i < PAGE_SIZE; i++) {
    ASSERT_EQUALS_INT(0, ph[i], "the new page should be filled with zero types ");
  }
  printf("First file page is filled with zero bytes\n");

  //close and destroy
  TEST_CHECK(closePageFile (&fh));
  TEST_CHECK(destroyPageFile (TESTPF));
  printf("Close and destroy file \n");

  // cannot open a file after destroyed
  ASSERT_ERROR((openPageFile(TESTPF, &fh)), "opening a destroyed file should return error.");

  TEST_DONE();
}

/*  Function Name: testReadWriteMethod
 *  Test:  writeCurrentBlock can write to file
 *         readFirstBlock can read from file
 */
void testReadWriteMethod (void) {
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i = 0;
  testName = "test read and write method ";
  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // create and open a file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("Create and open file \n");

  //create a page filled with '1'
  memset(ph, '1', PAGE_SIZE);

  // write current block with all 1
  TEST_CHECK(writeCurrentBlock(&fh, ph));
  printf("write block with '1'\n");
  //reset page
  memset(ph, 0, PAGE_SIZE);

  // read block from file to ph
  TEST_CHECK(readFirstBlock(&fh, ph));
  printf("read block \n");

 // check if the page has been succfully read
  for (i = 0; i < PAGE_SIZE; i++) {
    ASSERT_TRUE((ph[i] == '1'), "the page should be filled with '1' ");
  }

  //close and destroy
  TEST_CHECK(closePageFile (&fh));
  TEST_CHECK(destroyPageFile (TESTPF));
  printf("Close and destroy file \n");

  TEST_DONE();
}

/*  Function Name: testAppendPage
 *  Test:  After append a new page, the total number of pages increase by 1.
 *         After append a new page, the current position should be the new page.
 *         The page appended should be filled with zero bytes.
 */
void testAppendPage(void) {
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;
  testName = "test append a page in file";
  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("Create and open file \n");

  //wirte first block all '1'
  memset(ph, '1', PAGE_SIZE);
  TEST_CHECK(writeCurrentBlock(&fh, ph));

  ASSERT_TRUE((fh.curPagePos == 0), "new file position is 0");
  // append a new block
  TEST_CHECK(appendEmptyBlock(&fh));
  ASSERT_EQUALS_INT(1, fh.curPagePos, "after append, page position is 1");
  ASSERT_EQUALS_INT(2, fh.totalNumPages, "after append, total number of pages is 2");

  // read second block from file to ph
  TEST_CHECK(readCurrentBlock(&fh, ph));
  printf("Read block \n");

  for (i = 0; i < PAGE_SIZE; i++) {
      ASSERT_EQUALS_INT(0, ph[i], "the page append should be filled with 0 ");
  }

  //close and destroy
  TEST_CHECK(closePageFile (&fh));
  TEST_CHECK(destroyPageFile (TESTPF));
  printf("Close and destroy file \n");

  TEST_DONE();
}

/*  Function Name: testMultiPageContent
 *  Test:  readFirstBlock read the first block of file.
 *         readPreviousBlock read the previous block of file.
 *         readLastBlock read the last block of file.
 *         readNextBlock read the next block of file.
 *         readCurrentBlock read the current block of file.
 *         readFirstBlock at the first position should return error.
 *         readNextBlock at the last position should return error.
 *         writeCurrentBlock write the current block without change other blocks.
 *         Open an exist file again wont change its total number of pages.
 */
void testMultiPageContent(void) {
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;
  testName = "test multi page content";
  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("Create and open file \n");

  printf("Create 5 blocks filled with '0'~'4' for each \n");
  //wirte block at position 0 with all '0'
  memset(ph, '0', PAGE_SIZE);
  TEST_CHECK(writeCurrentBlock(&fh, ph));

  //append and write block at position 1 with all '1'
  TEST_CHECK(appendEmptyBlock(&fh));
  memset(ph, '1', PAGE_SIZE);
  TEST_CHECK(writeCurrentBlock(&fh, ph));

  //append and write block at position 1 with all '2'
  TEST_CHECK(appendEmptyBlock(&fh));
  memset(ph, '2', PAGE_SIZE);
  TEST_CHECK(writeCurrentBlock(&fh, ph));

  //append and write block at position 1 with all '3'
  TEST_CHECK(appendEmptyBlock(&fh));
  memset(ph, '3', PAGE_SIZE);
  TEST_CHECK(writeCurrentBlock(&fh, ph));

  //append and write block at position 1 with all '4'
  TEST_CHECK(appendEmptyBlock(&fh));
  memset(ph, '4', PAGE_SIZE);
  TEST_CHECK(writeCurrentBlock(&fh, ph));
  //test position and total number after append
  ASSERT_EQUALS_INT(4, fh.curPagePos, "after append, page position is 4");
  ASSERT_EQUALS_INT(5, fh.totalNumPages, "after append, total number of pages is 5");

  printf("Test each method about readblock \n");
  // test readFirstBlock   
  TEST_CHECK(readFirstBlock(&fh, ph));
  for (i = 0; i < PAGE_SIZE; i++) {
      ASSERT_EQUALS_INT('0', ph[i], "the first block is filled with '0' ");
  }
  ASSERT_EQUALS_INT(0, fh.curPagePos, "after readFirstBlock, page position is 0");

  // readFirstBlock at the first position should return error
  ASSERT_ERROR(readPreviousBlock(&fh, ph), "readPreviousBlock should return error when at the first block");
  ASSERT_EQUALS_INT(0, fh.curPagePos, "after readFirstBlock return error, page position remains the same");

  //test readNextBlock
  TEST_CHECK(readNextBlock(&fh, ph));
  for (i = 0; i < PAGE_SIZE; i++) {
      ASSERT_EQUALS_INT('1', ph[i], "the second block is filled with '1' ");
  }
  ASSERT_EQUALS_INT(1, fh.curPagePos, "after readNextBlock, page position is 1");

  //test readLastBlock
  TEST_CHECK(readLastBlock(&fh, ph));
  for (i = 0; i < PAGE_SIZE; i++) {
      ASSERT_EQUALS_INT('4', ph[i], "the last block is filled with '4' ");
  }
  ASSERT_EQUALS_INT(4, fh.curPagePos, "after readLastBlock, page position is 4");

  // readNextBlock at the last position should return error
  ASSERT_ERROR(readNextBlock(&fh, ph), "readNextBlock should return error when at the last block");
  ASSERT_EQUALS_INT(4, fh.curPagePos, "after readNextBlock return error, page position remains the same");

  //test readPreviousBlock
  TEST_CHECK(readPreviousBlock(&fh, ph));
  for (i = 0; i < PAGE_SIZE; i++) {
      ASSERT_EQUALS_INT('3', ph[i], "the 4th block is filled with '3' ");
  }
  ASSERT_EQUALS_INT(3, fh.curPagePos, "after readPreviousBlock, page position is 3");

  //test readCurrentBlock
  TEST_CHECK(readCurrentBlock(&fh, ph));
  for (i = 0; i < PAGE_SIZE; i++) {
      ASSERT_EQUALS_INT('3', ph[i], "the 4th block is filled with '3' ");
  }
  ASSERT_EQUALS_INT(3, fh.curPagePos, "after readCurrentBlock, page position is 3");


  printf("Test read file again \n");
  //test open file again
  TEST_CHECK(closePageFile (&fh));

  TEST_CHECK(openPageFile (TESTPF, &fh));
  ASSERT_EQUALS_INT(5, fh.totalNumPages, "expect 5 pages when open again");
 // ASSERT_EQUALS_INT(3, fh.curPagePos, "page position should be still 3"); //the requirement doesn't mention this part

  TEST_CHECK(closePageFile (&fh));
  TEST_CHECK(destroyPageFile (TESTPF));
  printf("Close and destroy file \n");

  TEST_DONE();

}

/*  Function Name: testEnsureCapacity
 *  Test:  EnsureCapacity to a larger number n will increase the number of pages to n.
 *         EnsureCapacity to the current number doesn't change anything.
 *         EnsureCapacity to a smaller number than current should return an error.
 *         New pages generated by EnsureCapacity are filled with '\0'.
 */
void testEnsureCapacity(void) {
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;
  testName = "test multi page content";
  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("Create and open file \n");

  printf("Test ensureCapacity to a larger / the current / a smaller number \n");
  TEST_CHECK(ensureCapacity(3, &fh));
  ASSERT_EQUALS_INT(3, fh.totalNumPages, "expect 3 pages after ensureCapacity to 3, a larger number");

  memset(ph, '1', PAGE_SIZE);
  readCurrentBlock(&fh, ph);
  for (i = 0; i < PAGE_SIZE; i++) {
    ASSERT_TRUE((ph[i] == 0), "New pages generated by EnsureCapacity are filled with zero types");
  }

  TEST_CHECK(ensureCapacity(3, &fh));
  ASSERT_EQUALS_INT(3, fh.totalNumPages, "expect 3 pages after ensureCapacity to 3, the same as current");

  ASSERT_ERROR(ensureCapacity(2, &fh), "expect error after ensureCapacity to 2, a smaller number");
  ASSERT_EQUALS_INT(3, fh.totalNumPages, "expect remain 3 pages after ensureCapacity return an error");

  TEST_CHECK(closePageFile (&fh));
  TEST_CHECK(destroyPageFile (TESTPF));
  printf("Close and destroy file \n");

  TEST_DONE();

}
