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
    - Implement core writer (for Hindley's extensionality axioms)
- Structure
    - Implement persistent serializer
    - Create core structures from theory
- Compiler
    - Get skj.theory.cpp to compile (probably by fixing pomagma/compiler.py)
- Grower
    - Figure out how to set item_dim at runtime
    - Make Carrier::unsafe_insert safe as try_insert()
    - Implement expression sampler
    - Implement dump, load operations
    - Implement language reading & data structure
    - Flesh out unit test to exercise all methods
        - Add tests for DenseSet::Iterator2, 3
    - Add full test that builds 14400-element H4 group
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

- Minimum: run grower system tests
- Interactive: implement navigator as web-app
- Scalable: implement aggregator, trimmer
- Evolvable: implement linguist, theorist

System Architecture
-------------------

![Architecture](pomagma/raw/master/doc/architecture.png)

