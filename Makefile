TEST_DIR = test
SOURCE_DIR = src
CFLAGS = $(shell cat compile_flags.txt | tr '\n' ' ')

CWPACK_DIR = ext/CWPack
VECTOR_SRC = $(SOURCE_DIR)/vector.c $(CWPACK_DIR)/src/cwpack.c $(CWPACK_DIR)/goodies/utils/cwpack_utils.c

test/%.test: $(SOURCE_DIR)/sand.c $(SOURCE_DIR)/%.c $(SOURCE_DIR)/%.test.c
	cc $(CFLAGS) -o $@ $^

test/vector.test: $(VECTOR_SRC) $(SOURCE_DIR)/sand.c $(SOURCE_DIR)/vector.test.c
	cc $(CFLAGS) -o $@ $^ -lm

test/spice.test: $(SOURCE_DIR)/spice.c $(SOURCE_DIR)/sand.c $(VECTOR_SRC) $(SOURCE_DIR)/spice.test.c
	cc $(CFLAGS) -o $@ $^ -lngspice -lm
	
test/pso.test: ${SOURCE_DIR}/pso.test.c $(SOURCE_DIR)/pso.c $(SOURCE_DIR)/random.c $(SOURCE_DIR)/sand.c
	cc $(CFLAGS) -o $@ $^ -lgsl -lm



	
	

	
