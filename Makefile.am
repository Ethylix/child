SUBDIRS=src src/modules tests
bin_SCRIPTS=childsymlink

module:
	cd src; ${MAKE} ${MAKEARGS} MODFILE=${MODFILE} module

childsymlink:
	@ln -sf src/child child
