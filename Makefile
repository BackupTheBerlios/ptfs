all:
	make -C aurochs/
	make -C src/

clean:
	make -C src/ clean