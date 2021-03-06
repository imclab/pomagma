all:
	$(MAKE) static-check
	pip install -e .
	$(MAKE) -C src/language
	$(MAKE) -C src/cartographer
	$(MAKE) -C src/analyst
	POMAGMA_DEBUG= python -m pomagma build
	python -m pomagma build

set-ulimit: FORCE
	$(call ulimit -c unlimited)

static-check: FORCE
	find src | grep '.py$$' | grep -v '_pb2.py' | xargs pyflakes
	find src | grep '.py$$' | grep -v '_pb2.py' | xargs pep8

unit-test: all set-ulimit FORCE
	POMAGMA_DEBUG= python -m pomagma unit-test
batch-test: all set-ulimit FORCE
	POMAGMA_DEBUG= python -m pomagma batch-test
h4-test: all set-ulimit FORCE
	POMAGMA_DEBUG= python -m pomagma batch-test h4
sk-test: all set-ulimit FORCE
	POMAGMA_DEBUG= python -m pomagma batch-test sk
skj-test: all set-ulimit FORCE
	POMAGMA_DEBUG= python -m pomagma batch-test skj
skrj-test: all set-ulimit FORCE
	POMAGMA_DEBUG= python -m pomagma batch-test skrj
test: all set-ulimit FORCE
	POMAGMA_DEBUG= python -m pomagma unit-test
	POMAGMA_DEBUG= python -m pomagma batch-test

h4: all
	python -m pomagma.batch make h4
sk: all
	python -m pomagma.batch make sk
skj: all
	python -m pomagma.batch make skj
skrj: all
	python -m pomagma.batch make skrj

python-libs:
	@$(MAKE) -C src/language all

profile:
	python -m pomagma unit_profile
	# TODO add profile for sequential & concurrent dense_set

clean: FORCE
	rm -rf build lib include
	git clean -fdx -e pomagma.egg-info -e data

FORCE:
