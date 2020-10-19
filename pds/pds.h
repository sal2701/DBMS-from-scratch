#ifndef PDS_H
#define PDS_H

// Error codes
// Renamed REPO with ENTITY
#define PDS_SUCCESS 0
#define PDS_FILE_ERROR 1
#define PDS_ADD_FAILED 2
#define PDS_REC_NOT_FOUND 3
#define PDS_MODIFY_FAILED 4
#define PDS_DELETE_FAILED 5
#define PDS_UNDELETE_FAILED 6
#define PDS_ENTITY_ALREADY_OPEN 12
#define PDS_NDX_SAVE_FAILED 13
#define PDS_ENTITY_NOT_OPEN 14

// New codes for Database handling

#define PDS_DB_ALREADY_OPEN 112
#define PDS_DB_CLOSED 111
#define PDS_DB_OPEN 110
#define MAX_ENTITY 5
#define MAX_RELATIONSHIPS 2

// New codes for implementing entity links
#define PDS_LINK_FAILED 7

// Maximumn of delete entries info
#define MAX_FREE 100

// Maximum number links for each key
#define MAX_LINKS 100

// Repository status values
#define PDS_ENTITY_OPEN 10
#define PDS_ENTITY_CLOSED 11

// For Create Schema
#define PDS_CREATE_SCHEMA_FAILED 15

// PDS_EntityInfo: Persistent data structure for holding entity meta data
// These structures are stored/read in/from .db file using fwrite/fread
// This will be stored as part of PDS_DB
struct PDS_EntityInfo{
	char entity_name[30];
	int entity_size; // For fixed length records
};

// PDS_LinkInfo: Contains information pertaining to all relationships in database
// These structures are stored/read in/from .db file using fwrite/fread
// This will be stored as part of PDS_DB
struct PDS_LinkInfo{
	char link_name[30];
	char pds_name1[30];
	char pds_name2[30];
};

// PDS_DBInfo: Persistent data structure for holding database meta data
// This whole data structure must be loaded from a file with .db extension
// If db_name is "demo" then the file will be named "demo.db"
// Created by pds_create_schema
// fread(&dbinfo,sizeof(struct PDS_DBInfo), 1, dbschemafile_fp);
// fwrite(&dbinfo,sizeof(struct PDS_DBInfo), 1, dbschemafile_fp);

struct PDS_DBInfo{
	char db_name[30];
	int num_entities;
	struct PDS_EntityInfo entities[MAX_ENTITY];
	int num_relationships;
	struct PDS_LinkInfo links[MAX_RELATIONSHIPS];
};

// PDS_NdxInfo: Persistent data structure for holding index
// These entries are stored in a file with .ndx extension
// Example: if entity name is person, then index file name is person.ndx
struct PDS_NdxInfo{
	int key;
	int offset;
};

// PDS_Link: Persistent data structure for holding links between entities
// These entries are stored in a file with .lnk extension
// Example: if link name is student_course, then index file name is student_course.lnk
// In this example, key will correspond to student key and linked_key will correspond to course key
struct PDS_Link{
	int key;
	int linked_key;
};

// PDS_RepoInfo: In-memory data structure for holding information pertaining to an entity
// Data into this is copied from PDS_EntityInfo
struct PDS_RepoInfo{
	char entity_name[30];
	FILE *pds_data_fp;
	FILE *pds_ndx_fp;
	int repo_status; 
	int entity_size; // For fixed length records
	int free_list[MAX_FREE];
	struct BST_Node *pds_bst;
};

// PDS_LinkFileInfo: In-memory data structure for holding information pertaining to a link
// Data into this is copied from PDS_LinkInfo
struct PDS_LinkFileInfo{
	char link_name[30];
	FILE *pds_link_fp;
	int link_status; 
	int free_list[MAX_FREE];
};

// PDS_DB_Master
// Contains the full information pertaining to the entire database
// This structure is to be populated during pds_db_open call
// This is populated in pds_db_open
struct PDS_DB_Master{
	int db_status;
	struct PDS_DBInfo db_info; // Populate this by reading from .db file
	struct PDS_RepoInfo entity_info[MAX_ENTITY]; // Populate this by reading from .db file
	struct PDS_LinkFileInfo rel_info[MAX_RELATIONSHIPS];// Populate this by reading from .db file
};
	
struct PDS_LinkedKeySet{
	int key;
	int link_count;
	int linked_keys[MAX_LINKS];
};

// Global variable for Master 
// Populated by pds_db_open
extern struct PDS_DB_Master db_handle;

// P00) pds_create_schema
// This function helps create the .db file with entity and relationship information
// The schema information is present in text file with the following format shown in example
// First line: name of database 			
// Example:				academia
// Entry for entity with name and size 			
// Example:				entity student 50
// Entry for entity with name and size 			
// Example:				entity course 40
// Entry for relationship with relationship name, entiy#1 name, enity#2 name
// Example:				relationship enrollment student course
// This function parses the above lines and stores in PDS_DB_Info 
// After parsing all the lines, the entirey PDS_DB_Info is to be stored in the .db file
// Example file name:			academia.db
//
// THIS IS A STAND-ALONE FUNCTION
// Create an executable that takes schema_file_name from command line and calls pds_create_schema
int pds_create_schema (char *schema_file_name);

// P01) pds_db_open:
// Open the database info file. The file is named with .db extension.
// Example: If db_name is "bank", then file name is "bank.db"
// This should populate the PDS_DB_Master global variable named db_handle;
int pds_db_open( char *db_name );

// P02) pds_open
// Open the data file and index file in rb+ mode
// Update the fields of PDS_RepoInfo appropriately
// Build BST and store in pds_bst by reading index entries from the index file
// Close only the index file
int pds_open( char *repo_name, int rec_size );

// P03) pds_load_ndx
// Internal function used by pds_open to read index entries into BST
int pds_load_ndx( struct PDS_RepoInfo *repo_handle );

// P04) put_rec_by_key
// Retrieve the repo_handle from PDS_DB_Master corresponding to given entity_name
// Create an index entry with the current data file location using ftell
// Add index entry to BST using offset returned by ftell
// Check if there are any deleted location available in the free list
// If available, seek to the free location, other seek to the end of the file
// 1. Write the key at the current data file location
// 2. Write the record after writing the key
int put_rec_by_key( char *entity_name, int key, void *rec );

// P05) get_rec_by_key
// Retrieve the repo_handle from PDS_DB_Master corresponding to given entity_name
// Search for index entry in BST
// Seek to the file location based on offset in index entry
// 1. Read the key at the current file location 
// 2. Read the record after reading the key
int get_rec_by_ndx_key( char *entity_name, int key, void *rec );

// P06) get_rec_by_non_ndx_key
// Retrieve the repo_handle from PDS_DB_Master corresponding to given entity_name
// Search based on a key field on which an index 
// does not exist. This function actually does a full table scan 
// by reading the data file until the desired record is found.
// The io_count is an output parameter to indicate the number of records
// that had to be read from the data file for finding the desired record
int get_rec_by_non_ndx_key( 
char *entity_name, /* The entity file from which data is to be read */
void *key,  			/* The search key */
void *rec,  			/* The output record */
int (*matcher)(void *rec, void *key), /*Function pointer for matching*/
int *io_count  		/* Count of the number of records read */
); 

// P07) update
// Retrieve the repo_handle from PDS_DB_Master corresponding to given entity_name
// Search for index entry in BST
// Seek to the file location based on offset in index entry
// Overwrite the existing record with the given record
// In case of any error, return PDS_MODIFY_FAILED
int update_by_key( char *entity_name, int key, void *newrec );

// P08) pds_delete
// Retrieve the repo_handle from PDS_DB_Master corresponding to given entity_name
// Search for index entry in BST
// store the offset value in to free list
int delete_by_key( char *entity_name, int key );

// P09) link_objects
// Retrieve the link_info from PDS_DB_Master corresponding to given link_name
// Adds a link entry to the link file establishing link between two given keys
// Example: link_data("student_course", 12, 15);
// Give error key1 does not exist or key2 does not 
// exist or <key, linke_key> combination already exists
int link_data(char *link_name, int key, int linked_key);

// P10) getLinkedData
// Returns the linked data for a given key
// Example: Retrieve all courses (linked_data) done by a given student (data_key)
int get_linked_data( char *link_name, int data_key, struct PDS_LinkedKeySet *linked_data );

// P11) pds_close
// Retrieve the repo_handle from PDS_DB_Master corresponding to given entity_name
// Open the index file in wb mode (write mode, not append mode)
// Unload the BST into the index file by traversing it in PRE-ORDER (overwrite the entire index file)
// Free the BST by call bst_destroy()
// Close the index file and data file
int pds_close( char *entity_name );

// P12) pds_db_close
// Close all the entity files by calling pds_close
// Close all link files
int pds_db_close();

#endif
