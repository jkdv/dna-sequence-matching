// Copyright 2015 Dante Fan, Heetae Kim, and Saddem Alsudais

#include "dna_util.h"
#include <string>
#include <sstream>
#include <iostream>
#include <cstring>

const int DnaUtil::kBufferSize = 1000 * 1000 * 100;
const int DnaUtil::kTagInputData = 0xFEF + 1;
const int DnaUtil::kTagTargetData = 0xFEF + 2;
const int DnaUtil::kTagMatchingData = 0xFEF + 3;

DnaUtil::DnaUtil()
: input_file_size_(0)
, target_file_size_(0)
, number_of_split_target_file_(0)
, start_of_file_number_(0)
, input_file_cursor_(0)
, target_stream_position_(0)
, target_stream_cursor_(0) {
}


DnaUtil::~DnaUtil() {
    safe_close(&input_file_);
    safe_close(&target_file_);
}


bool DnaUtil::open(const char* input_filename, const char* target_filename) {
    // Open target file
    target_file_.open(target_filename, std::ios::ate);
    if (!target_file_.is_open()) {
        std::cerr << "* Fail to open target DNA file: "
        << target_filename
        << std::endl;
        return false;
    }
    
    // Calculate target length
    target_file_size_ = target_file_.tellg();
    target_file_.seekg(0, std::ios::beg);
    
    // Open input file
    input_file_.open(input_filename, std::ios::ate);
    if (!input_file_.is_open()) {
        std::cerr << "* Fail to open input DNA file: "
        << input_filename
        << std::endl;
        return false;
    }
    
    // Calculate input length
    input_file_size_ = input_file_.tellg();
    input_file_.seekg(0, std::ios::beg);
    
    // Test
    std::cout << "* Input size : " << input_file_size_ << std::endl;
    std::cout << "* Target size : " << target_file_size_ << std::endl;
    
    return true;
}


bool DnaUtil::split_target_file(int start_index, unsigned int number_of_file) {
    if (!input_file_.is_open())
        return false;
    
    start_of_file_number_ = start_index;
    
    if (number_of_file == 1) {
        int index = start_of_file_number_;
        split_target_file_size_map_[index] = target_file_size_;
        split_target_file_position_map_[index] = 0;
        split_target_file_cursor_map_[index] = split_target_file_position_map_[index];
        return true;
    }
    
    number_of_split_target_file_ = number_of_file;
    
    
    std::streampos base_file_size, last_file_size;
    base_file_size = target_file_size_ / number_of_file;
    last_file_size = target_file_size_ - (base_file_size * (number_of_file - 1));
    
    // Test
    std::cout << "* Base file size : " << base_file_size << std::endl;
    std::cout << "* Last file size : " << last_file_size << std::endl;
    
    for (int i = 0; i < number_of_file; ++i) {
        int index = start_of_file_number_ + i;
        // First file
        if (i == 0) {
            split_target_file_size_map_[index] = base_file_size + input_file_size_ - (input_file_size_ / 2);
            split_target_file_position_map_[index] = 0;
        }
        // From second to second-to-last
        if (i > 0 && i < number_of_file - 1) {
            split_target_file_size_map_[index] = base_file_size + input_file_size_;
            split_target_file_position_map_[index] = split_target_file_position_map_[index - 1] + split_target_file_size_map_[index - 1] - input_file_size_;
        }
        // Last file
        if (i == number_of_file - 1) {
            split_target_file_size_map_[index] = last_file_size + (input_file_size_ / 2);
            split_target_file_position_map_[index] = split_target_file_position_map_[index - 1] + split_target_file_size_map_[index - 1] - input_file_size_;
        }
        // Get cursor ready
        split_target_file_cursor_map_[index] = split_target_file_position_map_[index];
        
        // Test
        std::cout
        << "* File(" << index << ") starts at : " << target_begin_of(index)
        << ", ends at : " << target_end_of(index)
        << ", cursor at : " << target_cursor_of(index)
        << ", size : " << target_length_of(index) << std::endl;
    }
    return true;
}


std::streampos DnaUtil::target_length_of(unsigned int which_target_file) const {
    return split_target_file_size_map_.at(which_target_file);
}


std::streampos DnaUtil::target_begin_of(unsigned int which_target_file) const {
    return split_target_file_position_map_.at(which_target_file);
}


std::streampos DnaUtil::target_end_of(unsigned int which_target_file) const {
    return target_begin_of(which_target_file) + target_length_of(which_target_file) - static_cast<std::streampos>(1);
}


std::streampos DnaUtil::target_cursor_of(unsigned int which_target_file) const {
    return split_target_file_cursor_map_.at(which_target_file);
}


std::streampos DnaUtil::target_byte_to_read_of(unsigned int which_target_file) const {
    return target_end_of(which_target_file) - target_cursor_of(which_target_file) + 1;
}


void DnaUtil::offset_target_cursor(unsigned int which_target_file,
                                   std::streampos offset) {
    split_target_file_cursor_map_[which_target_file] = target_cursor_of(which_target_file) + offset;
}


std::streamsize DnaUtil::read_next_target_file(unsigned int which_target_file,
                                               char* buffer) {
    if (split_target_file_cursor_map_.empty())
        return -1;
    
    std::streampos cursor, byte_to_read;
    cursor = target_cursor_of(which_target_file);
    byte_to_read = target_byte_to_read_of(which_target_file);
    
    if (byte_to_read <= 0)
        return -1;
    
    // Test
    // std::cout << "* Byte to read: " << byte_to_read << std::endl;
    
    // Read file
    target_file_.seekg(cursor, std::ios::beg);
    memset(buffer, 0, kBufferSize);
    
    if (kBufferSize > byte_to_read)
        target_file_.read(buffer, byte_to_read);
    else
        target_file_.read(buffer, kBufferSize);
    
    // Move cursor for next phase
    std::streamsize read_size = target_file_.gcount();
    offset_target_cursor(which_target_file, read_size);
    
    return read_size;
}


std::streampos DnaUtil::input_length() const {
    return input_file_size_;
}


std::streampos DnaUtil::input_cursor() const {
    return input_file_cursor_;
}


std::streampos DnaUtil::input_byte_to_read() const {
    return input_length() - input_cursor();
}


void DnaUtil::offset_input_cursor(std::streampos offset) {
    input_file_cursor_ += offset;
}


void DnaUtil::reset_input_cursor() {
    input_file_cursor_ = 0;
}


std::streamsize DnaUtil::read_next_input_file(char* buffer) {
    if (input_cursor() >= input_length())
        return -1;
    
    std::streampos cursor, byte_to_read;
    cursor = input_cursor();
    byte_to_read = input_byte_to_read();
    
    if (byte_to_read <= 0)
        return -1;
    
    // Read input file
    input_file_.seekg(cursor, std::ios::beg);
    memset(buffer, 0, kBufferSize);
    
    if (kBufferSize > byte_to_read)
        input_file_.read(buffer, byte_to_read);
    else
        input_file_.read(buffer, kBufferSize);

    // Move cursor for next phase
    std::streamsize read_size = input_file_.gcount();
    offset_input_cursor(read_size);
    
    return read_size;
}


void DnaUtil::keep_matching_from_stream(const std::vector<char>* input_sequence,
                                        const char* part_of_target_sequence) {
    // Insert to stream buffer
    for (int i = 0; i < kBufferSize; ++i) {
        if (part_of_target_sequence[i] == 0)
            break;
        target_stream_buffer_.insert({
            target_stream_position_,
            part_of_target_sequence[i]
        });
        target_stream_position_ += 1;
    }
    
    // Try matching until target buffer is run out
    std::streampos input_cursor = 0;
    char input_nucleobase, target_nucleobase;
    while (target_stream_buffer_.size() >= input_sequence->size()) {
        // Test
        // std::cout << "* Input(" << input_cursor
        // << "), Target(" << target_stream_cursor_
        // << ")" << std::endl;
        
        input_nucleobase = input_sequence->at(input_cursor);
        target_nucleobase = target_stream_buffer_.at(target_stream_cursor_);
        
        // If not matched,
        // remove the target buffer from begin to the current element
        // and initialize input cursor and increment target stream cursor
        if (input_nucleobase != target_nucleobase) {
            if (input_cursor == 0) {
                target_stream_buffer_.erase(target_stream_buffer_.begin());
                target_stream_cursor_ += 1;
            } else {
                auto it = target_stream_buffer_.find(target_stream_cursor_);
                target_stream_buffer_.erase(target_stream_buffer_.begin(), it);
                input_cursor = 0;
            }
        }
        
        // If matched,
        // and when input cursor indicates at the end of input sequence,
        // add current target stream cursor to intermediate match score,
        // erase target buffer from begin to current target cursor - input size
        // initialize input cursor and move target stream cursor to the begin,
        // but when input cursor does not indicates at the end of input sequence,
        // increament both input cursor and target cursor.
        if (input_nucleobase == target_nucleobase) {
            if (input_cursor == input_sequence->size() - 1) {
                // First position of the match
                std::streampos first_match_position;
                first_match_position = target_stream_cursor_ - input_cursor;
                match_position_set_.insert(first_match_position);
                target_stream_buffer_.erase(target_stream_buffer_.begin());
                
                input_cursor = 0;
                target_stream_cursor_ = first_match_position;
                target_stream_cursor_ += 1;
            } else {
                input_cursor += 1;
                target_stream_cursor_ += 1;
            }
        }
    }
}


std::set<std::streampos>::size_type DnaUtil::match_size() const {
    return match_position_set_.size();
}


std::set<std::streampos>& DnaUtil::match_position_set() {
    return match_position_set_;
}


std::string DnaUtil::match_position_set_as_string(const char* delimiter) const {
    std::ostringstream builder;
    for (auto i = match_position_set_.begin(); i != match_position_set_.end(); ++i) {
        builder << *i << delimiter;
    }
    return builder.str();
}


std::streampos DnaUtil::interpret_match_position(unsigned int which_target_file,
                                                 std::streampos match_position) {
    std::streampos offset = target_begin_of(which_target_file);
    return match_position + offset;
}


std::streampos DnaUtil::interpret_match_position(unsigned int which_target_file,
                                                 std::string match_position) {
    std::streampos offset = target_begin_of(which_target_file);
    std::streampos converted_position = std::stoull(match_position);
    return converted_position + offset;
}


void DnaUtil::close_all_files() {
    safe_close(&input_file_);
    safe_close(&target_file_);
}


void DnaUtil::close_input_file() {
    safe_close(&input_file_);
}


void DnaUtil::close_target_file() {
    safe_close(&target_file_);
}


void DnaUtil::safe_delete(char* memory) {
    if (memory)
        delete[] memory;
}


void DnaUtil::safe_close(std::ifstream* file) {
    if (file->is_open())
        file->close();
}
