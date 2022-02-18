#include <iostream>
#include <numeric>
#include <vector>
#include <algorithm>
#include <tuple>
#include <fstream>
#include "arith_enc_dec.h"
size_t BYTE_SIZE = 256;


std::vector<unsigned char> move_to_front(
        std::vector<unsigned char> data
) {
    std::vector<unsigned char> alphabet(BYTE_SIZE);
    std::iota(alphabet.begin(), alphabet.end(), 0);
    std::vector<unsigned char> encoded_data(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        auto current_symbol = data[i];
        auto found_index_it = std::find_if(alphabet.begin(), alphabet.end(),
                                           [&current_symbol](const unsigned char &c) -> bool {
                                               return c == current_symbol;
                                           });
        auto found_index = found_index_it - alphabet.begin();
        encoded_data[i] = found_index;
        if (found_index != 0 && found_index_it != alphabet.end()) {
            std::rotate(alphabet.begin(), found_index_it, found_index_it + 1);
        }
    }
    return encoded_data;
}

std::vector<unsigned char> move_to_front_reverse(
        std::vector<unsigned char> data
) {
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

size_t cyclic_index(
        const size_t &start_index,
        const size_t &offset,
        const size_t &n
) {
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

std::pair<size_t, std::vector<unsigned char>> bwt(
        std::vector<unsigned char> data
) {
    std::vector<size_t> shift_order(data.size());
    std::iota(shift_order.begin(), shift_order.end(), 0);
    std::stable_sort(shift_order.begin(), shift_order.end(), bwt_cmp_straight(data));

    std::vector<unsigned char> encoded(data.size());
    size_t shift_position;
    for (size_t i = 0; i < data.size(); ++i) {
        encoded[i] = data[cyclic_index(shift_order[i], data.size() - 1, data.size())];
        if (shift_order[i] == 0) shift_position = i;
    }
    return std::make_pair(shift_position, encoded);
}

std::vector<unsigned char> bwt_reverse(
        const std::vector<unsigned char> &bwt_data,
        size_t row_index
) {
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

    if (read_meta) {
        bwt_shift_position = *((size_t *) bytes.data());
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
        std::cout << "cast: " <<bwt_shift_position << std::endl;
    }
    fout.write(reinterpret_cast<const char *>(data.data()), static_cast<long>(data.size()));
    fout.close();
}

int main(int argc, char* argv[]) {
    std::vector<std::string> file_list = {"bib", "book1", "book2", "geo", "news", "obj1", "obj2", "paper1", "paper2",
                                          "pic", "progc", "progl", "progp", "trans"};
    std::string dir = "calgarycorpus/";
    int a = 0;
    for(auto& file : file_list) {
        std::string input_file = dir + file;
        std::string output_encoded_main_file = dir + file + ".enc";
        std::string output_temporary_file =  dir + "mtf";
        const auto &[bytes_input, dummy3] = read_bytes(input_file);
        //std::vector<unsigned char> bytes_input = read_bytes(input_file);
        auto bwt_result = bwt(bytes_input);
        auto bwt_data = bwt_result.second;
        auto bwt_shift_position = bwt_result.first;
        std::cout<< "pos: " << (size_t)bwt_shift_position << std::endl;
        auto mtf_data = move_to_front(bwt_data);
        write_bytes(output_temporary_file, mtf_data, bwt_shift_position);
        const char *cstr = output_temporary_file.c_str();
        const char *cstr_out = output_encoded_main_file.c_str();
        encode(cstr, cstr_out);
        std::remove(cstr);
        std::cout << ++a << std::endl;
    }
    return 0;
    /*
    if (argc < 2){
        std::cout << "Wrong argument! Enter [input file][output file]" << std::endl;
        return 1;
    }
    std::string input_file = argv[1];
    std::string output_encoded_main_file;
    if (argc == 3){
        output_encoded_main_file = input_file + ".enc";
    }
    else output_encoded_main_file = argv[2];
    std::string output_temporary_file =  "encode_mtf";
    const auto &[bytes_input, a] = read_bytes(input_file);
    auto bwt_result = bwt(bytes_input);
    auto bwt_data = bwt_result.second;
    auto bwt_shift_position = bwt_result.first;
    auto mtf_data = move_to_front(bwt_data);
    write_bytes(output_temporary_file, mtf_data);
    const char *cstr = output_temporary_file.c_str();
    const char *cstr_out = output_encoded_main_file.c_str();
    encode(cstr, cstr_out);
    std::remove(cstr);
    return 0;
     */
}
