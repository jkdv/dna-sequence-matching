// Copyright 2015 Dante Fan, Heetae Kim, and Saddem Alsudais
#include <mpi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "dna_util.h"

std::string serial_main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " "
        << "<input DNA file> <target DNA file>"
        << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    std::string input_filename(argv[1]);
    std::string target_filename(argv[2]);
    DnaUtil dna;
    
    // Open input and target file
    dna.open(input_filename.c_str(), target_filename.c_str());
    
    // Split the target file
    dna.split_target_file(1, 1);
    
    // Convert input data into vector
    std::vector<char> input_sequence;
    char* buffer = new char[DnaUtil::kBufferSize];
    while (true) {
        std::streampos number_of_byte = dna.read_next_input_file(buffer);
        if (number_of_byte <= 0)
            break;
        for (int i = 0; i < DnaUtil::kBufferSize; ++i) {
            if (buffer[i] == 0)
                break;
            input_sequence.push_back(buffer[i]);
        }
        
    }
    
    // Read target data
    while (true) {
        std::streampos number_of_byte = dna.read_next_target_file(1, buffer);
        if (number_of_byte <= 0)
            break;
        dna.keep_matching_from_stream(&input_sequence, buffer);
    }
    
    delete[] buffer;
    
    // Result
    std::string matching_result;
    matching_result = dna.match_position_set_as_string(",");
    
    // Interpret each position
    std::ostringstream output_stream;
    std::stringstream string_converter(matching_result);
    std::string string_position;
    while (std::getline(string_converter, string_position, ',')) {
        std::streampos correct_position;
        correct_position = dna.interpret_match_position(1, string_position);
        output_stream << correct_position << std::endl;
    }

    return output_stream.str();
}
