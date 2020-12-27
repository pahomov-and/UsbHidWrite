#include <iostream>
#include <vector>
#include <fstream>
#include <iterator>
#include <cstdlib>
#include <string>

#include <getopt.h>

#include "USB.h"

uint16_t vendorID = 0;
uint16_t productID = 0;
int interface_number = 0;
unsigned char endpoint = 1;
std::string file;

std::vector<uint8_t> readFile(const std::string filename)
{
    std::vector<uint8_t> vec;
    std::ifstream file(filename.c_str(), std::ios::binary);
    if  (!file.good() )
        return vec;

    file.unsetf(std::ios::skipws);

    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    vec.reserve(fileSize);

    vec.insert(vec.begin(),
               std::istream_iterator<uint8_t>(file),
               std::istream_iterator<uint8_t>());

    return vec;
}


void ParsingOptions(int argc, char **argv) {
    int c;
    while (1)
    {
        static struct option long_options[] =
                {

                        {"product",             required_argument, 0, 'p'},
                        {"vendor",              required_argument, 0, 'v'},
                        {"interface_number",    required_argument, 0, 'i'},
                        {"endpoint",            required_argument, 0, 'e'},
                        {"file",                required_argument, 0, 'f'},
                        {0, 0, 0, 0}
                };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "p:v:i:e:f:",
                         long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                LOG_INFO ("option ", long_options[option_index].name);
                if (optarg)
                    LOG_INFO (" with arg ", optarg);
                break;

            case 'p':
                productID = strtol(optarg, nullptr, 0);
                LOG_INFO ("productID: 0x", std::hex, productID, std::dec);
                break;

            case 'v':
                vendorID = strtol(optarg, nullptr, 0);
                LOG_INFO ("vendorID: 0x", std::hex, vendorID, std::dec);
                break;

            case 'i':
                interface_number = strtol(optarg, nullptr, 0);
                LOG_INFO ("interface_number: 0x", std::hex, interface_number, std::dec);
                break;

            case 'e':
                endpoint = strtol(optarg, nullptr, 0);
                LOG_INFO ("endpoint: 0x", std::hex, (int)endpoint, std::dec);
                break;

            case 'f':
                file = optarg;
                LOG_INFO ("file: ", file);
                break;

            case '?':
                /* getopt_long already printed an error message. */
                std::cout <<
                          "Writing data from file to USB:\n"
                          "Example: ./UsbHidWrite -v 0x1d50 -p 0x607a -i 0 -e 1 -f Makefile \n"
                             "-v - Vendor ID\n"
                             "-p - Product ID\n"
                             "-i - Interface number\n"
                             "-e - Endpoint\n"
                             "-f - File\n";
                break;

            default:
                abort ();
        }
    }

}

int main(int argc, char **argv) {
    USB usb;

    ParsingOptions(argc, argv);

    if (productID != 0 && vendorID != 0 && file.size() > 0) {
        std::vector<uint8_t> data = readFile(file);
        LOG_INFO("DATA SIZE: ", data.size());
        usb.Write(vendorID, productID, interface_number, endpoint, data.data(), data.size());
    }

    return 0;
}
