// Copyright 2015 Dante Fan, Heetae Kim, and Saddem Alsudais
#include <mpi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "dna_util.h"

std::string parallel_main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " "
                  << "<input DNA file> <target DNA file>"
                  << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int numprocs, rank, namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &namelen);

    // Check the number of processes
    if (numprocs < 2) {
        std::cout << "[ABORT!] Number of processes is less than 2"
                  << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    std::string input_filename(argv[1]);
    std::string target_filename(argv[2]);
    DnaUtil dna;

    /////////////////////////////////////////////////////////////////
    // Host node sends data
    if (rank == 0) {
        // Initialize buffer
        char* buffer = new char[DnaUtil::kBufferSize];
        
        // Open input and target file
        dna.open(input_filename.c_str(), target_filename.c_str());
        
        // Disseminate input file
        for (int i = 1; i < numprocs; ++i) {
            dna.reset_input_cursor();
            while (true) {
                std::streampos number_of_byte = dna.read_next_input_file(buffer);
                if (number_of_byte <= 0)
                    break;
                MPI_Send(buffer, static_cast<int>(number_of_byte), MPI_CHAR, i, DnaUtil::kTagInputData, MPI_COMM_WORLD);
            }
            // Send zero length to finish seding
            memset(buffer, 0, DnaUtil::kBufferSize);
            MPI_Send(buffer, 2, MPI_CHAR, i, DnaUtil::kTagInputData, MPI_COMM_WORLD);
        }

        // Split the target file
        dna.split_target_file(1, numprocs - 1);
        
        // Test
        // std::cout << "* Rank(" << rank << ") target DNA : ";
        
        for (int i = 1; i < numprocs; ++i) {
            while (true) {
                std::streampos number_of_byte = dna.read_next_target_file(i, buffer);
                if (number_of_byte <= 0)
                    break;
                MPI_Send(buffer, static_cast<int>(number_of_byte), MPI_CHAR, i, DnaUtil::kTagTargetData, MPI_COMM_WORLD);
            }
            // Send zero length to finish seding
            memset(buffer, 0, DnaUtil::kBufferSize);
            MPI_Send(buffer, 2, MPI_CHAR, i, DnaUtil::kTagTargetData, MPI_COMM_WORLD);
        }
        
        // Test
        // std::cout << std::endl;
        
        delete[] buffer;
        dna.close_all_files();
    }

    /////////////////////////////////////////////////////////////////
    // Other nodes receive data
    if (rank > 0) {
        MPI_Status status;
        std::vector<char> input_sequence;
        char* buffer = new char[DnaUtil::kBufferSize];
        
        // Test
        // std::cout << "* Rank(" << rank << ") start receiving input DNA" << std::endl;
        
        // Receive input data first
        while (true) {
            memset(buffer, 0, DnaUtil::kBufferSize);
            MPI_Recv(buffer, DnaUtil::kBufferSize, MPI_CHAR, 0, DnaUtil::kTagInputData, MPI_COMM_WORLD, &status);
            // Convert to vector
            for (int i = 0; i < DnaUtil::kBufferSize; ++i) {
                if (buffer[i] == 0)
                    break;
                input_sequence.push_back(buffer[i]);
            }
            // No more to receive
            if (buffer[0] == 0)
                break;
        }

        // Then, receive target data
        // std::cout << "* Rank(" << rank << ") start receiving target DNA" << std::endl;
        while (true) {
            memset(buffer, 0, DnaUtil::kBufferSize);
            MPI_Recv(buffer, DnaUtil::kBufferSize, MPI_CHAR, 0, DnaUtil::kTagTargetData, MPI_COMM_WORLD, &status);
            // Test
            // std::cout << buffer;
            // No more to receive
            if (buffer[0] == 0)
                break;
            // Match up!
            dna.keep_matching_from_stream(&input_sequence, buffer);
        }
        
        // Test
        // std::cout << std::endl;
        
        delete[] buffer;

        // Send back matching results
        std::string matching_result;
        matching_result = dna.match_position_set_as_string(",");
        MPI_Send(matching_result.c_str(), static_cast<int>(matching_result.size()), MPI_CHAR, 0, DnaUtil::kTagMatchingData, MPI_COMM_WORLD);
    }
    
    /////////////////////////////////////////////////////////////////
    // Host node collects data
    std::ostringstream output_stream;
    
    if (rank == 0) {
        MPI_Status status;
        char* buffer = new char[DnaUtil::kBufferSize];
        
        std::cout << "* Result" << std::endl;
        
        for (int i = 1; i < numprocs; ++i) {
            memset(buffer, 0, DnaUtil::kBufferSize);
            MPI_Recv(buffer, DnaUtil::kBufferSize, MPI_CHAR, i, DnaUtil::kTagMatchingData, MPI_COMM_WORLD, &status);
            
            // Interpret each position
            std::stringstream string_converter(buffer);
            std::string string_position;
            while (std::getline(string_converter, string_position, ',')) {
                std::streampos correct_position;
                correct_position = dna.interpret_match_position(i, string_position);
                std::cout << correct_position << std::endl;
                output_stream << correct_position << std::endl;
            }
        }
        
        delete[] buffer;
    }

    return output_stream.str();
}
