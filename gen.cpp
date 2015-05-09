// Copyright 2015 Dante Fan, Heetae Kim, and Saddem Alsudais

#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " "
        << "<output DNA file> <length>"
        << std::endl;
        return -1;
    }
    
    srand((unsigned int)time(NULL));
    char bases[] = {'G', 'A', 'T', 'C'};
    std::string filename(argv[1]);
    size_t length = atoi(argv[2]);

    std::ofstream file;
    file.open(filename);

    // 4 MB = 4000000 bytes
    // const int size = 1000 * 1000 * 4;
    
    for (size_t i = 0; i < length; ++i) {
        file << bases[rand() % 4];
    }

    file.close();

    return 0;
}
