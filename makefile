c_files = pds/pds.c pds/bst.c app/app.c
db_files = student.ndx student.dat course.dat course.ndx enrollment.lnk academia.db

mypds: clean $(c_files) $(db_files)
	@gcc -o mypds pds/pds.c pds/bst.c app/app.c -g
	@printf "Created mypds executable.\nRun ./mypds\n"

student.dat:
	@touch student.dat
	@echo "Created student.dat"

student.ndx:
	@touch student.ndx
	@echo "Created student.ndx"

course.dat:
	@touch course.dat
	@echo "Created course.dat"

course.ndx:
	@touch course.ndx
	@echo "Created course.ndx"

enrollment.lnk:
	@touch enrollment.lnk
	@echo "Created enrollment.lnk"

clean:
	@echo "Deleted old files"
	@rm -f $(db_files) mypds