## Personal Data Store
An Entity-Relationship DBMS from scratch. The whole application has been written in C. All entities & links are stored in the form of structs. All data is handled in simple binary files. Majority operations are done using proper file handling.

### Run
- Compile the files ```make```
- Run the executable ```./mypds```
- Delete files later ```make clean```
### Features
- Create Database from schema in txt.
- Create Custom entities & relationships.
- Handle Multiple Databases.
- Basic Operations like ADD, UPDATE, DELETE, MODIFY.
- Create LINKS between any 2 entities.
- All data, database info, schema are stored in the form of files.

### Structure
```
- app
  - app.c : main driver file
  - academia.h : file with Student & Course structs

- pds
  - bst.h : bst header file
  - bst.c : bst function implementations
  - pds.h : pds header file
  - pds.c : complete pds implementation

- academia.txt : database schema file
- makefile : for the make tool
```
