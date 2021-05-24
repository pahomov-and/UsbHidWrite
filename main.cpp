#include <iostream>
#include <vector>
#include <fstream>
#include <iterator>
#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>

#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>

#include "USB.h"

uint16_t vendorID = 0;
uint16_t productID = 0;
int interface_number = 0;
unsigned char endpoint = 1;
std::string file;
bool isPipe = false;
std::string pipe_str = "";

uint32_t display_width = 1024;
uint32_t display_height = 768;
uint32_t display_bp = 3;
uint32_t patternByts = display_width*display_height;
uint32_t patternCount = 1;

USB usb;
//std::vector<uint8_t> data;

using  namespace std::chrono_literals;

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
                        {"pattern_byte",        required_argument, 0, 'P'},
                        {"pattern_count",       required_argument, 0, 'N'},
                        {"pipe_line",           required_argument,0, 'l'},
                        {"width",               required_argument,0, 'W'},
                        {"height",              required_argument,0, 'H'},
                        {"bp",                  required_argument,0, 'b'},
                        {0, 0, 0, 0}
                };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "p:v:i:e:f:P:N:l:W:H:b:",
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

            case 'P':
                patternByts = strtol(optarg, nullptr, 0);
                LOG_INFO ("Pattern bytes: ", patternByts);
                break;
            case 'N':
                patternCount = strtol(optarg, nullptr, 0);
                LOG_INFO ("Patterns count: ", patternCount);
                break;
            case 'l':
                pipe_str.assign(optarg);
                isPipe = true;
                if (pipe_str.size() > 0) {
                    std::string cmd = "mknod " + pipe_str + " p";
                    system(cmd.c_str());
                }
                LOG_INFO ("Pipe: ", pipe_str);
                break;
            case 'W':
                display_width = strtol(optarg, nullptr, 0);
                LOG_INFO ("Display width: ", display_width);
                break;
            case 'H':
                display_height= strtol(optarg, nullptr, 0);
                LOG_INFO ("Display height: ", display_height);
                break;
            case 'b':
                display_bp = strtol(optarg, nullptr, 0);
                LOG_INFO ("Pixel format. Byte on pixel: ", display_bp);
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
                             "-f - File\n"
                             "-P - Pattern bytes\n"
                             "-N - Patterns count\n"
                             "-l - Pipe file\n"
                             "-W - display width\n"
                             "-H - display height\n"
                             "-b - Pixel format. Byte on pixel\n"
                             "\n"
                             "For linux: echo 2702700 > /proc/sys/fs/pipe-max-size \n"
                             "For linux: echo 1000 > /sys/module/usbcore/parameters/usbfs_memory_mb\n";


                usb.ShowDevices();
                break;

            default:
                abort ();
        }
    }

}

std::vector<uint8_t> data_pattern;

void PatternInc(std::vector<uint8_t> &data) {
    uint8_t byte = 0;

    for (int i = 0; i < patternByts; ++i) {
        data_pattern.push_back(byte);
        //std::cout << std::hex << (int) byte << std::dec << " ";
        byte++;
        if(byte >= 0xfd) byte = 0;
    }
}

void PatternDec(std::vector<uint8_t> &data) {
    uint8_t byte = 0xff;

    for (int i = 0; i < patternByts; ++i) {
        data_pattern.push_back(byte);
        byte--;
    }
}

void PatternShift(std::vector<uint8_t> &data) {
    uint8_t byte = 0x01;

    for (int i = 0; i < patternByts; ++i) {
        data_pattern.push_back(byte);
        byte = byte << 1;

        if(byte >= 0x80) byte = 0x01;
    }

}

int main(int argc, char **argv) {
    ParsingOptions(argc, argv);

    if (productID != 0 && vendorID != 0) {

        usb.SetVidPid(vendorID, productID);
        usb.Connect();
        usb.ClaimInterface(interface_number);

        if(isPipe) {
            int fd = 0;
            if (pipe_str.size() > 0) {
                fd = open(pipe_str.c_str(), O_RDONLY);
                if (fd < 0) {
                    LOG_ERROR("Open failed");
                    exit(1);
                }
            }

            uint32_t len_buf = display_width*display_height*display_bp;

            long pipe_size = (long)fcntl(fd, F_GETPIPE_SZ);
            if (pipe_size == -1) {
                LOG_ERROR("Get pipe size failed");
            }

            LOG_INFO("Default pipe size: ", pipe_size);

            int ret = fcntl(fd, F_SETPIPE_SZ, len_buf);
            if (ret < 0) {
                LOG_ERROR("Set pipe size failed");
            }

            pipe_size = (long)fcntl(fd, F_GETPIPE_SZ);
            if (pipe_size == -1) {
                LOG_ERROR("Get pipe size failed");
            }

            LOG_INFO("New pipe size: ", pipe_size);

            std::vector<uint8_t> data(pipe_size);

            int n;
            while ((n=read(fd, data.data(), len_buf)) > 0) {
                LOG_INFO("n: ", n);
                usb.Write(endpoint, data.data(), n);
            }
            close(fd);
            exit(0);
        }

        if (file.size() > 0) {
            std::vector<uint8_t> data = readFile(file);
            LOG_INFO("DATA SIZE: ", data.size());
            usb.Write(endpoint, data.data(), data.size());
        } else {

            PatternInc(data_pattern);
//            PatternDec(data_pattern);
//            PatternShift(data_pattern);

            for (int i = 0; i < patternCount; ++i) {

                LOG_INFO("DATA SIZE: ", data_pattern.size());
                usb.Write(endpoint, data_pattern.data(), data_pattern.size());
                std::this_thread::sleep_for(10ms);
            }
        }

        LOG_INFO("Push enter to exit!");
        std::getchar();
    }

    return 0;
}
