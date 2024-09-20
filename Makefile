TEST_DIR = test
SOURCE_DIR = src
CFLAGS = -g

test/%.test: $(SOURCE_DIR)/sand.c $(SOURCE_DIR)/%.c $(SOURCE_DIR)/%.test.c
	cc $(CFLAGS) -o $@ $^


	
	

	