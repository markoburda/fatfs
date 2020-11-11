#include <iostream>
#include <fstream>
#include <sstream>
#include <memory.h>


int read_bytes(void *var, std::string &src, int offset, int n) {
    memcpy(var, src.c_str() + offset, n);
    return 0;
}

struct boot_sector{
    uint16_t bytes_per_sector;
    uint16_t sectors_per_cluster;
    uint16_t reserved_area;
    uint16_t num_of_fats;
    uint16_t max_file_number;
    uint16_t size_of_fat;
    uint16_t signature_value;
};

struct file_struct {
    char name[11];
    uint16_t size_in_bytes;
    uint16_t modification_time;
    uint16_t modification_date;
    uint16_t attribute;
    uint16_t first_cluster;
};


int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Using: readimage <path>" << std::endl;
        return EXIT_FAILURE;
    }
    std::ifstream fin(argv[1], std::ifstream::binary);
    std::ostringstream ostrm;
    ostrm << fin.rdbuf();
    std::string image(ostrm.str());

    boot_sector boot;
    read_bytes(&boot.bytes_per_sector, image, 11, 2);
    read_bytes(&boot.sectors_per_cluster, image, 13, 1);
    read_bytes(&boot.reserved_area, image, 14, 2);
    read_bytes(&boot.num_of_fats, image, 16, 1);
    read_bytes(&boot.max_file_number, image, 17, 2);
    read_bytes(&boot.size_of_fat, image, 22, 2);
    read_bytes(&boot.signature_value, image, 510, 2);
    
    int offset = (boot.reserved_area + boot.num_of_fats * boot.size_of_fat) * boot.bytes_per_sector;

    for (int i = 0; i < boot.max_file_number; i++) {
        file_struct file;
        read_bytes(&file.name, image, offset, 11);
       



    }

    std::cout << "Boot sector info" << std::endl;
    std::cout << "Bytes per sector: " << boot.bytes_per_sector << std:: endl;
    std::cout << "Sectors per cluster: " << boot.sectors_per_cluster << std:: endl;
    std::cout << "Size of reserved area, in sectors: "<< boot.reserved_area << std:: endl;
    std::cout << "Number of FATs: "<< boot.num_of_fats << std:: endl;
    std::cout << "Maximum number of files in the root directory: " <<boot.max_file_number << std:: endl;
    std::cout << "Size of each FAT, in sectors: " << boot.size_of_fat << std:: endl;
    std::cout << "Signature value: 0x" << std::hex << boot.signature_value << std:: endl;

    return EXIT_SUCCESS;
}