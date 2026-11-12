
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

    const std::vector<nana::nkp::Entry>& entries =
        reader.entries();

    std::cout << "NANA RESOURCE DEEP ANALYSIS\n";
    std::cout << "File size: " << reader.fileSize() << "\n";
    std::cout << "Resources: " << reader.resourceCount() << "\n\n";

    for (size_t i = 0; i < entries.size(); ++i)
    {
        const nana::nkp::Entry& e = entries[i];

        if (e.type != nana::nkp::RESOURCE_NKF &&
            e.type != nana::nkp::RESOURCE_TMF)
            continue;

        const uint8_t* data = reader.resourceData(e);

        if (!data)
        {
            std::cerr << "ERROR: Invalid resource data\n";
            return 3;
        }

        std::cout << "================================\n";
        std::cout << e.name << "\n";
        std::cout << "Offset: 0x" << std::hex
                  << std::uppercase << e.dataOffset << "\n";
        std::cout << "Size: " << std::dec << e.size << "\n";
        std::cout << "Signature valid: "
                  << (e.signatureValid ? "YES" : "NO") << "\n";

        size_t limit = e.size < 64 ? e.size : 64;

        std::cout << "Bytes:\n";

        for (size_t j = 0; j < limit; ++j)
        {
            if (j % 16 == 0)
            {
                std::cout << "  ";
            }

            std::cout << std::hex << std::uppercase
                      << std::setw(2) << std::setfill('0')
                      << (unsigned int)data[j] << " ";

            if (j % 16 == 15 || j + 1 == limit)
                std::cout << "\n";
        }

        std::cout << std::dec << "\n";
    }

    std::cout << "DEEP ANALYSIS COMPLETE\n";

    return 0;
}
