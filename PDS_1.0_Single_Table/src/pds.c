#include <stdio.h>
#include <stdlib.h>
#include "pds.h"
#include <string.h>

int create_db(char *dbname)
{
    //must include init() in main...
    char dbfilename[50];
    char ndxfilename[50];
    strcpy(dbfilename, dbname);
    strcat(dbfilename, ".dat");
    strcpy(ndxfilename, dbname);
    strcat(ndxfilename, ".ndx");
    FILE *fp = fopen(dbfilename, "wb");
    if(fp == NULL)
    {
        perror("\nError creating the database file!");
        return FAILURE;
    }
    fclose(fp);

    FILE *fp1 = fopen(ndxfilename, "wb");
    if(fp1 == NULL)
    {
        perror("\nError creating the index file!");
        return FAILURE;
    }
    fwrite(&db_info.rec_count, sizeof(db_info.rec_count), 1, fp1);
    fclose(fp1);
    return SUCCESS;
}

void db_init()
{
    db_info.dbfile = NULL;
    db_info.ndxfile = NULL;
    strcpy(db_info.dbname, "");
    strcpy(db_info.ndxname, "");
    db_info.status = DB_CLOSE;
    db_info.rec_count = 0;
    db_info.rec_size = 0;
}

int open_db(char *dbname, int rec_size)
{
    if(db_info.status == DB_OPEN && db_info.dbfile == NULL) {
        db_init();
    }

    char dbfilename[50];
    char ndxfilename[50];
    strcpy(dbfilename, dbname);
    strcat(dbfilename, ".dat");
    strcpy(ndxfilename, dbname);
    strcat(ndxfilename, ".ndx");

    strcpy(db_info.dbname, dbfilename);
    strcpy(db_info.ndxname, ndxfilename);
    db_info.rec_size = rec_size;

    if(db_info.status == DB_CLOSE)
    {
        FILE *fp = fopen(db_info.dbname, "rb+");        //rb+ is read and update
        if(fp == NULL)
        {
            perror("\nError opening the database!");
            db_info.status = DB_CLOSE;
            return FAILURE;
        }
        db_info.dbfile = fp;

        FILE *fp1 = fopen(db_info.ndxname, "rb+");
        if(fp1 == NULL)
        {
            perror("\nError opening the index file!");
            fclose(fp);
            return FAILURE;
        }
        
        fseek(fp1, 0, SEEK_SET);
        fread(&db_info.rec_count, sizeof(int), 1, fp1);
        fread(db_info.ndxArray, sizeof(struct Rec_ndx), db_info.rec_count, fp1);
        fclose(fp1);
        
        db_info.status = DB_OPEN;
        return SUCCESS;
    }
    printf("Database already open!\n");
    return FAILURE;
}

int store_db(int key, void *c)
{
    if(db_info.status == DB_CLOSE) return FAILURE;
    if(db_info.rec_count<MAX)
    {
        fseek(db_info.dbfile, 0, SEEK_END);
        int loc = (int)ftell(db_info.dbfile);
        fwrite(&key, sizeof(int), 1, db_info.dbfile);
        fwrite(c, db_info.rec_size, 1, db_info.dbfile);

        struct Rec_ndx indx;
        indx.key = key;
        indx.loc = loc;
        indx.is_deleted = 0;
        indx.old_key = -1;

        db_info.ndxArray[db_info.rec_count++] = indx;
        return SUCCESS;
    }
    perror("MAX limit reached");
    return 1;
    
}

int get_db(int key, void *output)
{

    if(db_info.status == DB_CLOSE) return FAILURE;

    for(int i = 0; i<db_info.rec_count; i++)
    {
        if(db_info.ndxArray[i].key == key)
        {
            int loc = db_info.ndxArray[i].loc;
            fseek(db_info.dbfile, loc+sizeof(int), SEEK_SET);
            fread(output, db_info.rec_size, 1, db_info.dbfile);
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}

int update_db(int key, void *new_rec)
{
    if(db_info.status == DB_CLOSE) return FAILURE;

    for(int i = 0; i < db_info.rec_count; i++)
    {
        if(db_info.ndxArray[i].key == key)
        {
            int loc = db_info.ndxArray[i].loc;

            fseek(db_info.dbfile, loc, SEEK_SET);
            fwrite(&key, sizeof(int), 1, db_info.dbfile);
            fwrite(new_rec, db_info.rec_size, 1, db_info.dbfile);
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}

int delete_db(int key)
{
    if(db_info.status == DB_CLOSE) return FAILURE;
    for(int i = 0; i<db_info.rec_count; i++)
    {
        if(db_info.ndxArray[i].key == key)
        {
            db_info.ndxArray[i].is_deleted = 1;
            db_info.ndxArray[i].old_key = key;
            db_info.ndxArray[i].key = -1;
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}

int undelete_db(int key)
{
    if(db_info.status == DB_CLOSE) return FAILURE;
    for(int i = 0; i<db_info.rec_count; i++)
    {
        if(db_info.ndxArray[i].is_deleted == 1 && db_info.ndxArray[i].old_key == key)
        {
            db_info.ndxArray[i].is_deleted = 0;
            db_info.ndxArray[i].key = db_info.ndxArray[i].old_key;
            db_info.ndxArray[i].old_key = -1;
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}

int close_db()
{
    if(db_info.status == DB_CLOSE) return FAILURE;
    FILE *fp1 = fopen(db_info.ndxname, "wb");
    if (fp1 != NULL)
    {
        fwrite(&db_info.rec_count, sizeof(db_info.rec_count), 1, fp1);
        fwrite(db_info.ndxArray, sizeof(struct Rec_ndx), db_info.rec_count, fp1);
        fclose(fp1); 
    }
    
    if (db_info.dbfile != NULL) fclose(db_info.dbfile);
    
    db_info.status = DB_CLOSE;
    db_info.dbfile = NULL;
    db_info.ndxfile = NULL;
    return SUCCESS;
}