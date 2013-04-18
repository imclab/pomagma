import os
import sys
import shutil
import subprocess
import multiprocessing
import contextlib
import uuid
import tempfile
import timeit
import tables

SRC = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SRC)
THEORY = os.path.join(SRC, 'theory')
LANGUAGE = os.path.join(SRC, 'language')
DATA = os.path.join(ROOT, 'data')
debug = 'POMAGMA_DEBUG' in os.environ
if debug:
    print 'Running in debug mode'
    BUILD = os.path.join(ROOT, 'build', 'debug')
else:
    BUILD = os.path.join(ROOT, 'build', 'release')
BIN = os.path.join(BUILD, 'src')
CPU_COUNT = multiprocessing.cpu_count()

GROWERS = {
    'h4': 'h4.grow',
    'sk': 'sk.grow',
    'skj': 'skj.grow',
    }

MIN_SIZES = {
    'h4': 127,
    'sk': 1023,
    'skj': 1535,
    }


def print_dot(out=sys.stdout):
    out.write('.')
    out.flush()


def random_uuid():
    return str(uuid.uuid4())


@contextlib.contextmanager
def chdir(path):
    old_path = os.path.abspath(os.path.curdir)
    try:
        #os.makedirs(path)
        print 'cd {}\n'.format(path)
        os.chdir(path)
        yield os.path.curdir
    finally:
        print 'cd {}\n'.format(old_path)
        os.chdir(old_path)


@contextlib.contextmanager
def in_temp_dir():
    path = os.path.abspath(tempfile.mkdtemp())
    try:
        with chdir(path):
            yield path
    finally:
        shutil.rmtree(path)


@contextlib.contextmanager
def log_duration():
    try:
        start_time = timeit.default_timer()
        yield
    finally:
        duration = timeit.default_timer() - start_time
        print '# took {:0.3g} sec'.format(duration)


@contextlib.contextmanager
def load(filename):
    structure = tables.openFile(filename)
    yield structure
    structure.close()


def abspath(path):
    return os.path.abspath(os.path.expanduser(path))


def get_log_file(options):
    log_file = os.path.join(DATA, 'default.log')
    log_file = abspath(options.get('log_file', log_file))
    return log_file


def log_print(message, log_file):
    with open(log_file, 'a') as log:
        log.write(message)
        log.write('\n')
    print message


def make_env(options):
    options = dict(options)
    options['root'] = ROOT
    options.setdefault('threads', CPU_COUNT)
    log_file = get_log_file(options)
    options['log_file'] = log_file
    log_dir = os.path.dirname(log_file)
    if not os.path.exists(log_dir):
        os.makedirs(log_dir)
    options.setdefault('log_level', 9 if debug else 0)
    env = os.environ.copy()
    for key, val in options.iteritems():
        pomagma_key = 'POMAGMA_{}'.format(key.upper())
        sys.stderr.write('{}={} \\\n'.format(pomagma_key, val))
        env[pomagma_key] = str(val)
    return env


def check_call(*args):
    message = '{}\n'.format(' \\\n'.join(args))
    sys.stderr.write(message)
    with log_duration():
        info = subprocess.call(args)
    if info:
        sys.stderr.write('ERROR in {}'.format(message))
        sys.exit(info)


def print_logged_error(log_file):
    print
    print '==== LOG FILE ===='
    grep = ' '.join([
        'grep',
        '--before-context=40',
        '--after-context=3',
        '--ignore-case',
        '--color=always',
        '--text',
        '--max-count=1',
        'error'
        ])
    revgrep = 'tac {} | {} | tac'.format(log_file, grep)
    subprocess.call(revgrep, shell=True)


def get_stack_trace(binary):
    trace = '==== STACK TRACE ====\n'
    try:
        trace += subprocess.check_output([
            'gdb',
            binary,
            'core',
            '--batch',
            '-ex',
            'thread apply all bt',
            ])
    except subprocess.CalledProcessError:
        trace += 'ERROR stack trace failed'
    return trace


def log_call(*args, **options):
    args = map(str, args)
    env = make_env(options)
    log_file = env['POMAGMA_LOG_FILE']
    sys.stderr.write('{}\n'.format(' \\\n'.join(args)))
    if os.path.exists('core'):
        os.remove('core')
    if subprocess.check_output('ulimit -c', shell=True).strip() == '0':
        print 'WARNING cannot write core file; try `ulimit -c unlimited`'
    with log_duration():
        info = subprocess.call(args, env=env)
    if info:
        print_logged_error(log_file)
        trace = get_stack_trace(args[0])
        log_print(trace, log_file)
        sys.exit(info)


def build():
    buildtype = 'Debug' if debug else 'RelWithDebInfo'
    buildflag = '-DCMAKE_BUILD_TYPE={}'.format(buildtype)
    if not os.path.exists(BUILD):
        os.makedirs(BUILD)
    with chdir(BUILD):
        check_call('cmake', buildflag, ROOT)
        check_call('make', '--quiet', '--jobs=%d' % (1 + CPU_COUNT))


def test():
    build()
    buildtype = 'debug' if debug else 'release'
    opts = {
        'log_file': os.path.join(DATA, 'test', '{}.log'.format(buildtype)),
        'log_level': 3,
        }
    log_call('make', '-C', BUILD, 'test', **opts)


def count_obs(structure):
    points = structure.getNode('/carrier/points')
    item_dim = max(points)
    item_count, = points.shape
    return item_dim, item_count


def get_hash(infile):
    with load(infile) as structure:
        digest = structure.getNodeAttr('/', 'hash').tolist()
        return digest


def get_info(infile):
    with load(infile) as structure:
        item_dim, item_count = count_obs(structure)
        info = dict(item_dim=item_dim, item_count=item_count)
        return info

def print_info(infile):
    with load(infile) as structure:
        item_dim, item_count = count_obs(structure)
        print 'item_dim =', item_dim
        print 'item_count =', item_count
        for o in structure:
            print o
