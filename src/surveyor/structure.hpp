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
    void load (const std::string & filename);
    void dump (const std::string & filename);
};

} // namespace pomagma
