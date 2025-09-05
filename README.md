# VersionControl
Creating a version control system inspired by Git including implementing all the data structures by hand

# How to run this file

Following are the implemented commands and their syntax for using this file -

1. READ file -> READ <filename>
2. Create a new file -> CREATE <filename>
3. Insert/Append content in a file -> INSERT <filename> <content>
4. Update content in a file (deletes previous content) -> UPDATE <filename> <content>
5. Snapshot active version of a file (make it immutable) -> SNAPSHOT <filename> <message>
6. Rollback to a previous version of a file (if no version-id given goes back to the last version) -> ROLLBACK <filename> <version-id>
7. Print history of a file's versions -> HISTORY <filename>
8. Delete a file -> DELETE <filename>
9. Print files in order of their modification time (latest to first) -> RECENT_FILES <number>
10. Prints the files with most number of versions (max to min) -> BIGGEST_TREES <number>
11. End the program -> STOP
12. To see the details for each command -> HELP
