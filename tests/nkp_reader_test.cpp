
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <stdint.h>
#include <iomanip>

#include "nkp_reader.h"

int main()
{
    std::ifstream file("TITLE.NKP", std::ios::binary);

    if (!file)
    {
        std::cerr << "ERROR: TITLE.NKP no encontrado\n";
        return 1;
    }

    std::vector<uint8_t> buffer(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    nana::nkp::Reader reader;

    if (!reader.open(buffer.data(), buffer.size()))
    {
        std::cerr << "ERROR: " << reader.error() << "\n";
        return 2;
    }

    std::cout << "NANA NKT/NKF/TMF HEADER ANALYSIS\n";
    std::cout << "File size: " << reader.fileSize() << "\n";
    std::cout << "Resources: " << reader.resourceCount() << "\n\n";

    const std::vector<nana::nkp::Entry>& entries =
        reader.entries();

    for (size_t i = 0; i < entries.size(); ++i)
    {
        const nana::nkp::Entry& e = entries[i];
        const uint8_t* data = reader.resourceData(e);

        if (!data)
        {
            std::cerr << "ERROR: Invalid resource data\n";
            return 3;
        }

        std::cout << i + 1 << ". " << e.name << "\n";
        std::cout << "   Offset: 0x"
                  << std::hex << std::uppercase
                  << e.dataOffset << "\n";

        std::cout << "   Size: " << std::dec
                  << e.size << "\n";

        std::cout << "   Header: ";

        size_t bytesToPrint = e.size < 16 ? e.size : 16;

        for (size_t j = 0; j < bytesToPrint; ++j)
        {
            std::cout << std::hex << std::uppercase
                      << std::setw(2) << std::setfill('0')
                      << (unsigned int)data[j] << " ";
        }

        std::cout << std::dec << "\n\n";
    }

    std::cout << "HEADER ANALYSIS COMPLETE\n";
    return 0;
}
