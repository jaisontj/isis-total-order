NAME: JAISON JOHN TITUS

RUNNING PART 1:


- Run "make build" inside the part1/src/ directory will build the binary inside the same directory
- You can run the project using the command "prj1_tm -p port -h hostfile -c count"
- Helpers:
- "make clean" to remove the binary and other text files
- "make generate" to generate a "hostnames.txt" file with 4 ccis machines (030-034)


RUNNING PART 2:


- Run "make build" inside the part2/src/ directory will build the binary inside the same directory
- You can run the project using the command "prj1_ds -p port -h hostfile -c count -s X"
- Helpers: *same as above*

OrderedMessages:
These are printed on the console and also a file named "Process-<processid>.txt" are generated.
These skip the "<process-id>: " portion of the printed message. Hence, running "md5 Process-*.txt"
can tell you if all the processes printed the same order.

Testing:
- Both also accept flags
- "-d 2" simulates 2 second delays for receiving messages on this process
- "-l" simulates random dropping of received message on this process
- "-v" can take "verbose" "debug" or "info" as arguments and prints logs.

