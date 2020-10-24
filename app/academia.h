#ifndef ACADEMIA_H
#define ACADEMIA_H

// academia.h
// Size = 95 bytes
struct Student{
	int rollnumber;
	char student_name[30];
};

// Size = 38 bytes
struct Course{
	int courseid;
	char course_name[30];
};

int search_student( int rollnumber, struct Student *c );
int add_student( struct Student *c );

#define ACADEMIA_SUCCESS 0
#define ACADEMIA_FAILURE 1

#endif
