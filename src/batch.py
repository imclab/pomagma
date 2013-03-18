import os
import shutil
import parsable
import pomagma.util
import pomagma.wrapper


parsable_commands = []
def parsable_command(fun):
    parsable_commands.append(fun)
    return fun


@parsable_command
def test(theory):
    '''
    Test basic operations with a given theory:
    init, copy, grow, aggregate
    '''
    buildtype = 'debug' if pomagma.util.debug else 'release'
    data = '{}.{}.test'.format(theory, buildtype)
    data = os.path.join(pomagma.util.DATA, data)
    if os.path.exists(data):
        os.system('rm -f {}/*'.format(data))
    else:
        os.makedirs(data)
    with pomagma.util.chdir(data):

        min_size = pomagma.util.MIN_SIZES[theory]
        dsize = min(512, 1 + min_size)
        sizes = [min_size + i * dsize for i in range(10)]
        opts = dict(log_file='test.log')

        pomagma.wrapper.init(theory, '0.h5', sizes[0], **opts)
        pomagma.wrapper.copy('0.h5', '1.h5', **opts)
        pomagma.wrapper.grow(theory, '1.h5', '2.h5', sizes[1], **opts)
        pomagma.wrapper.init(theory, '3.h5', sizes[0], **opts)
        pomagma.wrapper.aggregate('2.h5', '3.h5', '4.h5', **opts)
        pomagma.wrapper.aggregate('4.h5', '0.h5', '5.h5', **opts)
        digest4 = pomagma.util.get_hash('4.h5')
        digest5 = pomagma.util.get_hash('5.h5')
        assert digest4 == digest5


if __name__ == '__main__':
    map(parsable.command, parsable_commands)
    parsable.dispatch()
