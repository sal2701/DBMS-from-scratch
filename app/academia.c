#include<stdio.h>
#include<stdlib.h>

#include "../pds/"
#include "academia.h"

void print_student( struct Student *c )
{
	printf("%d,%s,%s,%s\n", c->rollnumber,c->student_name,c->address,c->date_of_birth);
}

// Use get_rec_by_key function to retrieve academia
int search_student( int rollnumber, struct Student *c )
{
	int status;

	status = get_rec_by_ndx_key( "student", rollnumber, c );

	if( status == PDS_SUCCESS )
		status = ACADEMIA_SUCCESS;
	else
		status = ACADEMIA_FAILURE;

	return status;
}

// Add the given academia into the repository by calling put_rec_by_key
int add_student( struct Student *c )
{
	int status;

	status = put_rec_by_key( "student",c->rollnumber, c );

	if( status != PDS_SUCCESS ){
		fprintf(stderr, "Unable to add student with key %d. Error %d", c->rollnumber, status );
		status=ACADEMIA_FAILURE;
	}
	else
		status=ACADEMIA_SUCCESS;

	return status;
}

