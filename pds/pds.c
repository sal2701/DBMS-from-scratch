#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>

#include"pds.h"
#include"bst.h"

// Global Variable to store our structs.
// PDS stands for Personal Data Store.
struct PDS_RepoInfo repo_handle;
struct PDS_DBInfo db_info_handle;
struct PDS_DB_Master db_handle;
struct PDS_LinkFileInfo link_file_info_handle;
struct PDS_LinkInfo link_info_handle;
struct PDS_LinkedKeySet linked_key_set;

int processline( char * line);

int pds_create( char *repo_name );
void get_link_file_info( char *link_name );
void store_preorder( struct BST_Node * node );
int is_linked( char * entity_name, int key);
void display_linked_data( struct PDS_LinkedKeySet *linked_data );


////////////////////////////
// CREATE DATABASE SCHEMA //
///////////////////////////

// Function to read text description of the schema and create a struct db_info.
// This struct is written to a .db file and is later used to create the actual database.
int pds_create_schema(char *schema_file_name)
{
    char line[100];
    int status;
    // rb+ means open file for reading and writing ( The file must exist )
    // ab+ means open file for reading and writing ( Append is file exists )
    // wb+ overwrites if exists or creates a new one
    FILE* fptr = fopen(schema_file_name, "rb+");
    if( fptr == NULL ) perror(schema_file_name);

    char name[30];

    // [^\n] ensures that the '\n' value isn't scanned by the fcanf
    fscanf(fptr, "%s[^\n]", name);
    fgets(line, sizeof(line), fptr);
    strcat( name, ".db" );
    stpcpy( db_info_handle.db_name, name );

    db_info_handle.num_entities = 0;
    db_info_handle.num_relationships = 0;

    while (!feof(fptr))
    {
        fgets(line, sizeof(line), fptr);
        status = processline(line);

        if( status == PDS_CREATE_SCHEMA_FAILED )
        {
            return status;
        }
    }

    FILE* fptr1 = (FILE*) fopen( db_info_handle.db_name, "rb+");
    if( fptr1 == NULL ) perror(db_info_handle.db_name);

    fwrite( &db_info_handle, sizeof(struct PDS_DBInfo), 1, fptr1);
    status = PDS_SUCCESS;

    return status;
}


// Helper function to parse the text description of the schema
// The db_info struct is populated in this function
int processline( char * line)
{
    int status=0;
    char entry_type[30];
    sscanf(line, "%s", entry_type);

    if( strcmp( entry_type, "entity") == 0 )
    {
        char entity_name[30];
        struct PDS_EntityInfo * pds_entity_info = (struct PDS_EntityInfo *)malloc( sizeof(struct PDS_EntityInfo));

        sscanf(line, "%s %s %d", entry_type, pds_entity_info->entity_name, &pds_entity_info->entity_size);
        status = PDS_SUCCESS;

        db_info_handle.entities[ db_info_handle.num_entities ] = *pds_entity_info;
        db_info_handle.num_entities++;
    }
    else if( strcmp( entry_type, "relationship") == 0 )
    {
        struct PDS_LinkInfo * pds_link_info = (struct PDS_LinkInfo *)malloc( sizeof(struct PDS_LinkInfo) );

        sscanf(line, "%s %s %s %s", entry_type, pds_link_info->link_name, pds_link_info->pds_name1, pds_link_info->pds_name2);
        status = PDS_SUCCESS;

        db_info_handle.links[ db_info_handle.num_relationships ] = *pds_link_info;
        db_info_handle.num_relationships++;
    }
    else
    {
        status = PDS_CREATE_SCHEMA_FAILED;
    }
    
    return status;
}




//used to fetch the repo info from the PDS_DB_Master
//Used to fetch the actual RepoInfo from the Database
struct PDS_RepoInfo get_repo_handle( char * entity_name )
{
    int num_entities = db_handle.db_info.num_entities;

    for( int i=0; i<num_entities; i++)
    {
        if( strcmp( db_handle.entity_info[i].entity_name, entity_name ) == 0 )
        {
            return db_handle.entity_info[i];
        }
    }
    // return NULL;
}


//////////////////////////
// OPENING THE DATABASE //
/////////////////////////

// Function to read the schema stored in .db file in the form of db_info struct
// The info in db_info is used to create the actual database
// The actual database is in the struct PDS_DB_Master
int pds_db_open( char *db_name )
{
    char db_file_name[50];
    int status;

    if( db_handle.db_status == PDS_DB_OPEN ) return PDS_DB_ALREADY_OPEN;

    db_handle.db_status = PDS_DB_OPEN;
    strcpy( db_file_name, db_name);
    strcat( db_file_name, ".db");

    FILE* fptr = (FILE *) fopen( db_file_name, "rb+");

    fread( &db_handle.db_info, sizeof( struct PDS_DBInfo ), 1, fptr);


    int num_entities=0, num_relations=0;
    num_entities = db_handle.db_info.num_entities;
    num_relations = db_handle.db_info.num_relationships;

    for( int i=0; i<num_entities; i++)
    {
        struct PDS_EntityInfo pds_entity_info = db_handle.db_info.entities[i];

        // Here we read the entity info details 
        // Calling pds_open creates repo_info structs for each entity info
        // These repo_info structs are then stored in the pds_db_master
        repo_handle.repo_status = PDS_ENTITY_CLOSED;
        pds_open( pds_entity_info.entity_name, pds_entity_info.entity_size );
        db_handle.entity_info[i] = repo_handle;
    }

    for( int i=0; i<num_relations; i++)
    {

        // The links are stored as PDS_LinkFileInfo structs in the main database
        struct PDS_LinkInfo pds_link_info = db_handle.db_info.links[i];
        char link_file_name[50];

        strcpy( link_file_name, pds_link_info.link_name );
        strcat( link_file_name, ".lnk");
        strcpy( link_file_info_handle.link_name, pds_link_info.link_name );

        FILE* fptr = (FILE *) fopen( link_file_name, "rb+");
        link_file_info_handle.pds_link_fp = fptr;
        
        db_handle.rel_info[i] = link_file_info_handle;
    }

    return PDS_SUCCESS;
}


// This function does the main job of creating the repo info struct given an entity name
// This does the following jobs
// 1. Reads the .dat file for the data
// 2. Reads the .ndx file for the key-offset pairs and free list
// 3. Creates the BST from the key-offset pairs
int pds_open( char *repo_name, int rec_size )
{
    char repo_file[30];
    char ndx_file[30];

    if( repo_handle.repo_status == PDS_ENTITY_OPEN ) return PDS_ENTITY_ALREADY_OPEN;

    strcpy( repo_handle.entity_name, repo_name );

    strcpy( repo_file, repo_name);
    strcat( repo_file, ".dat");

    strcpy( ndx_file, repo_name);
    strcat( ndx_file, ".ndx");

    // rb+ means open file for reading and writing ( The file must exist )
    // ab+ means open file for reading and writing ( Append is file exists )
    // wb+ overwrites if exists or creates a new one
    repo_handle.pds_data_fp = (FILE*) fopen( repo_file, "rb+" );
    if( repo_handle.pds_data_fp == NULL ) perror(repo_file);

    repo_handle.pds_ndx_fp = (FILE*) fopen( ndx_file, "rb+");
    if( repo_handle.pds_ndx_fp == NULL ) perror(ndx_file);

    repo_handle.repo_status = PDS_ENTITY_OPEN;
    repo_handle.entity_size = rec_size;

    ///////////////////////////////////
    // Build BST from the Index File //
    ///////////////////////////////////

    int block_size = sizeof( struct PDS_NdxInfo );

    // SEEKSET puts pointer at beginning of file
    // SEEKEND puts pointer at end of file
    fseek( repo_handle.pds_ndx_fp, 0, SEEK_SET);

    int file_end = ftell( repo_handle.pds_ndx_fp);
    // Declare the root node
    repo_handle.pds_bst = NULL;
    bst_destroy( repo_handle.pds_bst );

    // The data in the root node
    struct PDS_NdxInfo *ndxentry = ( struct PDS_NdxInfo *) malloc( sizeof( struct PDS_NdxInfo ) );


    // Adding MAX_FREE integers as a placeholder for free list
    // This is done only once at the beginning, during the file creation
    if( file_end < MAX_FREE)
    {
        memset( repo_handle.free_list, -1, sizeof(repo_handle.free_list) );
    }
    else
    {
        fread( repo_handle.free_list, sizeof(int), MAX_FREE, repo_handle.pds_ndx_fp );
    }
    
    // Reading free list & the following ndx values
    fread( ndxentry, block_size, 1, repo_handle.pds_ndx_fp );

    // Add root node to bst
    bst_add_node( &repo_handle.pds_bst, ndxentry->key, ndxentry);

    while( !feof( repo_handle.pds_ndx_fp ) )
    {
        pds_load_ndx( &repo_handle );
    }

    // Closing the index file
    fclose( repo_handle.pds_ndx_fp );

    return PDS_SUCCESS;
}

// Helper function to read ndx file and create the BST
int pds_load_ndx( struct PDS_RepoInfo *repo_handle )
{
    int block_size = sizeof( struct PDS_NdxInfo );
    // This line does a lot. idk why. wasted 3 hours of my time.
    struct PDS_NdxInfo *ndxentry = ( struct PDS_NdxInfo *) malloc( sizeof( struct PDS_NdxInfo ) );
    ndxentry = ( struct PDS_NdxInfo *) malloc( sizeof( struct PDS_NdxInfo ) );

    fread( ndxentry, block_size, 1, repo_handle->pds_ndx_fp);

    if( bst_search( repo_handle->pds_bst, ndxentry->key) == NULL )
    {
        bst_add_node( &repo_handle->pds_bst, ndxentry->key, ndxentry);
    }
}



//////////////////////////
// DATABASE OPERATIONS ///
//////////////////////////

// Function to add/put data to the database
// This function does the following stuff
// 1. Checks for duplicate key
// 2. Checks free list for free spaces
// 3. Adds data to the database
int put_rec_by_key( char *entity_name, int key, void *rec )
{
    int offset, status, writesize;
    struct PDS_NdxInfo *ndx_entry;

    struct BST_Node index_node;

    repo_handle = get_repo_handle( entity_name );
    // go to end of file
    fseek( repo_handle.pds_data_fp, 0, SEEK_END);

    // get pointer to end of the file
    offset = ftell( repo_handle.pds_data_fp );

    // checking if key already present
    struct BST_Node * tmp = bst_search( repo_handle.pds_bst, key );

    // if key not present already
    if( tmp == NULL )
    {
        //checking free list for free spaces in between
        for( int i=0; i<MAX_FREE; i++)
        {
            if( repo_handle.free_list[i] != -1)
            {
                offset = repo_handle.free_list[i];
                repo_handle.free_list[i] = -1;
                fseek( repo_handle.pds_data_fp, offset, SEEK_SET );
                break;
            }
        }

        // The fwrite/fread function returns the total number of elements successfully
        // returned/read as a size_t object, which is an integral data type.
        // If this number differs from the nmemb parameter(here 1), it will show an error.
        // we use rec_size taken in the beginning while opening the PDS as we are using void *
        // we also write the key to data file before the record

        int* tmp = &key;
        fwrite( tmp, sizeof(int), 1, repo_handle.pds_data_fp);
        writesize = fwrite( rec, repo_handle.entity_size, 1, repo_handle.pds_data_fp );

        ndx_entry = ( struct PDS_NdxInfo *) malloc( sizeof( struct PDS_NdxInfo ) );

        ndx_entry->key = key;
        ndx_entry->offset = offset;

        status = bst_add_node( &repo_handle.pds_bst, key, ndx_entry);

        if( status!= BST_SUCCESS )
        {
            fprintf( stderr, "Unable to add index entry for key %d - E%d\n", key, status);
            free(ndx_entry);
            // Data has already gone in to the data file so we roll back by resetting the file pointer
            // SEEK_SET moves pointer to beginning of file and offsets from there
            fseek( repo_handle.pds_data_fp, offset, SEEK_SET );
            status = PDS_ADD_FAILED;
        }
    }
    else
    {
        // If key already present
        status = BST_DUP_KEY;
    }

    return status;
}

// This function uses the index to fetch/get the key-value pairs
// This function does the following
// 1. Searches BST for key
// 2. Checks free list to see if it was deleted before
// 3. Return key-value pair
int get_rec_by_ndx_key( char *entity_name, int key, void *rec )
{
    struct BST_Node * bst_node=NULL;
    struct PDS_NdxInfo * ndx_entry=NULL;

    int offset=0, status=0, readsize=0;

    repo_handle = get_repo_handle( entity_name );

    // Search the BST
    bst_node = bst_search( repo_handle.pds_bst, key);

    if( bst_node == NULL ) status = PDS_REC_NOT_FOUND;
    else
    {
        ndx_entry = ( struct PDS_NdxInfo * ) bst_node->data;
        offset = ndx_entry->offset;

        int deleted=0;
        for( int i=0; i<MAX_FREE; i++)
        {
            // check if the key was deleted
            if( repo_handle.free_list[i] == offset)
            {
                deleted=1;
                break;
            }
        }

        if( deleted == 0 )
        {
            fseek( repo_handle.pds_data_fp, offset, SEEK_SET );
            int tmp;
            fread( &tmp, sizeof(int), 1, repo_handle.pds_data_fp);
            readsize = fread( rec, repo_handle.entity_size, 1, repo_handle.pds_data_fp);
            status = PDS_SUCCESS;
        }
        else
        {
            status = PDS_REC_NOT_FOUND;
        }
    }
    
    return status;
}


// This function updates the already existing key-value pairs
// Returns fail if record doesn't exist
int update_by_key( char *entity_name, int key, void *newrec )
{
    struct BST_Node * bst_node=NULL;
    struct PDS_NdxInfo * ndx_entry=NULL;

    int offset=0, status=0, writesize=0;

    repo_handle = get_repo_handle( entity_name );
    bst_node = bst_search( repo_handle.pds_bst, key);

    if( bst_node == NULL ) status = PDS_MODIFY_FAILED;
    else
    {
        ndx_entry = ( struct PDS_NdxInfo * ) bst_node->data;
        offset = ndx_entry->offset;
        fseek( repo_handle.pds_data_fp, offset, SEEK_SET );
        // write key as well as record
        fwrite( &key, sizeof(int), 1, repo_handle.pds_data_fp);
        writesize = fwrite( newrec, repo_handle.entity_size, 1, repo_handle.pds_data_fp);
        status = PDS_SUCCESS;
    }

    return status;
}


int delete_by_key( char *entity_name, int key )
{
    struct BST_Node * bst_node=NULL;
    struct PDS_NdxInfo * ndx_entry=NULL;

    int offset=0, status=0, linked=0;

    repo_handle = get_repo_handle( entity_name );

    bst_node = bst_search( repo_handle.pds_bst, key);

    
    if( !is_linked(entity_name, key) )
    {
        if( bst_node == NULL ) status = PDS_DELETE_FAILED;
        else
        {
            status = bst_del_node( &repo_handle.pds_bst, key);
            ndx_entry = ( struct PDS_NdxInfo * ) bst_node->data;
            offset = ndx_entry->offset;

            for( int i=0; i<MAX_FREE; i++)
            {
                if( repo_handle.free_list[i] == -1 )
                {
                    repo_handle.free_list[i] = offset;
                    status = PDS_SUCCESS;
                    break;
                }
            }
        }
    }
    else
    {
        status = PDS_DELETE_FAILED;
    }

    return status;
}


int key_exists( char* entity_name, int key )
{
    repo_handle = get_repo_handle( entity_name );

    struct BST_Node * tmp = bst_search(repo_handle.pds_bst, key);

    if( tmp != NULL ) return 1;
    return 0;
}

int link_exists( int key, int linked_key )
{
    int link_count = linked_key_set.link_count;

    for( int i=0; i<link_count; i++)
    {
        if( linked_key == linked_key_set.linked_keys[i] )
        {
            return 1;
        }
    }

    return 0;
}

int link_data(char *link_name, int key, int linked_key)
{

    get_linked_data( link_name, key, &linked_key_set );
    int link_count = linked_key_set.link_count, status=1;
    char * entity_name_1 = link_info_handle.pds_name1;
    char * entity_name_2 = link_info_handle.pds_name2;

    if( !key_exists( entity_name_1, key) )
    {
        status = PDS_LINK_FAILED;
    }
    else if( !key_exists( entity_name_2, linked_key ) )
    {
        status = PDS_LINK_FAILED;
    }
    else if( link_exists( key, linked_key) )
    {
        status = PDS_LINK_FAILED;
    }
    else
    {
        fseek( link_file_info_handle.pds_link_fp, 0, SEEK_END );
        struct PDS_Link * pds_link = (struct PDS_Link *)malloc( sizeof(struct PDS_Link) );

        pds_link->key = key;
        pds_link->linked_key = linked_key;

        fwrite( pds_link, sizeof( struct PDS_Link), 1, link_file_info_handle.pds_link_fp);
        status = PDS_SUCCESS;
    }

    return status;

}

void get_link_meta( char *link_name )
{
    int num_relations = db_handle.db_info.num_relationships;

    for( int i=0; i<num_relations; i++)
    {
        if(!strcmp( db_handle.rel_info[i].link_name, link_name ))
        {
            link_file_info_handle = db_handle.rel_info[i];
            link_info_handle = db_handle.db_info.links[i];
        }
    }
}

// goes through the .lnk file and retrieves info for a given data_key
// it returns the filled PDS_LinkedKeySet struct
// should return error if data key doesn't exist
int get_linked_data( char *link_name, int data_key, struct PDS_LinkedKeySet *linked_data )
{
    int status = 1;
    get_link_meta( link_name );

    int num_relations = db_handle.db_info.num_relationships, count=0;
    FILE * fptr = link_file_info_handle.pds_link_fp;

    struct PDS_Link * pds_link = (struct PDS_Link *)malloc( sizeof(struct PDS_Link) );

    // debug();
    fseek( fptr, 0, SEEK_SET);

    while( !feof( fptr ) )
    {
        fread( pds_link, sizeof( struct PDS_Link ), 1, fptr );
        
        // printf("KEYS={%d %d}, EXPECTED=%d\n", pds_link->key, pds_link->linked_key, data_key);
        if( pds_link->key == data_key )
        {
            linked_data->linked_keys[count++] = pds_link->linked_key;
        }
        // pds_link = (struct PDS_Link *)malloc( sizeof(struct PDS_Link) );
        pds_link->key = -1;
        pds_link->linked_key = -1;
    }

    linked_data->key = data_key;
    linked_data->link_count = count;

    // printf("COUNT=%d", linked_data->link_count);

    return 0;
}





int is_linked( char * entity_name, int key)
{
    int num_relationships = db_handle.db_info.num_relationships;

    for( int i=0; i<num_relationships; i++)
    {
        struct PDS_LinkFileInfo pds_link_file_info = db_handle.rel_info[i];
        struct PDS_LinkedKeySet * pds_linked_key_set = ( struct PDS_LinkedKeySet *) malloc( sizeof( struct PDS_LinkedKeySet ) );

        get_linked_data( pds_link_file_info.link_name, key, pds_linked_key_set);

        if( pds_linked_key_set->link_count != 0 ) return 1;
    }
    return 0;
}

void display_linked_data( struct PDS_LinkedKeySet *linked_data )
{
    for( int i=0; i<linked_data->link_count; i++)
    {
        printf("%d  -  %d\n", linked_data->key, linked_data->linked_keys[i]);
    }
}

////////////////////////
//CLOSING THE DATABASE//
///////////////////////


//close the pds repos and link files
int pds_db_close()
{
    int num_entities = db_handle.db_info.num_entities;
    int num_relationships = db_handle.db_info.num_relationships;

    for( int i=0; i<num_entities; i++)
    {
        //call pds_close for each repo
        pds_close( db_handle.entity_info[i].entity_name );
    }

    for( int i=0; i<num_relationships; i++)
    {
        //close each link file directly
        fclose( db_handle.rel_info[i].pds_link_fp);
    }

    db_handle.db_status = PDS_DB_CLOSED;

    return PDS_SUCCESS;
}


//function to close the pds repos.
//the ndx file, which is stored as a BST is stored as the preorder traversal
//called for each repo by the pds_db_close function
int pds_close( char *entity_name )
{
    repo_handle = get_repo_handle( entity_name );

    char ndx_file[30];
    strcpy( ndx_file, repo_handle.entity_name );
    strcat( ndx_file, ".ndx" );
    //open in wb+ as we want to overwrite the old version
    repo_handle.pds_ndx_fp = (FILE*) fopen( ndx_file, "wb+");

    //store freelist first
    fwrite( repo_handle.free_list, sizeof(int), MAX_FREE, repo_handle.pds_ndx_fp);

    //storing tree in preorder
    store_preorder( repo_handle.pds_bst );

    //closing everything
    strcpy( repo_handle.entity_name, "");
    bst_destroy( repo_handle.pds_bst );
    fclose( repo_handle.pds_data_fp );
    fclose( repo_handle.pds_ndx_fp);
    repo_handle.pds_bst = NULL;
    repo_handle.repo_status = PDS_ENTITY_CLOSED;

    return PDS_SUCCESS;
}


//helper function to traverse and store the preorder traversal of the BST
void store_preorder( struct BST_Node * node )
{
    if( node == NULL ) return;

    struct PDS_NdxInfo * ndxentry = ( struct PDS_NdxInfo * ) (node->data);

    fwrite( ndxentry, sizeof( struct PDS_NdxInfo ), 1, repo_handle.pds_ndx_fp );
    fseek( repo_handle.pds_ndx_fp, sizeof( struct PDS_NdxInfo ), SEEK_CUR );

    store_preorder( node->left_child );
    store_preorder( node->right_child );
}

// rm scandemo.* && touch scandemo.ndx scandemo.dat && gcc -o pds_tester pds_tester.c bst.c pds.c ../4sess/contact.c && gcc -o contact_loader contact_loader.c bst.c contact.c pds.c
// ./contact_loader scandemo contact_dump.txt
// ./pds_tester testcase2.in