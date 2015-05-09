// Copyright 2015 Dante Fan, Heetae Kim, and Saddem Alsudais
#include <mpi.h>

#include <iostream>
#include <string>

extern std::string serial_main(int argc, char* argv[]);
extern std::string parallel_main(int argc, char* argv[]);

int main(int argc, char *argv[]) {
    int numprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double t1, t2;
    t1 = MPI_Wtime();
    std::string szParallel = parallel_main(argc, argv);
    t2 = MPI_Wtime();

    if (rank == 0) {
        std::cout << "* Parallel sequence finding took " << t2 - t1 << " sec." << std::endl;
        std::cout << "* Position: " << szParallel << std::endl;
        
        // t1 = MPI_Wtime();
        // std::string szSerial = serial_main(argc, argv);
        // t2 = MPI_Wtime();
        
        // std::cout << "* Serial sequence finding took " << t2 - t1 << " sec." << std::endl;
        
        // if (szParallel == szSerial) {
        //     std::cout << "* Result of parallel and serial processing is the same."
        //     << std::endl;
        // } else {
        //     std::cout << "* Result of parallel and serial processing is NOT the same."
        //     << std::endl;
        // }
    }

    MPI_Finalize();

    return 0;
}
