//
// Created by wwwis on 15.02.2022.
//
#include <iostream>
#include <numeric>
#include <vector>
#include <algorithm>
#include <tuple>
#include <fstream>
#include "arith_enc_dec.h"
size_t BYTE_SIZE = 256;
/*
std::vector<unsigned char> read_bytes(const std::string &file_name, const bool read_meta = false) {
    size_t bwt_shift_position = SIZE_MAX;
    std::vector<unsigned char> data;

    std::ifstream fin(file_name, std::ios::binary);
    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(fin)), {});
    fin.close();

    if (read_meta) {
        bwt_shift_position = *((size_t *) bytes.data());
        data = bytes;
        data.push_back(bwt_shift_position);
    } else {
        data = bytes;
    }
    return data;
}
*/

std::vector<unsigned char> move_to_front_reverse(std::vector<unsigned char> data) {
    std::vector<unsigned char> alphabet(BYTE_SIZE);
    std::iota(alphabet.begin(), alphabet.end(), 0);
    std::vector<unsigned char> decoded_data(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        auto current_index = data[i];
        decoded_data[i] = alphabet[current_index];

        auto found_index_it = alphabet.begin() + current_index;
        if (found_index_it != alphabet.begin() && found_index_it != alphabet.end()) {
            std::rotate(alphabet.begin(), found_index_it, found_index_it + 1);
        }
    }
    return decoded_data;
}

size_t cyclic_index(const size_t &start_index, const size_t &offset, const size_t &n) {
    return start_index + offset < n ? start_index + offset : start_index + offset - n;
}

class bwt_cmp_straight {
    const std::vector<unsigned char> &data;
public:
    explicit bwt_cmp_straight(const std::vector<unsigned char> &bwt_data) : data(bwt_data) {}

    bool operator()(size_t left, size_t right) {
        size_t i = 0;
        while (data[cyclic_index(left, i, data.size())] == data[cyclic_index(right, i, data.size())] &&
               i < data.size()) {
            ++i;
        }
        return data[cyclic_index(left, i, data.size())] < data[cyclic_index(right, i, data.size())];
    }
};

class bwt_cmp_reverse {
    const std::vector<unsigned char> &data;
public:
    explicit bwt_cmp_reverse(const std::vector<unsigned char> &bwt_data) : data(bwt_data) {}

    bool operator()(size_t left, size_t right) {
        return data[left] < data[right];
    }
};

std::vector<unsigned char> bwt_reverse(const std::vector<unsigned char> &bwt_data, size_t row_index) {
    std::vector<size_t> l_shift(bwt_data.size());
    std::iota(l_shift.begin(), l_shift.end(), 0);
    std::stable_sort(l_shift.begin(), l_shift.end(), bwt_cmp_reverse(bwt_data));

    std::vector<unsigned char> initial_data(bwt_data.size(), '0');
    for (size_t i = 0; i < bwt_data.size(); ++i) {
        initial_data[i] = bwt_data[l_shift[row_index]];
        row_index = l_shift[row_index];
    }
    return initial_data;
}

std::tuple<std::vector<unsigned char>, size_t>
read_bytes(
        const std::string &file_name,
        const bool read_meta = false
) {
    size_t bwt_shift_position = SIZE_MAX;
    long size_of_tree = SIZE_MAX;
    std::vector<unsigned char> data;
    std::ifstream fin(file_name, std::ios::binary);
    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(fin)), {});

    fin.close();

    std::cout <<"size: " <<bytes.size() << std::endl;
    if (read_meta) {
        bwt_shift_position = *((size_t *) bytes.data());
        auto iter = bytes.begin();
        bytes.erase(iter, iter+4);
        data = bytes;
    } else {
        data = bytes;
    }

    return {data, bwt_shift_position};
}

void write_bytes(
        const std::string &file_name,
        const std::vector<unsigned char> &data,
        const size_t bwt_shift_position = SIZE_MAX
) {
    std::ofstream fout(file_name, std::ios::binary);
    if (bwt_shift_position != SIZE_MAX) {
        fout.write(reinterpret_cast<const char *>(&bwt_shift_position), sizeof(size_t));
    }
    fout.write(reinterpret_cast<const char *>(data.data()), static_cast<long>(data.size()));
    fout.close();
}

int main(int argc, char* argv[]) {
    std::vector<std::string> file_list = {"bib", "book1", "book2", "geo", "news", "obj1", "obj2", "paper1", "paper2",
                                          "pic", "progc", "progl", "progp", "trans"};
    //for(auto& file : file_list) {
    std::string input_file = "bib_enc_1";
    std::string output_decoded_main_file = "bib_decoded_2";
    std::string output_temporary_file = "debug_decode_mtf2";
    const char *cstr = input_file.c_str();
    const char *cstr_out = output_temporary_file.c_str();
    decode(cstr, cstr_out);
    //std::vector<unsigned char> bytes_input = read_bytes(output_temporary_file, true);
    const auto &[bytes_input, bwt_shift_position] = read_bytes(
            output_temporary_file, true);
    //auto bwt_shift_position = bytes_input.back();
    //bytes_input.pop_back();
    auto decoded_mtf = move_to_front_reverse(bytes_input);
    std::cout<< "pos: " << bwt_shift_position << std::endl;
    auto decoded_data = bwt_reverse(decoded_mtf, bwt_shift_position);
    write_bytes(output_decoded_main_file, decoded_data);
    //std::remove(cstr_out);
    return 0;
  //  }
    return 0;
}


