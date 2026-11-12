
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <stdint.h>

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

    std::cout << "NANA NKP READER TEST\n";
    std::cout << "File size: " << reader.fileSize() << "\n";
    std::cout << "Declared size: " << reader.declaredSize() << "\n";
    std::cout << "Resources: " << reader.resourceCount() << "\n\n";

    const std::vector<nana::nkp::Entry>& entries =
        reader.entries();

    for (size_t i = 0; i < entries.size(); ++i)
    {
        const nana::nkp::Entry& e = entries[i];

        std::cout << i + 1 << ". " << e.name
                  << " | size=" << e.size
                  << " | signature="
                  << (e.signatureValid ? "YES" : "NO")
                  << "\n";

        if (!reader.resourceData(e))
        {
            std::cerr << "ERROR: Invalid resource data\n";
            return 3;
        }
    }

    if (reader.fileSize() != 1681424 ||
        reader.declaredSize() != 1681424 ||
        reader.resourceCount() != 25)
    {
        std::cerr << "ERROR: Validation failed\n";
        return 4;
    }

    std::cout << "\nBASIC TESTS PASSED\n";
    return 0;
}
