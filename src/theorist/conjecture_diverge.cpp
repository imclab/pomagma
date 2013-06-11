#include "conjecture_diverge.hpp"
#include <pomagma/macrostructure/carrier.hpp>
#include <pomagma/macrostructure/binary_relation.hpp>
#include <pomagma/macrostructure/nullary_function.hpp>
#include <pomagma/macrostructure/router.hpp>
#include <pomagma/language/language.hpp>
#include <algorithm>

namespace pomagma
{

namespace detail
{

std::vector<Ob> conjecture_diverge (
        Structure & structure,
        const std::vector<float> & probs,
        const std::vector<std::string> & routes,
        const char * conjectures_file)
{
    POMAGMA_INFO("Conjecturing divergent terms");
    const Carrier & carrier = structure.carrier();
    const BinaryRelation & NLESS = structure.binary_relation("NLESS");
    const Ob BOT = structure.nullary_function("BOT").find();

    POMAGMA_DEBUG("collecting conjectures");
    DenseSet conjecture_set(carrier.item_count());
    conjecture_set = NLESS.get_Lx_set(BOT);
    conjecture_set.complement();
    conjecture_set.remove(BOT);
    std::vector<Ob> conjectures;
    for (auto iter = conjecture_set.iter(); iter.ok(); iter.next()) {
        Ob ob = * iter;
        conjectures.push_back(ob);
    }

    POMAGMA_DEBUG("sorting " << conjectures.size() << " conjectures");
    std::sort(
        conjectures.begin(),
        conjectures.end(),
        [&](const Ob & x, const Ob & y){ return probs[x] > probs[y]; });

    POMAGMA_DEBUG("writing conjectures to " << conjectures_file);
    std::ofstream file(conjectures_file, std::ios::out | std::ios::trunc);
    POMAGMA_ASSERT(file, "failed to open " << conjectures_file);
    file << "# divergence conjectures generated by pomagma";
    for (auto ob : conjectures) {
        file << "\nEQUAL BOT " << routes[ob];
    }

    return conjectures;
}

} // namespace detail

void conjecture_diverge (
        Structure & structure,
        const char * language_file,
        const char * conjectures_file)
{
    const auto language = load_language(language_file);
    Router router(structure, language);
    const std::vector<float> probs = router.measure_probs();
    const std::vector<std::string> routes = router.find_routes();

    detail::conjecture_diverge(
        structure,
        probs,
        routes,
        conjectures_file);
}

} // namespace pomagma
