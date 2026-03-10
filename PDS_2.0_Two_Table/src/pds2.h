#include <stdio.h>

#define MAX 10000
#define SUCCESS 0
#define FAILURE 1
#define REC_NOT_FOUND 1
#define DB_OPEN 0
#define DB_CLOSE 1

struct Rec_ndx
{
    int key;
    int loc;
    int is_deleted;
    int old_key;
};

struct TableInfo
{
    char tablename[100];
    char ndxname[100];
    int rec_size;
    FILE *tfile;
    FILE *ndxfile;
    struct Rec_ndx ndxArray[MAX];
    int rec_count;
};

struct Db_Info
{
    struct TableInfo tinfo[2];
    int num_table;
    int db_status;
};

extern struct Db_Info db_info;

int db_create(char *tname1, char *tname2);
int db_open(char *tname1, char *tname2, int rec_size1, int rec_size2);
int db_close();
void init();

struct TableInfo * get_table_info(char *tname);

int table_create(char *tname);
int table_open(char *tname, int rec_size);
int table_store(char *tname, int key, void *c);
int table_get(char *tname, int key, void *output);
int table_update(char *tname, int key, void *new_rec);
int table_delete(char *tname, int key); 
int table_undelete(char *tname, int key);
int table_close(struct TableInfo *table);