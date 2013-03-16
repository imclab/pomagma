import re
import parsable
from pomagma.compiler.util import TODO
from pomagma.compiler import parser
from pomagma.compiler.compiler import add_costs, get_events, compile_full, compile_given
from pomagma.compiler.extensional import derive_facts, validate
from pomagma.compiler import cpp


def print_compiles(compiles):
    for cost, strategy in compiles:
        print '# cost = {0}'.format(cost)
        print re.sub(': ', '\n', repr(strategy))
        print


def measure_sequent(sequent):
    print '-' * 78
    print 'Compiling full search: {0}'.format(sequent)
    compiles = compile_full(sequent)
    print_compiles(compiles)
    full_cost = add_costs(*[cost for cost, _ in compiles])

    incremental_cost = None
    for event in get_events(sequent):
        print 'Compiling incremental search given: {0}'.format(event)
        compiles = compile_given(sequent, event)
        print_compiles(compiles)
        if event.args:
            cost = add_costs(*[cost for cost, _ in compiles])
            if incremental_cost:
                incremental_cost = add_costs(incremental_cost, cost)
            else:
                incremental_cost = cost
        else:
            pass  # event is only triggered once, so ignore cost

    print '# full cost =', full_cost, 'incremental cost =', incremental_cost


@parsable.command
def contrapositves(*filenames):
    '''
    Close rules under contrapositve
    '''
    sequents = []
    for filename in filenames:
        sequents += parser.parse_rules(filename)
    for sequent in sequents:
        print sequent.ascii()
        print
        for neg in sequent.contrapositives():
            print neg.ascii(indent=4)
            print


@parsable.command
def measure(*filenames):
    '''
    Measure complexity of rules in files
    '''
    sequents = []
    for filename in filenames:
        sequents += parser.parse_rules(filename)
    for sequent in sequents:
        measure_sequent(sequent)


@parsable.command
def report(*filenames):
    '''
    Make report.html of rule complexity in files
    '''
    TODO('write sequents to file, coloring by incremental complexity')


@parsable.command
def test_compile(*filenames):
    '''
    Compile rules -> C++
    '''
    for stem_rules in filenames:
        assert stem_rules[-6:] == '.rules', stem_rules

        sequents = parser.parse_rules(stem_rules)
        for sequent in sequents:
            for cost, strategy in compile_full(sequent):
                print '\n'.join(strategy.cpp_lines())

            for event in get_events(sequent):
                for cost, strategy in compile_given(sequent, event):
                    print '\n'.join(strategy.cpp_lines())


@parsable.command
def test_close_rules(infile, is_extensional=True):
    '''
    Compile extensionally some.rules -> some.derived.facts
    '''
    assert infile.endswith('.rules')
    rules = parser.parse_rules(infile)
    for rule in rules:
        print
        print '#', rule
        for fact in derive_facts(rule):
            print fact
            if is_extensional:
                validate(fact)


@parsable.command
def compile(*infiles, **kwargs):
    '''
    Compile rules -> C++.
    Optional keyword arguments:
        outfile=OUTFILE
        extensional=True
        negrelations=True   # TODO set default to False
    '''
    outfile = kwargs.get(
            'outfile',
            '{0}.theory.cpp'.format(infiles[-1].split('.')[0]))
    parse_bool = lambda s: {'true':True, 'false':False}[s.lower()]
    extensional = parse_bool(kwargs.get('extensional', 'true'))
    negrelations = parse_bool(kwargs.get('negrelations', 'true'))

    print '# writing', outfile
    argstring = ' '.join(
            list(infiles) +
            ['{0}={1}'.format(key, val) for key, val in kwargs.iteritems()])

    rules = []
    facts = []
    for infile in infiles:
        suffix = infile.split('.')[-1]
        if suffix == 'rules':
            rules += parser.parse_rules(infile)
        elif suffix == 'facts':
            facts += parser.parse_facts(infile)
        else:
            raise TypeError('unknown file type: %s' % infile)
    if extensional:
        for rule in rules:
            facts += derive_facts(rule)

    code = cpp.Code()
    code('''
        // This file was auto generated by pomagma using:
        // python run.py compile $argstring
        ''',
        argstring = argstring,
        ).newline()

    cpp.write_theory(code, rules, facts, negrelations=negrelations)

    with open(outfile, 'w') as f:
        f.write(str(code))


if __name__ == '__main__':
    parsable.dispatch()
