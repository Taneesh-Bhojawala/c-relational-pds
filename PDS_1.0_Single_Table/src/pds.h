#include <stdio.h>

#define MAX 10000
#define SUCCESS 0
#define FAILURE 1
#define REC_NOT_FOUND 1
#define DB_OPEN 0
#define DB_CLOSE 1

/*Now making it generic so we remove this */

// struct Course
// {
//     int course_num;
//     char course_name[50];
//     char course_instructor[50];
// };

struct Rec_ndx
{
    int key;
    int loc;
    int is_deleted;
    int old_key;
};

struct DbInfo
{
    FILE *dbfile;
    FILE *ndxfile;
    char dbname[50];
    char ndxname[50];
    int status;
    struct Rec_ndx ndxArray[MAX];
    int rec_count;
    int rec_size;
};

extern struct DbInfo db_info;

int create_db(char *dbname);  //creates empty file(open in wb mode and close the file)

int open_db(char *dbname, int rec_size);    //simply opens the file in rb+ mode(open existing file for update)

void db_init();   //initialize the cdb_info

int store_db(int key, void *c);    //fseeks to end of file and then fwrites the struct to end of file.

int get_db(int key, void *output);    //fseek to begenning, iterate over each record until matching is found or eof reached, 0 if found, 1 if not found

int update_db(int key, void *new_rec);

int delete_db(int key);

int undelete_db(int key);

int close_db();    //close coursedb
