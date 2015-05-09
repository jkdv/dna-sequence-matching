// Copyright 2015 Dante Fan, Heetae Kim, and Saddem Alsudais

#ifndef _DNATUTIL_H_
#define _DNATUTIL_H_

#include <fstream>
#include <map>
#include <set>
#include <vector>

class DnaUtil {
public:
    static const int kBufferSize;
    static const int kTagInputData;
    static const int kTagTargetData;
    static const int kTagMatchingData;

private:
    std::ifstream input_file_;
    std::ifstream target_file_;
    std::streampos input_file_size_;
    std::streampos target_file_size_;
    std::map<int, std::streampos> split_target_file_position_map_;
    std::map<int, std::streampos> split_target_file_cursor_map_;
    std::map<int, std::streampos> split_target_file_size_map_;
    unsigned int number_of_split_target_file_;
    unsigned int start_of_file_number_;
    std::streampos input_file_cursor_;
    std::set<std::streampos> match_position_set_;
    std::map<std::streampos, char> target_stream_buffer_;
    std::streampos target_stream_position_;
    std::streampos target_stream_cursor_;

public:
    DnaUtil();
    ~DnaUtil();

    bool open(const char* input_filename, const char* target_filename);
    bool split_target_file(int start_index, unsigned int number_of_file);
    std::streampos target_length_of(unsigned int which_target_file) const;
    std::streampos target_begin_of(unsigned int which_target_file) const;
    std::streampos target_end_of(unsigned int which_target_file) const;
    std::streampos target_cursor_of(unsigned int which_target_file) const;
    std::streampos target_byte_to_read_of(unsigned int which_target_file) const;
    std::streamsize read_next_target_file(unsigned int which_target_file,
                                          char* buffer);
    void offset_target_cursor(unsigned int which_target_file,
                              std::streampos offset);
    
    std::streampos input_length() const;
    std::streampos input_cursor() const;
    std::streampos input_byte_to_read() const;
    std::streamsize read_next_input_file(char* buffer);
    void offset_input_cursor(std::streampos offset);
    void reset_input_cursor();
    void keep_matching_from_stream(const std::vector<char>* input_sequence,
                                   const char* part_of_target_sequence);
    std::set<std::streampos>::size_type match_size() const;
    std::set<std::streampos>& match_position_set();
    std::string match_position_set_as_string(const char* delimiter) const;
    std::streampos interpret_match_position(unsigned int which_target_file,
                                            std::streampos match_position);
    std::streampos interpret_match_position(unsigned int which_target_file,
                                            std::string match_position);
    
    void close_all_files();
    void close_input_file();
    void close_target_file();

private:
    void safe_delete(char* memory);
    void safe_close(std::ifstream* file);
};

#endif
