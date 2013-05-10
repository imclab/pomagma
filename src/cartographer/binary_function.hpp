#pragma once

#include "util.hpp"
#include "base_bin_rel.hpp"
#include <pomagma/util/sequential_dense_set.hpp>
#include <unordered_map>
#include <google/sparse_hash_map>

namespace pomagma
{

class BinaryFunction : noncopyable
{
    // DEPRECATED
    mutable base_bin_rel m_lines;
    mutable std::unordered_map<std::pair<Ob, Ob>, Ob, ObPairHash> m_values;

    // TODO
    typedef std::vector<google::sparse_hash_map<Ob, Ob>> Map;
    mutable Map m_lhs_rhs_val;
    mutable Map m_rhs_lhs_val;

public:

    BinaryFunction (Carrier & carrier);
    void validate () const;
    void log_stats () const;

    // raw operations
    static bool is_symmetric () { return false; }
    size_t count_pairs () const { return m_values.size(); }
    void raw_insert (Ob lhs, Ob rhs, Ob val);
    void update() { m_lines.copy_Lx_to_Rx(); }
    void clear ();

    // safe operations
    // m_values is source of truth; m_lines lag
    DenseSet get_Lx_set (Ob lhs) const { return m_lines.Lx_set(lhs); }
    DenseSet get_Rx_set (Ob rhs) const { return m_lines.Rx_set(rhs); }
    bool defined (Ob lhs, Ob rhs) const;
    Ob find (Ob lhs, Ob rhs) const;
    Ob raw_find (Ob lhs, Ob rhs) const { return find(lhs, rhs); }
    DenseSet::Iterator iter_lhs (Ob lhs) const;
    DenseSet::Iterator iter_rhs (Ob rhs) const;
    void insert (Ob lhs, Ob rhs, Ob val) const;
    void update_values () const; // postcondition: all values are reps

    // unsafe operations
    void unsafe_merge (const Ob dep);

private:

    const Carrier & carrier () const { return m_lines.carrier(); }
    const DenseSet & support () const { return m_lines.support(); }
    size_t item_dim () const { return support().item_dim(); }
};

inline bool BinaryFunction::defined (Ob lhs, Ob rhs) const
{
    POMAGMA_ASSERT5(support().contains(lhs), "unsupported lhs: " << lhs);
    POMAGMA_ASSERT5(support().contains(rhs), "unsupported rhs: " << rhs);
    return m_lines.get_Lx(lhs, rhs);
}

inline Ob BinaryFunction::find (Ob lhs, Ob rhs) const
{
    POMAGMA_ASSERT5(support().contains(lhs), "unsupported lhs: " << lhs);
    POMAGMA_ASSERT5(support().contains(rhs), "unsupported rhs: " << rhs);
    auto i = m_values.find(std::make_pair(lhs, rhs));
    return i == m_values.end() ? 0 : i->second;
    //if (i == m_values.end()) {
    //    return 0;
    //} else {
    //    Ob & val = i->second;
    //    Ob rep = carrier().find(val);
    //    if (rep != val) {
    //        val = rep;
    //    }
    //    return rep;
    //}
}

inline DenseSet::Iterator BinaryFunction::iter_lhs (Ob lhs) const
{
    POMAGMA_ASSERT5(support().contains(lhs), "unsupported lhs: " << lhs);
    return DenseSet::Iterator(item_dim(), m_lines.Lx(lhs));
}

inline DenseSet::Iterator BinaryFunction::iter_rhs (Ob rhs) const
{
    POMAGMA_ASSERT5(support().contains(rhs), "unsupported rhs: " << rhs);
    return DenseSet::Iterator(item_dim(), m_lines.Rx(rhs));
}

inline void BinaryFunction::raw_insert (Ob lhs, Ob rhs, Ob val)
{
    POMAGMA_ASSERT5(support().contains(lhs), "unsupported lhs: " << lhs);
    POMAGMA_ASSERT5(support().contains(rhs), "unsupported rhs: " << rhs);
    POMAGMA_ASSERT5(support().contains(val), "unsupported val: " << val);

    m_values.insert(std::make_pair(std::make_pair(lhs, rhs), val));
    m_lines.Lx(lhs, rhs).one();
}

inline void BinaryFunction::insert (Ob lhs, Ob rhs, Ob val) const
{
    POMAGMA_ASSERT5(support().contains(lhs), "unsupported lhs: " << lhs);
    POMAGMA_ASSERT5(support().contains(rhs), "unsupported rhs: " << rhs);
    POMAGMA_ASSERT5(support().contains(val), "unsupported val: " << val);

    Ob & val_ref = m_values[std::make_pair(lhs, rhs)];
    if (val_ref) {
        carrier().set_and_merge(val_ref, val);
    } else {
        val_ref = val;
        m_lines.Lx(lhs, rhs).one();
        m_lines.Rx(lhs, rhs).one();
    }
}

} // namespace pomagma
