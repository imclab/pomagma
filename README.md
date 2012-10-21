Pomagma
=======

A deductive database for partially ordered magmas.<br />
An experiment in coding under extensional semantics.<br />
A lens into a supremely beautiful mathematical object.<br />
A toy model of Lakatosian mathematical evolution.<br />
A mashup of first-person-shooter with theorem prover.<br />
A distributed rewrite of [Johann](Johann) with an artsy front-end.

Roadmap
-------

- Language
    - Add langauges to git: sk, skj
- Theory
    - Implement fact compiler to enumerate Hindley's extensionality axioms
- Structure
    - Implement persistent serializer
- Compiler
    - Get skj.theory.cpp to compile (probably by fixing pomagma/compiler.py)
- Grower
    - Implement dump, load operations
    - Flesh out unit test to exercise all methods
        - Add tests for DenseSet::Iterator2, 3
- Aggregator
    - Implement following merge logic in grower
    - Assume only forward-mapping tables
        - Work out injective function merge logic
    - Use 32-bit ob indices (how big a structure can we stored in memory?)
- Trimmer
    - Read globe; randomly prune to given size; write
- Navigator Server
    - Adapt syntactic algorithms from [Johann](http://github.com/fritzo/Johann)
- Navigator Client
    - Implement HTML5 UI
    - Implement native mobile UIs
- Theorist
    - Adapt auto conjecturing algorithms from [Johann](Johann)
    - Implement via CUDA/GPU or Eigen+OpenMP
- Linguist
    - Adapt language optimization algorithms from [Johann](http://github.com/fritzo/Johann)
    - Implement via CUDA/GPU or Eigen+OpenMP
- Controller
    - Implement master controller via python
    - Use boto to provision machines

Milestones
----------

- Minimum: run grower system tests (h4, sk, skj)
- Interactive: implement navigator as web-app
- Scalable: implement aggregator, trimmer
- Evolvable: implement linguist, theorist

System Architecture
-------------------

![Architecture](pomagma/raw/master/doc/architecture.png)

- Language - A probabilistic grammar defining an algebra's generators
- Theory - Inference rules and facts definint an algebra's relations
- Structure - A finite piece representing the target algebra
- Compiler - An optimizing compiler for forward-chaining inference
- Grower - A parallel Todd-Coxeter rules engine
- Aggregator - Joins charts from growers into a global atlas
- Trimmer - Cuts off pieces of the structure for further growth
- Navigator - The user interface for exploring the mapped algebra
- Theorist - Statistically conjectures new relations
- Linguist - A Bayesian grammar optimizer / MCMC sampler

