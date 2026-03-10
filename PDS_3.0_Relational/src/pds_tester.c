#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pds3.h"

typedef struct Hospital
{
	int hospital_id;
	char name[100];
	char address[200];
	char email[50];
} Hospital;

#define TREPORT(a1, a2)                        \
	do                                         \
	{                                          \
		printf("Status: %s - %s\n\n", a1, a2); \
		fflush(stdout);                        \
	} while (0)

void process_line(char *test_case);

// PDS function wrappers
int pds_create(char *tname1, char *tname2) {
	return db_create(tname1, tname2);
}

int pds_open(char *tname1, int rec_size1, char *tname2, int rec_size2) {
	return db_open(tname1, tname2, rec_size1, rec_size2);
}

int add_record(char *tname, int key, void *record) {
	return table_store(tname, key, record);
}

int search_record(char *tname, int key, void *record) {
	return table_get(tname, key, record);
}

int update_record(char *tname, int key, void *record) {
	return table_update(tname, key, record);
}

int delete_record(char *tname, int key) {
	return table_delete(tname, key);
}

int undelete_record(char *tname, int key) {
	return table_undelete(tname, key);
}

int pds_close() {
	return db_close();
}

int pds_create_rel(char *relname, char *primary, char *secondary) {
	return rel_create(relname, primary, secondary);
}

int pds_open_rel(char *relname) {
	return rel_open(relname);
}

int pds_store_rel(int key1, int key2) {
	return rel_store(key1, key2);
}

int pds_get_rel(void *rec, int key1) {
	return rel_get(rec, key1);
}

int pds_delete_rel(char *tname, int key)
{
	return rel_delete(tname, key);
}

int pds_close_rel() {
	return rel_close();
}

// Main
int main(int argc, char *argv[])
{
	FILE *cfptr;
	char test_case[500];

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s testcasefile\n", argv[0]);
		exit(1);
	}

	init(); //I ADDED THISSSSSS

	cfptr = (FILE *)fopen(argv[1], "r");
	if (!cfptr)
	{
		fprintf(stderr, "Error opening file %s\n", argv[1]);
		exit(1);
	}

	while (fgets(test_case, sizeof(test_case) - 1, cfptr))
	{
		if (!strcmp(test_case, "\n") || !strcmp(test_case, ""))
			continue;
		process_line(test_case);
	}

	fclose(cfptr);
	return 0;
}

void process_line(char *test_case)
{
	char command[30], tname1[30], tname2[30], relname[30];
	char info[1024];
	int status, expected_status, key1, key2;
	int rec_size = sizeof(Hospital);
	Hospital testHospital;

	// Extract the command
	sscanf(test_case, "%s", command);
	printf("Test case: %s", test_case);
	fflush(stdout);

	if (!strcmp(command, "CREATE"))
	{
		sscanf(test_case, "%s %s %s %d", command, tname1, tname2, &expected_status);
		status = pds_create(tname1, tname2);

		if (status == expected_status) TREPORT("PASS", "");
		else { sprintf(info, "pds_create returned status %d", status); TREPORT("FAIL", info); }
	}
	else if (!strcmp(command, "OPEN"))
	{
		sscanf(test_case, "%s %s %s %d", command, tname1, tname2, &expected_status);
		status = pds_open(tname1, rec_size, tname2, rec_size);

		if (status == expected_status) TREPORT("PASS", "");
		else { sprintf(info, "pds_open returned status %d", status); TREPORT("FAIL", info); }
	}
	else if (!strcmp(command, "STORE"))
	{
		char name[100], address[200], email[50];
		sscanf(test_case, "%s %s %d %d %s %s %s", command, tname1, &key1, &expected_status, name, address, email);

		testHospital.hospital_id = key1;
		strcpy(testHospital.name, name);
		strcpy(testHospital.address, address);
		strcpy(testHospital.email, email);

		status = add_record(tname1, key1, &testHospital);

		if (status == expected_status) TREPORT("PASS", "");
		else { sprintf(info, "add_record returned status %d", status); TREPORT("FAIL", info); }
	}
	else if (!strcmp(command, "SEARCH"))
	{
		sscanf(test_case, "%s %s %d %d", command, tname1, &key1, &expected_status);
		status = search_record(tname1, key1, &testHospital);

		if (status != expected_status)
		{
			sprintf(info, "search key: %d; Got status %d, expected %d", key1, status, expected_status);
			TREPORT("FAIL", info);
		}
		else if (expected_status == SUCCESS)
		{
			char exp_name[100], exp_addr[200], exp_email[50];
			sscanf(test_case, "%*s %*s %*d %*d %s %s %s", exp_name, exp_addr, exp_email);

			if (testHospital.hospital_id == key1 &&
				strcmp(testHospital.name, exp_name) == 0 &&
				strcmp(testHospital.address, exp_addr) == 0 &&
				strcmp(testHospital.email, exp_email) == 0)
			{
				TREPORT("PASS", "");
			}
			else
			{
				sprintf(info, "Data mismatch... Expected:{%d,%s,%s,%s} Got:{%d,%s,%s,%s}\n",
						key1, exp_name, exp_addr, exp_email,
						testHospital.hospital_id, testHospital.name, testHospital.address, testHospital.email);
				TREPORT("FAIL", info);
			}
		}
		else
		{
			TREPORT("PASS", "");
		}
	}
	else if (!strcmp(command, "UPDATE"))
	{
		char name[100], address[200], email[50];
		sscanf(test_case, "%s %s %d %d %s %s %s", command, tname1, &key1, &expected_status, name, address, email);

		testHospital.hospital_id = key1;
		strcpy(testHospital.name, name);
		strcpy(testHospital.address, address);
		strcpy(testHospital.email, email);

		status = update_record(tname1, key1, &testHospital);

		if (status == expected_status) TREPORT("PASS", "");
		else { sprintf(info, "update_record returned status %d", status); TREPORT("FAIL", info); }
	}
	else if (!strcmp(command, "DELETE"))
	{
		sscanf(test_case, "%s %s %d %d", command, tname1, &key1, &expected_status);
		status = delete_record(tname1, key1);

		if (status == expected_status) TREPORT("PASS", "");
		else { sprintf(info, "delete_record returned status %d", status); TREPORT("FAIL", info); }
	}
	else if (!strcmp(command, "UNDELETE"))
	{
		sscanf(test_case, "%s %s %d %d", command, tname1, &key1, &expected_status);
		status = undelete_record(tname1, key1);

		if (status == expected_status) TREPORT("PASS", "");
		else { sprintf(info, "undelete_record returned status %d", status); TREPORT("FAIL", info); }
	}
	else if (!strcmp(command, "CLOSE"))
	{
		sscanf(test_case, "%s %d", command, &expected_status);
		status = pds_close();

		if (status == expected_status) TREPORT("PASS", "");
		else { sprintf(info, "pds_close returned status %d", status); TREPORT("FAIL", info); }
	}
	else if (!strcmp(command, "REL_CREATE"))
	{
		sscanf(test_case, "%s %s %s %s %d", command, relname, tname1, tname2, &expected_status);
		status = pds_create_rel(relname, tname1, tname2);

		if (status == expected_status) TREPORT("PASS", "");
		else { sprintf(info, "create_rel returned status %d", status); TREPORT("FAIL", info); }
	}
	else if (!strcmp(command, "REL_OPEN"))
	{
		sscanf(test_case, "%s %s %d", command, relname, &expected_status);
		status = pds_open_rel(relname);

		if (status == expected_status) TREPORT("PASS", "");
		else { sprintf(info, "open_rel returned status %d", status); TREPORT("FAIL", info); }
	}
	else if (!strcmp(command, "REL_STORE"))
	{
		sscanf(test_case, "%s %d %d %d", command, &key1, &key2, &expected_status);
		status = pds_store_rel(key1, key2);

		if (status == expected_status) TREPORT("PASS", "");
		else { sprintf(info, "store_rel(%d,%d) returned status %d", key1, key2, status); TREPORT("FAIL", info); }
	}
	else if (!strcmp(command, "REL_SEARCH"))
	{
		sscanf(test_case, "%s %d %d", command, &key1, &expected_status);
		Hospital relHospital;
		status = pds_get_rel(&relHospital, key1);

		if (status != expected_status)
		{
			sprintf(info, "get_rel(%d) returned status %d, expected %d", key1, status, expected_status);
			TREPORT("FAIL", info);
		}
		else if (expected_status == SUCCESS)
		{
			int exp_id;
			char exp_name[100], exp_addr[200], exp_email[50];
			sscanf(test_case, "%*s %*d %*d %d %s %s %s", &exp_id, exp_name, exp_addr, exp_email);

			if (relHospital.hospital_id == exp_id &&
				strcmp(relHospital.name, exp_name) == 0 &&
				strcmp(relHospital.address, exp_addr) == 0 &&
				strcmp(relHospital.email, exp_email) == 0)
			{
				TREPORT("PASS", "");
			}
			else
			{
				sprintf(info, "Data mismatch. Expected:{%d,%s,%s,%s} Got:{%d,%s,%s,%s}",
						exp_id, exp_name, exp_addr, exp_email,
						relHospital.hospital_id, relHospital.name, relHospital.address, relHospital.email);
				TREPORT("FAIL", info);
			}
		}
		else
		{
			TREPORT("PASS", "");
		}
	}

	else if (!strcmp(command, "REL_DELETE"))
	{
		int table_idx;
		sscanf(test_case, "%s %d %d %d", command, &key1, &table_idx, &expected_status);

		char *target_table = (table_idx == 0) ? db_info.rinfo.primary_table_name : db_info.rinfo.rel_table_name;

		status = pds_delete_rel(target_table, key1);

		if (status == expected_status) 
		{
			TREPORT("PASS", "");
		}
		else 
		{ 
			sprintf(info, "pds_delete_rel(%s, %d) returned status %d, expected %d", target_table, key1, status, expected_status); 
			TREPORT("FAIL", info); 
		}
	}

	else if (!strcmp(command, "REL_CLOSE"))
	{
		sscanf(test_case, "%s %d", command, &expected_status);
		status = pds_close_rel();

		if (status == expected_status) TREPORT("PASS", "");
		else { sprintf(info, "close_rel returned status %d", status); TREPORT("FAIL", info); }
	}
}
