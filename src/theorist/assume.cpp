#include "assume.hpp"
#include "consistency.hpp"
#include "find_parser.hpp"
#include <pomagma/macrostructure/binary_relation.hpp>
#include <pomagma/macrostructure/scheduler.hpp>
#include <pomagma/macrostructure/compact.hpp>
#include <algorithm>

namespace pomagma
{

namespace detail
{

std::map<std::string, size_t> assume_facts (
        Structure & structure,
        const char * theory_file)
{
    POMAGMA_INFO("assuming core facts");

    size_t pos_count = 0;
    size_t neg_count = 0;

    for (LineParser iter(theory_file); iter.ok(); iter.next()) {
        const std::string & expression = * iter;
        POMAGMA_DEBUG("assume " << expression);

        FindParser parser(structure.signature());
        parser.begin(expression);
        std::string type = parser.parse_token();
        Ob lhs = parser.parse_term();
        Ob rhs = parser.parse_term();
        parser.end();

        if (type == "EQUAL") {
            if (lhs != rhs) {
                structure.carrier().ensure_equal(lhs, rhs);
                ++pos_count;
            }
        } else {
            BinaryRelation & rel = structure.binary_relation(type);
            if (not rel.find(lhs, rhs)) {
                rel.insert(lhs, rhs);
                ++(type[0] == 'N' ? neg_count : pos_count);
            }
        }
    }

    std::map<std::string, size_t> counts;
    counts["pos"] = pos_count;
    counts["neg"] = neg_count;
    return counts;
}

} // namespace detail

std::map<std::string, size_t> assume (
        Structure & structure,
        const char * theory_file)
{
    POMAGMA_INFO("Assuming statements");

    int pre_item_count = structure.carrier().item_count();

    configure_scheduler_to_merge_if_consistent(structure);
    auto counts = detail::assume_facts(structure, theory_file);
    process_mergers(structure.signature());
    compact(structure);

    int post_item_count = structure.carrier().item_count();
    int merge_count = pre_item_count - post_item_count;
    POMAGMA_INFO("assumption merged " << merge_count << " obs");

    counts["merge"] = merge_count;
    return counts;
}

} // namespace pomagma
