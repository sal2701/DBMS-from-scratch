mypds: student.dat student.ndx course.dat course.ndx enrollment.lnk
	gcc -o mypds pds/pds.c pds/bst.c app/app.c -g

student.dat:
	touch student.dat

student.ndx:
	touch student.ndx

course.dat:
	touch course.dat

course.ndx:
	touch course.ndx

enrollment.lnk:
	touch enrollment.lnk

clean:
	rm -f student.ndx student.dat course.ndx course.dat enrollment.lnk