#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include"academia.h"
#include"../pds/pds.h"

void display_linked_data( struct PDS_LinkedKeySet *linked_data );

int input()
{
    int input = 0;
    printf(
            "Choose one of the following operations\n"
            "1 - ADD Student\n"
            "2 - ADD Course\n"
            "3 - UPDATE Student Name\n"
            "4 - UPDATE Course Name\n"
            "5 - LINK Student to a Course\n"
            "6 - SEARCH Student by ID\n"
            "7 - SEARCH Course by ID\n"
            "8 - RETRIEVE Student's Courses\n"
            "0 - Close & Exit\n"
    );

    scanf("%d", &input);
    return input;
}


int main()
{
    int exit = 0, status = 0;
    struct Course c;
    struct Student s;
    struct PDS_LinkedKeySet * l;

    status = pds_create_schema("academia.txt");
    status = pds_db_open("academia");

    printf("\nPersonal Data Store\n\n");

    while(!exit)
    {
        switch( input() )
        {
            // Add student
            case 1:
                printf("Enter Student ID & Name.");
                scanf("%d%s", &s.rollnumber, s.student_name);
                status = put_rec_by_key( "student", s.rollnumber, &s);
                printf("Status = %d\n", status);
                break;

            // Add course
            case 2:
                printf("Enter Course ID & Name.");
                scanf("%d%s", &c.courseid, c.course_name);
                status = put_rec_by_key( "course", c.courseid, &c);
                printf("Status = %d\n", status);
                break;
            
            // Modify student
            case 3:
                printf("Enter Student ID & Modified Name.");
                scanf("%d%s", &s.rollnumber, s.student_name);
                strcat( s.student_name, "-modified");
                status = update_by_key("student", s.rollnumber, &s);
                printf("Status = %d\n", status);
                break;

            // Modify course
            case 4:
                printf("Enter Course ID & Modified Name.");
                scanf("%d%s", &c.courseid, c.course_name);
                strcat( c.course_name, "-modified");
                status = put_rec_by_key("course", c.courseid, &c);
                printf("Status = %d\n", status);
                break;

            // Link Student & Course
            case 5:
                printf("Enter Student ID & Course ID");
                int key1, key2;
                scanf("%d%d", &key1, &key2);
                status = link_data("enrollment", key1, key2);
                printf("Status = %d\n", status);
                break;

            // Search student by ID
            case 6:
                printf("Enter Student ID.");
                scanf("%d", &s.rollnumber);
                status = get_rec_by_ndx_key("student", s.rollnumber, &s);
                printf("%d %s\n", s.rollnumber, s.student_name);
                printf("Status = %d\n", status);
                break;

            // Search course by ID            
            case 7:
                printf("Enter Course ID.");
                scanf("%d", &c.courseid);
                status = get_rec_by_ndx_key("student", c.courseid, &c);
                printf("%d %s\n", c.courseid, c.course_name);
                printf("Status = %d\n", status);
                break;

            // Retrieve student's courses
            case 8:
                l = (struct PDS_LinkedKeySet* ) malloc( sizeof( struct PDS_LinkedKeySet ));
                printf("Enter Student ID.");
                scanf("%d", &s.rollnumber);
                status = get_linked_data( "enrollment", s.rollnumber, l);
                printf("Status = %d\n", status);
                display_linked_data(l);
                break;

            // Exit
            case 0:
                printf("EXIT");
                exit = 1;
                break;
        }
    }
    return 0;
}