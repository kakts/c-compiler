.PHONY build:
build:
	docker build -t compilerbook .

.PHONY test:
test:
	docker run --rm -v ${SOURCE_DIR}:${DEST_DIR} -w ${DEST_DIR} compilerbook cat test.txt

.PHONY shell:
shell:
	docker run -it -v ${SOURCE_DIR}:${DEST_DIR} -w ${DEST_DIR} compilerbook