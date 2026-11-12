
#include "nkp_reader.h"

#include <string.h>
#include <stdio.h>

namespace nana
{
namespace nkp
{

static uint32_t readU32LE(const uint8_t *p)
{
    return (uint32_t)p[0]
        | ((uint32_t)p[1] << 8)
        | ((uint32_t)p[2] << 16)
        | ((uint32_t)p[3] << 24);
}

static ResourceType detectType(const std::string& name)
{
    size_t dot = name.find_last_of('.');

    if (dot == std::string::npos)
        return RESOURCE_UNKNOWN;

    std::string ext = name.substr(dot + 1);

    if (ext == "nkt") return RESOURCE_NKT;
    if (ext == "nkf") return RESOURCE_NKF;
    if (ext == "tmf") return RESOURCE_TMF;

    return RESOURCE_UNKNOWN;
}

Reader::Reader()
    : m_buffer(0),
      m_bufferSize(0),
      m_declaredSize(0)
{
}

bool Reader::fail(const char *message)
{
    m_error = message;
    return false;
}

void Reader::close()
{
    m_buffer = 0;
    m_bufferSize = 0;
    m_declaredSize = 0;
    m_entries.clear();
    m_error.clear();
}

bool Reader::open(const void *buffer, size_t bufferSize)
{
    close();

    if (!buffer)
        return fail("Null input buffer");

    if (bufferSize < 16)
        return fail("File too small");

    m_buffer = (const uint8_t *)buffer;
    m_bufferSize = bufferSize;

    if (memcmp(m_buffer, "NKP\x1A", 4) != 0)
        return fail("Invalid NKP signature");

    m_declaredSize = readU32LE(m_buffer + 4);

    uint32_t count = readU32LE(m_buffer + 12);

    if (count > 4096)
        return fail("Resource count too large");

    if (m_declaredSize > bufferSize)
        return fail("Declared size exceeds file size");

    if (16ULL + (uint64_t)count * 8ULL > bufferSize)
        return fail("Directory exceeds file bounds");

    for (uint32_t i = 0; i < count; ++i)
    {
        size_t entryPos = 16 + (size_t)i * 8;

        uint32_t nameOffset = readU32LE(m_buffer + entryPos);
        uint32_t dataOffset = readU32LE(m_buffer + entryPos + 4);

        if (nameOffset >= m_declaredSize ||
            dataOffset >= m_declaredSize)
        {
            return fail("Entry offset outside declared file size");
        }

        const uint8_t *nameStart = m_buffer + nameOffset;
        size_t remaining = m_declaredSize - nameOffset;

        const uint8_t *terminator =
            (const uint8_t *)memchr(nameStart, 0, remaining);

        if (!terminator)
            return fail("Resource name is not terminated");

        Entry entry;

        entry.name.assign(
            (const char *)nameStart,
            (size_t)(terminator - nameStart)
        );

        entry.nameOffset = nameOffset;
        entry.dataOffset = dataOffset;
        entry.size = 0;
        entry.type = detectType(entry.name);
        entry.signatureValid = false;

        m_entries.push_back(entry);
    }

    for (size_t i = 0; i < m_entries.size(); ++i)
    {
        Entry& entry = m_entries[i];

        uint32_t nextOffset = m_declaredSize;

        for (size_t j = 0; j < m_entries.size(); ++j)
        {
            uint32_t candidate = m_entries[j].dataOffset;

            if (candidate > entry.dataOffset &&
                candidate < nextOffset)
            {
                nextOffset = candidate;
            }
        }

        if (nextOffset <= entry.dataOffset)
            return fail("Invalid resource boundaries");

        entry.size = nextOffset - entry.dataOffset;

        if ((uint64_t)entry.dataOffset + entry.size >
            m_declaredSize)
        {
            return fail("Resource exceeds declared file bounds");
        }

        const uint8_t *p = m_buffer + entry.dataOffset;

        if (entry.size >= 4)
        {
            if (entry.type == RESOURCE_NKT &&
                memcmp(p, "NKT\x1A", 4) == 0)
            {
                entry.signatureValid = true;
            }
            else if (entry.type == RESOURCE_NKF &&
                     memcmp(p, "NF2\x1A", 4) == 0)
            {
                entry.signatureValid = true;
            }
        }
    }

    m_error.clear();
    return true;
}

bool Reader::isOpen() const
{
    return m_buffer != 0 && !m_entries.empty();
}

uint32_t Reader::declaredSize() const
{
    return m_declaredSize;
}

uint32_t Reader::resourceCount() const
{
    return (uint32_t)m_entries.size();
}

size_t Reader::fileSize() const
{
    return m_bufferSize;
}

const std::vector<Entry>& Reader::entries() const
{
    return m_entries;
}

const Entry* Reader::find(const char *resourceName) const
{
    if (!resourceName)
        return 0;

    for (size_t i = 0; i < m_entries.size(); ++i)
    {
        if (m_entries[i].name == resourceName)
            return &m_entries[i];
    }

    return 0;
}

const uint8_t* Reader::resourceData(const Entry& entry) const
{
    if (!m_buffer)
        return 0;

    if ((uint64_t)entry.dataOffset + entry.size >
        m_declaredSize)
    {
        return 0;
    }

    return m_buffer + entry.dataOffset;
}

const char* Reader::error() const
{
    return m_error.c_str();
}

} // namespace nkp
} // namespace nana
