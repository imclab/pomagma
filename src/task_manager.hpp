#ifndef POMAGMA_TASK_MANAGER_HPP
#define POMAGMA_TASK_MANAGER_HPP

#include "util.hpp"

namespace pomagma
{

class NullaryFunction;
class UnaryFunction;
class BinaryFunction;
class SymmetricFunction;


struct EquationTask
{
    oid_t dep;

    EquationTask () {}
    EquationTask (oid_t d) : dep(d) {}
};

struct PositiveOrderTask
{
    oid_t lhs;
    oid_t rhs;

    PositiveOrderTask () {}
    PositiveOrderTask (oid_t l, oid_t r) : lhs(l), rhs(r) {}
};

struct NegativeOrderTask
{
    oid_t lhs;
    oid_t rhs;

    NegativeOrderTask () {}
    NegativeOrderTask (oid_t l, oid_t r) : lhs(l), rhs(r) {}
};

struct NullaryFunctionTask
{
    NullaryFunction * fun;

    NullaryFunctionTask () {}
    NullaryFunctionTask (NullaryFunction * f) : fun(f) {}
};

struct UnaryFunctionTask
{
    UnaryFunction * fun;
    oid_t arg;

    UnaryFunctionTask () {}
    UnaryFunctionTask (UnaryFunction * f, oid_t a) : fun(f), arg(a) {}
};

struct BinaryFunctionTask
{
    BinaryFunction * fun;
    oid_t lhs;
    oid_t rhs;

    BinaryFunctionTask () {}
    BinaryFunctionTask (BinaryFunction * f, oid_t l, oid_t r)
        : fun(f), lhs(l), rhs(r)
    {}
};

struct SymmetricFunctionTask
{
    SymmetricFunction * fun;
    oid_t lhs;
    oid_t rhs;

    SymmetricFunctionTask () {}
    SymmetricFunctionTask (SymmetricFunction * f, oid_t l, oid_t r)
        : fun(f), lhs(l), rhs(r)
    {}
};


// these are defined by the TaskManager and called by the user
void enqueue (const EquationTask & task);
void enqueue (const PositiveOrderTask & task);
void enqueue (const NegativeOrderTask & task);
void enqueue (const NullaryFunctionTask & task);
void enqueue (const UnaryFunctionTask & task);
void enqueue (const BinaryFunctionTask & task);
void enqueue (const SymmetricFunctionTask & task);


// these are defined by the user and called by the TaskManager
void execute (const EquationTask & task);
void execute (const PositiveOrderTask & task);
void execute (const NegativeOrderTask & task);
void execute (const NullaryFunctionTask & task);
void execute (const UnaryFunctionTask & task);
void execute (const BinaryFunctionTask & task);
void execute (const SymmetricFunctionTask & task);


namespace TaskManager
{

void start (size_t thread_count);
void stopall ();

} // namespace TaskManager


} // namespace pomagma

#endif // POMAGMA_TASK_MANAGER_HPP
