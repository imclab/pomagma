#include "util.hpp"
#include "inverse_bin_fun.hpp"
#include "binary_function.hpp"

using namespace pomagma;

void test_random (size_t size, float fill = 0.3)
{
    POMAGMA_INFO("Buiding fun,inv of size " << size);
    Carrier carrier(size);
    const DenseSet & support = carrier.support();
    for (Ob i = 1; i <= size; ++i) {
        carrier.insert();
    }
    for (Ob i = 1; i <= size; ++i) {
        if (random_bool(0.2)) {
            carrier.remove(i);
        }
    }

    BinaryFunction fun(carrier);
    inverse_bin_fun inv(carrier);
    fun.validate();
    inv.validate(fun);

    POMAGMA_INFO("testing insertion");
    size_t insert_count = size * size * fill;
    for (size_t n = 0; n < insert_count; ++n) {
        Ob lhs;
        Ob rhs;
        while (true) {
            lhs = random_int(1, size);
            if (not support.contains(lhs)) continue;
            rhs = random_int(1, size);
            if (not support.contains(rhs)) continue;
            if (fun.defined(lhs, rhs)) continue;
            break;
        }
        Ob val = random_int(1, size);

        fun.insert(lhs, rhs, val);
        inv.insert(lhs, rhs, val);
    }

    fun.validate();
    inv.validate(fun);
}

int main ()
{
    test_random(1 << 9);

    // TODO test with multiple threads

    return 0;
}
