all:
	pip install -e .
	$(MAKE) -C src/vehicles
	POMAGMA_DEBUG= python -m pomagma build
	python -m pomagma build

set-ulimit: FORCE
	$(call ulimit -c unlimited)

PYFLAKES = find src | grep '.py$$' | grep -v '_pb2.py' | xargs pyflakes

unit-test: set-ulimit
	@$(PYFLAKES)
	POMAGMA_DEBUG= python -m pomagma unit-test
batch-test: all set-ulimit
	POMAGMA_DEBUG= python -m pomagma batch-test
h4-test: all set-ulimit
	POMAGMA_DEBUG= python -m pomagma batch-test h4
sk-test: all set-ulimit
	POMAGMA_DEBUG= python -m pomagma batch-test sk
skj-test: all set-ulimit
	POMAGMA_DEBUG= python -m pomagma batch-test skj
test: all set-ulimit
	@$(PYFLAKES)
	POMAGMA_DEBUG= python -m pomagma unit-test
	POMAGMA_DEBUG= python -m pomagma batch-test

h4: all
	python -m pomagma.batch survey h4
sk: all
	python -m pomagma.batch survey sk
skj: all
	python -m pomagma.batch survey skj

python-libs:
	@$(MAKE) -C src/vehicles all

profile:
	python -m pomagma profile

clean: FORCE
	rm -rf build lib include
	git clean -fdx -e pomagma.egg-info

FORCE:
