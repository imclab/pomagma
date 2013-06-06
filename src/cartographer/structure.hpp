#pragma once

#include <pomagma/platform/util.hpp>
#include <pomagma/platform/signature.hpp>

namespace pomagma
{

class Structure : noncopyable
{
    Signature m_signature;

public:

    Structure () {}

    Signature & signature () { return m_signature; }
    Carrier & carrier () { return * m_signature.carrier(); }

    void validate ();
    void clear ();
    void load (const std::string & filename, size_t extra_item_dim = 0);
    void dump (const std::string & filename);
    void init_signature (Structure & other, size_t item_dim);
};

} // namespace pomagma
