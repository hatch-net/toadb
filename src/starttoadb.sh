#! /bin/sh
# toadb is started with service backend.
 rm  /dev/shm/sem.client_share_*
 ./toadb-0-01 -M 2

# valgrind --tool=memcheck --log-file=log.txt --leak-check=yes ./toadb-0-01 -M 1
