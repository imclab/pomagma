import contextlib
import pomagma.util
from pomagma.analyst import client
from pomagma.analyst import server


connect = client.Client
serve = server.Server


@contextlib.contextmanager
def load(theory, world, address=None, **opts):
    with pomagma.util.log_duration():
        server = serve(theory, world, address, **opts)
        yield server.connect()
        server.stop()
