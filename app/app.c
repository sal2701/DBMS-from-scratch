#include<stdio.h>
#include<string.h>

#include"academia.h"
#include"../pds/pds.h"


int input()
{
    int input = 0;
    printf("Choose one of the following operations\n1 - Add Student Data\n2 - Add Course Data\n3 - Modify Student Name\n4 - Modify Course Name\n5 - Enroll a Student to a Course\n6 - Retrieve a Student's Courses\n7 - View Database\n0 - Close & Exit\n");
    scanf("%d", &input);
    return input;
}


int main()
{
    int exit = 0, status = 0;
    struct Course c;
    struct Student s;

    status = pds_create_schema("academia.txt");
    status = pds_db_open("academia");

    printf("\nPersonal Data Store\n\n");

    while(!exit)
    {
        switch( input() )
        {
            case 1:
                printf("Enter Student ID & Name.");
                scanf("%d%s", &s.rollnumber, s.student_name);
                status = put_rec_by_key( "student", s.rollnumber, &s);
                printf("Status = %d\n", status);
                break;

            case 2:
                printf("Enter Course ID & Name.");
                scanf("%d%s", &c.courseid, c.course_name);
                status = put_rec_by_key( "course", c.courseid, &c);
                printf("Status = %d\n", status);
                break;

            case 3:
                printf("Enter Student ID & Modified Name.");
                scanf("%d%s", &s.rollnumber, s.student_name);
                strcat( s.student_name, "-modified");
                status = update_by_key("student", s.rollnumber, &s);
                printf("Status = %d\n", status);
                break;

            case 4:
                printf("Enter Course ID & Modified Name.");
                scanf("%d%s", &c.courseid, c.course_name);
                strcat( c.course_name, "-modified");
                status = put_rec_by_key("course", c.courseid, &c);
                printf("Status = %d\n", status);
                break;

            case 5:
                printf("Enter Student ID & Course ID");
                int key1, key2;
                scanf("%d%d", &key1, &key2);
                status = link_data("enrollment", key1, key2);
                printf("Status = %d\n", status);
                break;

            case 6:
            case 7:
            case 8:
            case 0:
                printf("EXIT");
                exit = 1;
                break;
        }
    }
    return 0;
}