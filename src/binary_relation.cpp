#include "binary_relation.hpp"
#include "aligned_alloc.hpp"
#include <cstring>

namespace pomagma
{

BinaryRelation::BinaryRelation (const Carrier & carrier)
    : m_lines(carrier)
{
    POMAGMA_DEBUG("creating BinaryRelation with "
            << round_word_dim() << " words");
}

BinaryRelation::~BinaryRelation ()
{
}

void BinaryRelation::move_from (
        const BinaryRelation & other,
        const oid_t * new2old)
{
    POMAGMA_DEBUG("Copying BinaryRelation");

    if (POMAGMA_DEBUG_LEVEL >= 1) other.validate();

    // WARNING: assumes this has been done
    //bzero(m_lines.Lx(), sizeof(Word) * data_size_words());
    //bzero(m_lines.Rx(), sizeof(Word) * data_size_words());

    if (new2old == NULL) {
        POMAGMA_DEBUG("copying by column and by row");
        m_lines.move_from(other.m_lines);
    } else {
        POMAGMA_DEBUG("copying and reordering");
        // copy & reorder WIKKIT SLOW
        for (oid_t i_new = 1; i_new <= item_dim(); ++i_new) {
            if (not supports(i_new)) continue;
            oid_t i_old = new2old[i_new];

            for (oid_t j_new = 1; j_new <= item_dim(); ++j_new) {
                if (not supports(j_new)) continue;
                oid_t j_old = new2old[j_new];

                if (other.contains(i_old, j_old)) insert(i_new, j_new);
            }
        }
    }
    if (POMAGMA_DEBUG_LEVEL >= 1) validate();
}

//----------------------------------------------------------------------------
// Diagnostics

// supa-slow, try not to use
size_t BinaryRelation::count_pairs () const
{
    size_t result = 0;
    for (DenseSet::Iter i(support()); i.ok(); i.next()) {
        result += get_Lx_set(*i).count_items();
    }
    return result;
}

void BinaryRelation::validate () const
{
    POMAGMA_DEBUG("Validating BinaryRelation");

    m_lines.validate();

    size_t num_pairs = 0;

    DenseSet Lx(round_item_dim(), NULL);
    DenseSet Rx(round_item_dim(), NULL);
    for (oid_t i = 1; i <= item_dim(); ++i) {
        bool sup_i = supports(i);
        Lx.init(m_lines.Lx(i));

        for (oid_t j = 1; j <= item_dim(); ++j) {
            bool sup_ij = sup_i and supports(j);
            Rx.init(m_lines.Rx(j));

            bool Lx_ij = Lx.contains(j);
            bool Rx_ij = Rx.contains(i);
            num_pairs += Rx_ij;

            POMAGMA_ASSERT(Lx_ij == Rx_ij,
                    "Lx,Rx disagree at " << i << "," << j
                    << ", Lx is " << Lx_ij << ", Rx is " << Rx_ij  );

            POMAGMA_ASSERT(sup_ij or not Lx_ij,
                    "Lx unsupported at " << i << "," << j );

            POMAGMA_ASSERT(sup_ij or not Rx_ij,
                    "Rx unsupported at " << i << "," << j );
        }
    }

    size_t true_size = count_pairs();
    POMAGMA_ASSERT(num_pairs == true_size,
            "incorrect number of pairs: "
            << num_pairs << " should be " << true_size);
}

void BinaryRelation::validate_disjoint (const BinaryRelation & other) const
{
    POMAGMA_DEBUG("Validating disjoint pair of BinaryRelations");

    // validate supports agree
    POMAGMA_ASSERT_EQ(support().item_dim(), other.support().item_dim());
    POMAGMA_ASSERT_EQ(
            support().count_items(),
            other.support().count_items());
    POMAGMA_ASSERT(support() == other.support(),
            "BinaryRelation supports differ");

    // validate disjointness
    DenseSet this_set(item_dim(), NULL);
    DenseSet other_set(item_dim(), NULL);
    for (DenseSet::Iter i(support()); i.ok(); i.next()) {
        this_set.init(m_lines.Lx(*i));
        other_set.init(other.m_lines.Lx(*i));
        POMAGMA_ASSERT(this_set.disjoint(other_set),
                "BinaryRelations intersect at row " << *i);
    }
}

void BinaryRelation::print_table (size_t n) const
{
    if (n == 0) n = item_dim();
    for (oid_t i = 1; i <= n; ++i) {
        std::cout << '\n';
        for (oid_t j = 1; j <= n; ++j) {
            std::cout << (contains(i, j) ? 'O' : '.');
        }
    }
    std::cout << std::endl;
}

//----------------------------------------------------------------------------
// Operations

void BinaryRelation::remove_Lx (const DenseSet & is, oid_t j)
{
    // slower version
    //for (DenseSet::Iter i(is); i.ok(); i.next()) {
    //    remove_Lx(*i, j);
    //}

    // faster version
    Word mask = ~(Word(1) << (j % BITS_PER_WORD));
    size_t offset = j / BITS_PER_WORD;
    Word * lines = m_lines.Lx() + offset;
    for (DenseSet::Iter i(is); i.ok(); i.next()) {
         lines[*i * round_word_dim()] &= mask; // ATOMIC
    }
}

void BinaryRelation::remove_Rx (oid_t i, const DenseSet& js)
{
    // slower version
    //for (DenseSet::Iter j(js); j.ok(); j.next()) {
    //    remove_Rx(i, *j);
    //}

    // faster version
    Word mask = ~(Word(1) << (i % BITS_PER_WORD));
    size_t offset = i / BITS_PER_WORD;
    Word * lines = m_lines.Rx() + offset;
    for (DenseSet::Iter j(js); j.ok(); j.next()) {
         lines[*j * round_word_dim()] &= mask; // ATOMIC
    }
}

void BinaryRelation::remove (oid_t i)
{
    DenseSet set(item_dim(), NULL);

    // remove column
    set.init(m_lines.Lx(i));
    remove_Rx(i, set);
    set.zero();

    // remove row
    set.init(m_lines.Rx(i));
    remove_Lx(set, i);
    set.zero();
}

void BinaryRelation::ensure_inserted (
        oid_t i,
        const DenseSet & js,
        void (*change)(oid_t, oid_t))
{
    DenseSet diff(item_dim());
    DenseSet dest(item_dim(), m_lines.Lx(i));
    if (dest.ensure(js, diff)) {
        for (DenseSet::Iter k(diff); k.ok(); k.next()) {
            insert_Rx(i, *k);
            change(i, *k);
        }
    }
}

void BinaryRelation::ensure_inserted (
        const DenseSet & is,
        oid_t j,
        void (*change)(oid_t, oid_t))
{
    DenseSet diff(item_dim());
    DenseSet dest(item_dim(), m_lines.Rx(j));
    if (dest.ensure(is, diff)) {
        for (DenseSet::Iter k(diff); k.ok(); k.next()) {
            insert_Lx(*k, j);
            change(*k, j);
        }
    }
}

// policy: call move_to if i~k but not j~k
void BinaryRelation::merge (
        oid_t i, // dep
        oid_t j, // rep
        void (*move_to)(oid_t, oid_t)) // typically enforce_
{
    POMAGMA_ASSERT4(j != i, "BinaryRelation tried to merge item with self");

    DenseSet diff(item_dim());
    DenseSet rep(item_dim(), NULL);
    DenseSet dep(item_dim(), NULL);

    // merge rows (i, _) into (j, _)
    dep.init(m_lines.Lx(i));
    remove_Rx(i, dep);
    rep.init(m_lines.Lx(j));
    if (rep.merge(dep, diff)) {
        for (DenseSet::Iter k(diff); k.ok(); k.next()) {
            insert_Rx(j, *k);
            move_to(j, *k);
        }
    }

    // merge cols (_, i) into (_, j)
    dep.init(m_lines.Rx(i));
    remove_Lx(dep, i);
    rep.init(m_lines.Rx(j));
    if (rep.merge(dep, diff)) {
        for (DenseSet::Iter k(diff); k.ok(); k.next()) {
            insert_Lx(*k, j);
            move_to(*k, j);
        }
    }
}

// saving/loading, quicker rather than smaller
inline void safe_fread (void * ptr, size_t size, size_t count, FILE * file)
{
    size_t read = fread(ptr, size, count, file);
    POMAGMA_ASSERT(read == count, "fread failed");
}
inline void safe_fwrite (const void * ptr, size_t size, size_t count, FILE * file)
{
    size_t written = fwrite(ptr, size, count, file);
    POMAGMA_ASSERT(written == count, "fwrite failed");
}

oid_t BinaryRelation::data_size () const
{
    return 2 * sizeof(Word) * data_size_words();
}
void BinaryRelation::write_to_file (FILE * file)
{
    safe_fwrite(m_lines.Lx(), sizeof(Word), data_size_words(), file);
    safe_fwrite(m_lines.Rx(), sizeof(Word), data_size_words(), file);
}
void BinaryRelation::read_from_file (FILE * file)
{
    // WARNING assumes support is full
    safe_fread(m_lines.Lx(), sizeof(Word), data_size_words(), file);
    safe_fread(m_lines.Rx(), sizeof(Word), data_size_words(), file);
}

// iteration
void BinaryRelation::iterator::_find_rhs ()
{
    while (m_lhs.ok()) {
        m_rhs_set.init(m_rel.m_lines.Lx(*m_lhs));
        m_rhs.begin();
        if (m_rhs.ok()) {
            _update_rhs();
            _update_lhs();
            POMAGMA_ASSERT5(m_rel.contains(m_pos),
                    "BinaryRelation::iterator landed outside of relation: "
                    << m_pos.lhs << "," << m_pos.rhs);
            return;
        }
        m_lhs.next();
    }
    _finish();
}

} // namespace pomagma
