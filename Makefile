.PHONY build:
build:
	docker build -t compilerbook .

.PHONY test:
test:
	docker run --rm -v ${SOURCE_DIR}:${DEST_DIR} -w ${DEST_DIR} compilerbook cat test.txt

.PHONY shell:
shell:
	docker run -it -v ${SOURCE_DIR}:${DEST_DIR} -w ${DEST_DIR}  --platform=linux/amd64 compilerbook

.PHONY dass-do:
dass-do: # deassembly binary file from docker
	docker run --rm -v ${SOURCE_DIR}:${DEST_DIR} -w ${DEST_DIR} compilerbook make dass

.PHONY dass:
dass: # deassembly a.out file
	objdump -d -M intel a.out