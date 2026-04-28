/*This program implements a distributed pairing system using MPI.
The teacher collects student requests and pairs them, while the students request the teacher to assign them a partner.

The teacher has rank 0 and creates an array containing the ranks of all students who need to be paired. 
The teacher then iterates through this array, processing students two at a time and skipping every other student. 
If the last iteration reaches a single remaining student (an odd number of students), they will be paired with themselves. 
Otherwise, students are paired normally.

If a process has a rank greater than 0, it represents a student who wishes to be paired. 
In this case, the student sends their rank to the teacher, and the teacher receives and processes all student requests.
*/

#include <mpi.h>
#include <stdio.h>

#define TEACHER 0  // teacher's rank is always 0

//MPI initializations 
int main(int argc, char *argv[]) {  
    int rank, partner, numStudents;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numStudents);

//teacher collects student request for partner
    if (rank == TEACHER) {  
        int students[numStudents - 1];  // Stores student requests
        printf("Teacher: Time to pair up!\n");

        // Receiving student requests
        for (int i = 1; i < numStudents; i++) { //the teacher receieves requests and stores the student's ranks in the array
            MPI_Recv(&students[i - 1], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // Pairing students
        for (int i = 0; i < numStudents - 1; i += 2) {
            if (i == numStudents - 2 && numStudents % 2 == 0) {  
                // if there are odd numbers one student will be paired with themselves
                MPI_Send(&students[i], 1, MPI_INT, students[i], 0, MPI_COMM_WORLD);
            } else {  
                // pairing if there are even numbers
                MPI_Send(&students[i + 1], 1, MPI_INT, students[i], 0, MPI_COMM_WORLD);
                MPI_Send(&students[i], 1, MPI_INT, students[i + 1], 0, MPI_COMM_WORLD);
            }
        }
    } else {  // It is the student that wants to request a partner 
        MPI_Send(&rank, 1, MPI_INT, TEACHER, 0, MPI_COMM_WORLD);
        MPI_Recv(&partner, 1, MPI_INT, TEACHER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Student %d's partner is Student %d\n", rank, partner);
    }

    MPI_Finalize();
    return 0;
}
