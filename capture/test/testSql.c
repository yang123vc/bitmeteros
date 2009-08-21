#include "test.h"
#include "common.h"
#include <string.h>
#include <stdarg.h>
#include "capture.h"
#include "CuTest.h"

void setup();
static int callback(void *notUsed, int argc, char **argv, char **azColName);
static int cbStoreRows(void *notUsed, int argc, char **argv, char **azColName);
static int callBackCount, rowCount, now;
static int getRowCount();

struct RowData {
    unsigned long ts;
    unsigned long dr;
	unsigned long dl;
	unsigned long ul;
	unsigned char* ad;
	struct RowData* next;
};
static struct RowData* storedData;

void getDbPath(char* path){
    strcpy(path, IN_MEMORY_DB);
}

void doSleep(int interval){

}
void getLogPath(char* path){
    strcpy(path, "");
}
int getTime(){
    return now;
}

void testUpdateDbNull(CuTest *tc) {
    int rowsBefore = getRowCount();
    updateDb(1,1,NULL);
    int rowsAfter = getRowCount();
    CuAssertIntEquals(tc, rowsBefore, rowsAfter);
}

void testUpdateDbMultiple(CuTest *tc) {
    int rowsBefore = getRowCount();
    struct BwData data3 = { 3, 3, 5, "eth0", NULL};
    struct BwData data2 = { 2, 2, 5, "eth1", &data3};
    struct BwData data1 = { 1, 1, 5, "eth2", &data2};

    updateDb(1,1,&data1);
    int rowsAfter = getRowCount();
    CuAssertIntEquals(tc, rowsBefore + 3, rowsAfter);
}

void testGetNextCompressTime(CuTest *tc){
    now = 1234;
    CuAssertIntEquals(tc, 1234 + 3600, getNextCompressTime());
}

void testCompressSec1Adapter(CuTest *tc){
    now = 7200;
    executeSql("delete from data", NULL);
    executeSql("insert into data values (3601, 1, 'eth0',  1,  1)", NULL);
    executeSql("insert into data values (3600, 1, 'eth0',  2,  2)", NULL);
    executeSql("insert into data values (3599, 1, 'eth0',  4,  4)", NULL);
    executeSql("insert into data values (3598, 1, 'eth0',  8,  8)", NULL);
    executeSql("insert into data values (3597, 1, 'eth0', 16, 16)", NULL);
    compressDb();

    struct RowData row1 = {3601, 1,   1,  1, "eth0", NULL};
    struct RowData row2 = {3600, 60, 30, 30, "eth0", NULL};

    checkTableContents(tc, 2, row1, row2);
}

void testCompressSecMultiAdapters(CuTest *tc){
    now = 7200;
    executeSql("delete from data", NULL);
    executeSql("insert into data values (3601, 1, 'eth0',  1,  1)", NULL);
    executeSql("insert into data values (3601, 1, 'eth1',  2,  2)", NULL);
    executeSql("insert into data values (3601, 1, 'eth2',  4,  4)", NULL);
    executeSql("insert into data values (3600, 1, 'eth0',  8,  8)", NULL);
    executeSql("insert into data values (3600, 1, 'eth1', 16, 16)", NULL);
    executeSql("insert into data values (3600, 1, 'eth2', 32, 32)", NULL);
    executeSql("insert into data values (3599, 1, 'eth0', 64, 64)", NULL);
    executeSql("insert into data values (3598, 1, 'eth1',128,128)", NULL);
    executeSql("insert into data values (3597, 1, 'eth2',256,256)", NULL);
    compressDb();

    struct RowData row1 = {3601, 1,    1,   1, "eth0", NULL};
    struct RowData row2 = {3601, 1,    2,   2, "eth1", NULL};
    struct RowData row3 = {3601, 1,    4,   4, "eth2", NULL};
    struct RowData row4 = {3600, 60,  72,  72, "eth0", NULL};
    struct RowData row5 = {3600, 60, 144, 144, "eth1", NULL};
    struct RowData row6 = {3600, 60, 288, 288, "eth2", NULL};

    checkTableContents(tc, 6, row1, row2, row3, row4, row5, row6);
}

void testCompressSecMultiIterations(CuTest *tc){
    now = 7200;
    executeSql("delete from data", NULL);
    executeSql("insert into data values (3601, 1, 'eth0',    1,    1)", NULL);
    executeSql("insert into data values (3601, 1, 'eth1',    2,    2)", NULL);
    executeSql("insert into data values (3601, 1, 'eth2',    4,    4)", NULL);
    executeSql("insert into data values (3600, 1, 'eth0',    8,    8)", NULL);
    executeSql("insert into data values (3600, 1, 'eth1',   16,   16)", NULL);
    executeSql("insert into data values (3600, 1, 'eth2',   32,   32)", NULL);
    executeSql("insert into data values (3599, 1, 'eth0',   64,   64)", NULL);
    executeSql("insert into data values (3598, 1, 'eth1',  128,  128)", NULL);
    executeSql("insert into data values (3597, 1, 'eth2',  256,  256)", NULL);
    executeSql("insert into data values (3540, 1, 'eth0',  512,  512)", NULL);
    executeSql("insert into data values (3540, 1, 'eth1', 1024, 1024)", NULL);
    executeSql("insert into data values (3540, 1, 'eth2', 2048, 2048)", NULL);
    executeSql("insert into data values (3539, 1, 'eth0', 4096, 4096)", NULL);
    executeSql("insert into data values (3538, 1, 'eth1', 8192, 8192)", NULL);
    executeSql("insert into data values (3537, 1, 'eth2',16384,16384)", NULL);
    compressDb();

    struct RowData row1 = {3601, 1,     1,    1, "eth0", NULL};
    struct RowData row2 = {3601, 1,     2,    2, "eth1", NULL};
    struct RowData row3 = {3601, 1,     4,    4, "eth2", NULL};
    struct RowData row4 = {3600, 60,   72,   72, "eth0", NULL};
    struct RowData row5 = {3600, 60,  144,  144, "eth1", NULL};
    struct RowData row6 = {3600, 60,  288,  288, "eth2", NULL};
    struct RowData row7 = {3540, 60, 4608, 4608, "eth0", NULL};
    struct RowData row8 = {3540, 60, 9216, 9216, "eth1", NULL};
    struct RowData row9 = {3540, 60,18432,18432, "eth2", NULL};

    checkTableContents(tc, 9, row1, row2, row3, row4, row5, row6, row7, row8, row9);
}

void testCompressMin1Adapter(CuTest *tc){
    now = 86400 + 3600;
    executeSql("delete from data", NULL);
    executeSql("insert into data values (3601, 60, 'eth0',  1,  1)", NULL);
    executeSql("insert into data values (3600, 60, 'eth0',  2,  2)", NULL);
    executeSql("insert into data values (3599, 60, 'eth0',  4,  4)", NULL);
    executeSql("insert into data values (3598, 60, 'eth0',  8,  8)", NULL);
    executeSql("insert into data values (3597, 60, 'eth0', 16, 16)", NULL);
    compressDb();

    struct RowData row1 = {3601,   60,   1,  1, "eth0", NULL};
    struct RowData row2 = {3600, 3600,  30, 30, "eth0", NULL};

    checkTableContents(tc, 2, row1, row2);
}

void testCompressMinMultiAdapters(CuTest *tc){
    now = 86400 + 3600;
    executeSql("delete from data", NULL);
    executeSql("insert into data values (3601, 60, 'eth0',  1,  1)", NULL);
    executeSql("insert into data values (3601, 60, 'eth1',  2,  2)", NULL);
    executeSql("insert into data values (3601, 60, 'eth2',  4,  4)", NULL);
    executeSql("insert into data values (3600, 60, 'eth0',  8,  8)", NULL);
    executeSql("insert into data values (3600, 60, 'eth1', 16, 16)", NULL);
    executeSql("insert into data values (3600, 60, 'eth2', 32, 32)", NULL);
    executeSql("insert into data values (3599, 60, 'eth0', 64, 64)", NULL);
    executeSql("insert into data values (3598, 60, 'eth1',128,128)", NULL);
    executeSql("insert into data values (3597, 60, 'eth2',256,256)", NULL);
    compressDb();

    struct RowData row1 = {3601,   60,   1,   1, "eth0", NULL};
    struct RowData row2 = {3601,   60,   2,   2, "eth1", NULL};
    struct RowData row3 = {3601,   60,   4,   4, "eth2", NULL};
    struct RowData row4 = {3600, 3600,  72,  72, "eth0", NULL};
    struct RowData row5 = {3600, 3600, 144, 144, "eth1", NULL};
    struct RowData row6 = {3600, 3600, 288, 288, "eth2", NULL};

    checkTableContents(tc, 6, row1, row2, row3, row4, row5, row6);
}

void checkTableContents(CuTest *tc, int rowCount, ...){
    va_list ap;
    va_start(ap,rowCount);
    storedData = NULL;
    executeSql("select ts, dr, dl, ul, ad from data order by ts desc, ad asc", cbStoreRows);

    struct RowData expected;
    struct RowData* pStored   = storedData;

    int i;
    for(i=0; i<rowCount; i++){
        expected = va_arg(ap, struct RowData);
        CuAssertTrue(tc, pStored != NULL);
        CuAssertIntEquals(tc, expected.ts, pStored->ts);
        CuAssertIntEquals(tc, expected.dr, pStored->dr);
        CuAssertIntEquals(tc, expected.dl, pStored->dl);
        CuAssertIntEquals(tc, expected.ul, pStored->ul);
        CuAssertStrEquals(tc, expected.ad, pStored->ad);

        pStored   = pStored->next;
    }
    va_end(ap);

    CuAssertTrue(tc, pStored == NULL);
}

void setup(){
    openDb();
    executeSql("create table config (key, value)", NULL);
    executeSql("insert into config (key,value) values ('cap.compress_interval',  3600)", NULL);
    executeSql("insert into config (key,value) values ('cap.keep_sec_limit',     3600)", NULL);
    executeSql("insert into config (key,value) values ('cap.keep_min_limit',     86400)", NULL);
    executeSql("insert into config (key,value) values ('cap.busy_wait_interval', 60000)", NULL);
    executeSql("create table data (ts,dr,ad,dl,ul)", NULL);
    setupDb();
}

CuSuite* sqlGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testUpdateDbNull);
    SUITE_ADD_TEST(suite, testUpdateDbMultiple);
    SUITE_ADD_TEST(suite, testCompressSec1Adapter);
    SUITE_ADD_TEST(suite, testCompressSecMultiAdapters);
    SUITE_ADD_TEST(suite, testCompressSecMultiIterations);
    SUITE_ADD_TEST(suite, testCompressMin1Adapter);
    SUITE_ADD_TEST(suite, testCompressMinMultiAdapters);
    SUITE_ADD_TEST(suite, testGetNextCompressTime);
    return suite;
}

static int cbCountRows(void *notUsed, int argc, char **argv, char **azColName){
    char* countTxt = argv[0];
    rowCount = atoi(countTxt);
    return 0;
}

static int cbStoreRows(void *notUsed, int argc, char **argv, char **azColName){
	char* ts = argv[0];
	char* dr = argv[1];
	char* dl = argv[2];
	char* ul = argv[3];
    char* ad = argv[4];

	struct RowData* newData = (struct RowData*) malloc (sizeof (struct RowData));
	newData->ts = atoi(ts);
	newData->dr = atoi(dr);
	newData->dl = atoi(dl);
	newData->ul = atoi(ul);
	newData->next = NULL;

    int addrLen = strlen(ad);
    char* addr = (char*) malloc(addrLen);
    strcpy(addr, ad);
    newData->ad = addr;

	if (storedData == NULL){
        storedData = newData;
	} else {
        struct RowData* oldData = storedData;
        while(oldData->next != NULL){
            oldData = oldData->next;
        }
        oldData->next = newData;
	}

    return 0;
}

static int getRowCount(){
    executeSql("select count(*) from data", cbCountRows);
    return rowCount;
}
