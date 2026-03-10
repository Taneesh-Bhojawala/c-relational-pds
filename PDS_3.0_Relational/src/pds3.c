#include <stdio.h>
#include <stdlib.h>
#include "pds3.h"
#include <string.h>

struct Db_Info db_info;

void init()
{
    db_info.num_table = 0;
    db_info.db_status = DB_CLOSE;
    db_info.rel_status = DB_CLOSE;
    db_info.rinfo.rel_file = NULL;
    strcpy(db_info.rinfo.relname, "");
    strcpy(db_info.rinfo.rel_table_name, "");

    for(int  i = 0; i<2; i++)
    {
        db_info.tinfo[i].tfile = NULL;
        db_info.tinfo[i].ndxfile = NULL;
        db_info.tinfo[i].rec_count = 0;
        db_info.tinfo[i].rec_size = 0;
        strcpy(db_info.tinfo[i].tablename, "");
        strcpy(db_info.tinfo[i].ndxname, "");
    }
}

int db_create(char *tname1, char *tname2)
{
    int r1 = table_create(tname1);
    int r2 = table_create(tname2);
    if(r1 == SUCCESS && r2 == SUCCESS) return SUCCESS;
    else return FAILURE;
}

struct TableInfo * get_table_info(char *tname)
{
    char tempname[100];
    strcpy(tempname, tname);
    strcat(tempname, ".dat");
    for(int i = 0; i < db_info.num_table; i++)
    {
        if(strcmp(db_info.tinfo[i].tablename, tempname) == 0)
        {
            return &db_info.tinfo[i];
        }
    }
    return NULL;
}

int table_create(char *tname)
{
    char tablefilename[100];
    char ndxfilename[100];
    strcpy(tablefilename, tname);
    strcat(tablefilename, ".dat");
    strcpy(ndxfilename, tname);
    strcat(ndxfilename, ".ndx");

    FILE *fp = fopen(tablefilename, "wb");
    if(fp == NULL)
    {
        perror("Error creating table file!\n");
        return FAILURE;
    }
    fclose(fp);

    FILE *fp1 = fopen(ndxfilename, "wb");
    if(fp1 == NULL)
     {
        perror("Error creating table index file!\n");
        return FAILURE;
    }
    int ini_count = 0;
    fwrite(&ini_count, sizeof(int), 1, fp1);
    fclose(fp1);
    return SUCCESS;
}

int db_open(char *tname1, char *tname2, int rec_size1, int rec_size2)
{
    if(db_info.db_status == DB_OPEN)
    {
        printf("Database is already open\n");
        return FAILURE;
    }
    init();
    if(table_open(tname1, rec_size1) != SUCCESS) return FAILURE;
    if(table_open(tname2, rec_size2) != SUCCESS) return FAILURE;

    db_info.db_status = DB_OPEN;
    return SUCCESS;
}

int table_open(char *tname, int rec_size)
{
    if(db_info.num_table >= 2) return FAILURE;

    struct TableInfo *table = &db_info.tinfo[db_info.num_table];
    char tablefilename[100];
    char ndxfilename[100];
    strcpy(tablefilename, tname);
    strcat(tablefilename, ".dat");
    strcpy(ndxfilename, tname);
    strcat(ndxfilename, ".ndx");

    strcpy(table->tablename, tablefilename);
    strcpy(table->ndxname, ndxfilename);
    table->rec_size = rec_size;

    table->tfile = fopen(tablefilename, "rb+");
    if(table->tfile == NULL)
    {
        db_info.num_table = 0;
        return FAILURE;
    }

    FILE *fp = fopen(ndxfilename, "rb+");
    if(fp == NULL)
    {
        db_info.num_table = 0;
        return FAILURE;
    }

    fseek(fp, 0, SEEK_SET);
    fread(&table->rec_count, sizeof(int), 1, fp);
    fread(table->ndxArray, sizeof(struct Rec_ndx), table->rec_count, fp);
    fclose(fp);
    db_info.num_table++;
    return SUCCESS;
}

int table_store(char *tname, int key, void *c)
{
    if(db_info.db_status == DB_CLOSE) return FAILURE;
    struct TableInfo *table = get_table_info(tname);
    if(table == NULL) return FAILURE;

    if(table->rec_count<MAX)
    {
        fseek(table->tfile, 0, SEEK_END);
        int loc = (int)ftell(table->tfile);
        fwrite(&key, sizeof(int), 1, table->tfile);
        fwrite(c, table->rec_size, 1, table->tfile);

        struct Rec_ndx indx;
        indx.key = key;
        indx.loc = loc;
        indx.is_deleted = 0;
        indx.old_key = -1;

        table->ndxArray[table->rec_count++] = indx;
        return SUCCESS;
    }
    printf("MAX limit reached this table.\n");
    return FAILURE;
}

int table_get(char *tname, int key, void *output)
{
    if(db_info.db_status == DB_CLOSE) return FAILURE;

    struct TableInfo *table = get_table_info(tname);
    if(table == NULL) return FAILURE;

    for(int i = 0; i<table->rec_count; i++)
    {
        if(table->ndxArray[i].key == key && table->ndxArray[i].is_deleted == 0)
        {
            int loc = table->ndxArray[i].loc;

            fseek(table->tfile, loc+sizeof(int), SEEK_SET);
            fread(output, table->rec_size, 1, table->tfile);
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}

int table_update(char *tname, int key, void *new_rec)
{
    if(db_info.db_status == DB_CLOSE) return FAILURE;

    struct TableInfo *table = get_table_info(tname);
    if(table == NULL) return FAILURE;

    for(int i = 0; i<table->rec_count; i++)
    {
        if(table->ndxArray[i].key == key && table->ndxArray[i].is_deleted == 0)
        {
            int loc = table->ndxArray[i].loc;

            fseek(table->tfile, loc, SEEK_SET);
            fwrite(&key, sizeof(int), 1, table->tfile);
            fwrite(new_rec, table->rec_size, 1, table->tfile);
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}

int table_delete(char *tname, int key)
{
    if(db_info.db_status == DB_CLOSE) return FAILURE;

    struct TableInfo *table = get_table_info(tname);
    if(table == NULL) return FAILURE;

    for(int i = 0; i<table->rec_count; i++)
    {
        if(table->ndxArray[i].key == key && table->ndxArray[i].is_deleted == 0)
        {
            table->ndxArray[i].is_deleted = 1;
            table->ndxArray[i].old_key = key;
            table->ndxArray[i].key = -1;
            rel_delete(tname, key);
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}

int table_undelete(char *tname, int key)
{
    if(db_info.db_status == DB_CLOSE) return FAILURE;

    struct TableInfo *table = get_table_info(tname);
    if(table == NULL) return FAILURE;

    for(int i = 0; i < table->rec_count; i++)
    {
        if(table->ndxArray[i].is_deleted == 1 && table->ndxArray[i].old_key == key)
        {
            table->ndxArray[i].is_deleted = 0;
            table->ndxArray[i].key = table->ndxArray[i].old_key;
            table->ndxArray[i].old_key = -1;
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}

int table_close(struct TableInfo *table)
{
    if(table == NULL) return FAILURE;

    FILE *fp1 = fopen(table->ndxname, "wb");
    if(fp1 != NULL)
    {
        fwrite(&table->rec_count, sizeof(int), 1, fp1);
        fwrite(table->ndxArray, sizeof(struct Rec_ndx), table->rec_count, fp1);
        fclose(fp1); 
    }
    
    if(table->tfile != NULL)
    {
        fclose(table->tfile);
        table->tfile = NULL;
    }
    
    return SUCCESS;
}

int db_close()
{
    if(db_info.db_status == DB_CLOSE) return FAILURE;

    for(int i = 0; i < db_info.num_table; i++)
    {
        table_close(&db_info.tinfo[i]);
    }

    if(db_info.rel_status == DB_OPEN) rel_close();

    db_info.db_status = DB_CLOSE;
    db_info.num_table = 0;
    
    return SUCCESS;
}

int rel_create(char *relname, char *tname1, char *tname2)
{
    char rel_filename[200];
    strcpy(rel_filename, relname);
    strcat(rel_filename, ".rel");

    FILE *fp = fopen(rel_filename, "wb");
    if(fp == NULL) return FAILURE;

    char table1[100], table2[100];
    strcpy(table1, tname1);
    strcpy(table2, tname2);
    fwrite(table1, 100, 1, fp);
    fwrite(table2, 100, 1, fp);
    fclose(fp);
    
    return SUCCESS;
}

int rel_open(char *relname)
{
    if(db_info.rel_status == DB_OPEN) return FAILURE;

    char rel_filename[200];
    strcpy(rel_filename, relname);
    strcat(rel_filename, ".rel");

    FILE *fp = fopen(rel_filename, "rb+");
    if(fp == NULL) return FAILURE;

    char table1[100];
    char table2[100];
    fread(table1, 100, 1, fp);
    fread(table2, 100, 1, fp);

    if (get_table_info(table1) == NULL || get_table_info(table2) == NULL)
    {
        fclose(fp);
        return FAILURE;
    }

    db_info.rinfo.rel_file = fp;
    strcpy(db_info.rinfo.relname, rel_filename);

    strcpy(db_info.rinfo.primary_table_name, table1);
    strcpy(db_info.rinfo.rel_table_name, table2); 
    
    db_info.rel_status = DB_OPEN;

    return SUCCESS;
}

int rel_store(int primary_key, int rel_key)
{

    if(db_info.rel_status == DB_CLOSE) return FAILURE;
    char temp[1024];
    if (table_get(db_info.rinfo.primary_table_name, primary_key, temp) != SUCCESS) return FAILURE;
    if (table_get(db_info.rinfo.rel_table_name, rel_key, temp) != SUCCESS) return FAILURE;


    struct Rel_rec rr;
    rr.primary_key = primary_key;
    rr.rel_key = rel_key;
    rr.is_deleted = 0;

    fseek(db_info.rinfo.rel_file, 0, SEEK_END);
    fwrite(&rr, sizeof(struct Rel_rec), 1, db_info.rinfo.rel_file);
    return SUCCESS;
}

int rel_get(void *rel_rec, int search_key)
{
    if(db_info.rel_status == DB_CLOSE) return FAILURE;

    struct Rel_rec rr;
    fseek(db_info.rinfo.rel_file, sizeof(db_info.rinfo.primary_table_name)*2, SEEK_SET);

    while(fread(&rr, sizeof(struct Rel_rec), 1, db_info.rinfo.rel_file) == 1)
    {
        if(rr.primary_key == search_key && rr.is_deleted == 0)
        {
            if(table_get(db_info.rinfo.rel_table_name, rr.rel_key, rel_rec) == SUCCESS)
            {
                return SUCCESS;
            }
            else
            {
                continue;
            }
        }
    }
    return REC_NOT_FOUND;
}

int rel_delete(char *tname, int search_key)
{
    if(db_info.rel_status == DB_CLOSE) return FAILURE;

    int is_primary = 0, is_related = 0;
    if(strcmp(tname, db_info.rinfo.primary_table_name) == 0) is_primary = 1;
    if(strcmp(tname, db_info.rinfo.rel_table_name) == 0) is_related = 1;

    struct Rel_rec rr;
    fseek(db_info.rinfo.rel_file, sizeof(db_info.rinfo.primary_table_name)*2, SEEK_SET);
    
    while(fread(&rr, sizeof(struct Rel_rec), 1, db_info.rinfo.rel_file) == 1)
    {
        if(is_primary && rr.primary_key == search_key && rr.is_deleted == 0)
        {
            rr.is_deleted = 1;
            fseek(db_info.rinfo.rel_file, -sizeof(struct Rel_rec), SEEK_CUR);
            fwrite(&rr, sizeof(struct Rel_rec), 1, db_info.rinfo.rel_file);
            return SUCCESS;
        }
        else if(is_related && rr.rel_key == search_key && rr.is_deleted == 0)
        {
            rr.is_deleted = 1;
            fseek(db_info.rinfo.rel_file, -sizeof(struct Rel_rec), SEEK_CUR);
            fwrite(&rr, sizeof(struct Rel_rec), 1, db_info.rinfo.rel_file);
            return SUCCESS;
        }
    }
    return FAILURE;
}

int rel_close()
{
    if(db_info.rel_status == DB_CLOSE) return FAILURE;

    fclose(db_info.rinfo.rel_file);
    db_info.rel_status = DB_CLOSE;
    return SUCCESS;
}
