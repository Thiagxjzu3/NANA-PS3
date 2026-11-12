
#ifndef NANA_NKP_READER_H
#define NANA_NKP_READER_H

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

namespace nana
{
namespace nkp
{

enum ResourceType
{
    RESOURCE_UNKNOWN = 0,
    RESOURCE_NKT,
    RESOURCE_NKF,
    RESOURCE_TMF
};

struct Entry
{
    std::string name;
    uint32_t nameOffset;
    uint32_t dataOffset;
    uint32_t size;
    ResourceType type;
    bool signatureValid;
};

class Reader
{
public:
    Reader();

    bool open(const void *buffer, size_t bufferSize);
    void close();

    bool isOpen() const;
    uint32_t declaredSize() const;
    uint32_t resourceCount() const;
    size_t fileSize() const;

    const std::vector<Entry>& entries() const;
    const Entry* find(const char *resourceName) const;
    const uint8_t* resourceData(const Entry& entry) const;

    const char* error() const;

private:
    const uint8_t *m_buffer;
    size_t m_bufferSize;
    uint32_t m_declaredSize;
    std::vector<Entry> m_entries;
    std::string m_error;

    bool fail(const char *message);
};

} // namespace nkp
} // namespace nana

#endif
