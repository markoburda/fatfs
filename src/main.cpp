#include <iostream>
#include <fstream>
#include <sstream>
#include <memory.h>
#include <map>
#include <iomanip>


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
    uint16_t create_time;
    uint16_t create_date;
    uint16_t modify_time;
    uint16_t modify_date;
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

    boot_sector boot {};
    read_bytes(&boot.bytes_per_sector, image, 11, 2);
    read_bytes(&boot.sectors_per_cluster, image, 13, 1);
    read_bytes(&boot.reserved_area, image, 14, 2);
    read_bytes(&boot.num_of_fats, image, 16, 1);
    read_bytes(&boot.max_file_number, image, 17, 2);
    read_bytes(&boot.size_of_fat, image, 22, 2);
    read_bytes(&boot.signature_value, image, 510, 2);

    std::cout << "Boot sector info" << std::endl;
    std::cout << "Bytes per sector: " << boot.bytes_per_sector << std:: endl;
    std::cout << "Sectors per cluster: " << boot.sectors_per_cluster << std:: endl;
    std::cout << "Size of reserved area, in sectors: "<< boot.reserved_area << std:: endl;
    std::cout << "Number of FATs: "<< boot.num_of_fats << std:: endl;
    std::cout << "Maximum number of files in the root directory: " <<boot.max_file_number << std:: endl;
    std::cout << "Size of each FAT, in sectors: " << boot.size_of_fat << std:: endl;
    std::cout << "Signature value: 0x" << std::hex << boot.signature_value << std:: endl;

    int offset = (boot.reserved_area + boot.num_of_fats * boot.size_of_fat) * boot.bytes_per_sector;

    std::map<uint16_t, std::string> attributes = {
            {0x01, "read-only"},
            {0x02, "hidden"},
            {0x04, "system"},
            {0x08, "volume"},
            {0x0f, "long-filename"},
            {0x10, "dir"},
            {0x20, "archive"},
    };
    std::cout << std::left << std::setw(11) << "filename" << '\t' << \
        std::left << std::setw(9) << "File size" << '\t' << \
        std::left << std::setw(11) << "Modify time" << '\t' << \
        std::left << std::setw(11) << "Modify date" << '\t' << \
        std::left << std::setw(10) << "Attributes" << '\t' << \
        std::left << std::setw(13) << "First cluster" << \
        std::left << std::setw(13) << "First sector" << std::endl;

    for (int i = 0; i < boot.max_file_number; i++) {
        file_struct file {};
        read_bytes(&file.name, image, offset, 11);
        if ((file.name == NULL)|| (file.name[0] == '\0'))
            break;
        read_bytes(&file.attribute, image, offset + 11, 1);
        read_bytes(&file.modify_time, image, offset + 22, 2);
        read_bytes(&file.modify_date, image, offset + 24, 2);
        read_bytes(&file.first_cluster, image, offset + 26, 2);
        read_bytes(&file.size_in_bytes, image, offset + 28, 4);
        offset += 32;
        std::ostringstream size;
        if (attributes[file.attribute]=="dir") {
            size << "DIR";
        } else {
            size << file.size_in_bytes;
        }
        uint16_t day_b = file.modify_date & 0b11111;
        uint16_t month_b = file.modify_date >> 5 & 0b1111;
        uint16_t year_b = (file.modify_date >> 9 & 0b1111111) + 1980;
        uint16_t hour_b = file.modify_time >> 11 & 0b11111;
        uint16_t minute_b = file.modify_time >> 5 & 0b111111; 
        uint16_t second_b = (file.modify_time & 0b11111) * 2;
        std::ostringstream day, month, hour, minute, second, year;
        year << year_b;
        if (day_b < 10) 
            day << "0" << day_b;
        else
            day << day_b;
        if (month_b < 10) 
            month << "0" << month_b;
        else
            month << month_b;
        if (hour_b < 10) 
            hour << "0" << hour_b;
        else
            hour << hour_b;
        if (minute_b < 10) 
            minute << "0" << minute_b;
        else
            minute << minute_b;
        if (second_b < 10) 
            second << "0" << second_b;
        else
            second << second_b;
        uint16_t first_sector;
        if (file.first_cluster == 0)
            first_sector = 0;
        else
            first_sector = 1256 + (file.first_cluster-2)*boot.sectors_per_cluster;
        std::cout << std::left << std::setw(11) << file.name << '\t' << \
        std::left << std::setw(9) << size.str() << '\t' << \
        std::left << std::setw(2) << hour.str() << ":" << std::setw(2) << minute.str() << ":" << std::setw(2) << second.str() << '\t' << \
        std::left << std::setw(2) << day.str() << "-" << std::setw(2) << month.str() << "-" << std::setw(4) << year.str() << '\t' << \
        std::left << std::setw(10) << attributes[file.attribute] << '\t' << \
        std::left << std::setw(10) << std::dec << file.first_cluster << '\t' << \
        std::left << std::setw(10) << std::dec << first_sector << '\t' << std::endl;
    }

    return EXIT_SUCCESS;
}
