TEST_DIR = test
SOURCE_DIR = src
CFLAGS = -g

test/%.test: $(SOURCE_DIR)/sand.c $(SOURCE_DIR)/%.c $(SOURCE_DIR)/%.test.c
	cc $(CFLAGS) -o $@ $^

test/spice.test: $(SOURCE_DIR)/spice.c $(SOURCE_DIR)/sand.c $(SOURCE_DIR)/vector.c $(SOURCE_DIR)/spice.test.c
	cc $(CFLAGS) -o $@ $^ -lngspice


	
	

	