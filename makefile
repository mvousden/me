UNITY_DIR ?= /home/mark/repos/unity/
UNITY_OBJ ?= $(UNITY_DIR)src/unity.o
UNITY_OUTPUT_PARSER ?= $(UNITY_DIR)auto/parse_output.rb
CC ?= ccache clang
CFLAGS = -std=c99 -Wall -Wextra -pedantic -O3
SANITIZEFLAGS = #-fsanitize=address -fsanitize=undefined
DEBUGFLAGS = -g3 -O0 $(SANITIZEFLAGS)
LDLIBS =
OBJ = buffer.o cmds.o conf.o error.o helpers.o keyb.o line.o state.o term.o
EXEC_DIR = exec
TESTS = $(patsubst test_%.c, test_%, $(filter test_%.c, $(shell ls)))
TEST_PATHS = $(patsubst test_%, $(EXEC_DIR)/test_%, $(TESTS))
TEST_OUTPUTS_DIR = test_outputs
EXEC = me

$(EXEC_DIR)/$(EXEC): $(EXEC_DIR) $(EXEC).o $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(filter %.o, $^) $(LDLIBS)
$(EXEC_DIR) $(TEST_OUTPUTS_DIR):
	mkdir --parents "$@"
debug: CFLAGS += $(DEBUGFLAGS)
debug: LDLIBS += $(SANITIZEFLAGS)
debug: $(EXEC_DIR)/$(EXEC)
test: CFLAGS += -I"$(UNITY_DIR)src/" $(DEBUGFLAGS)
test: LDLIBS += $(SANITIZEFLAGS)
test: $(EXEC_DIR) $(TEST_PATHS)
# Recipe needed explicitly here because the target and prerequisites use a
# wildcard, making this an implicit rule - make can't connect implicit rules
# together.
$(EXEC_DIR)/test_%: test_%.o $(UNITY_OBJ) $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)
exec_tests: test $(TEST_OUTPUTS_DIR)
	RC=0; \
	for TEST in $(TESTS); do \
		RESULT_TXT_PATH="$(TEST_OUTPUTS_DIR)/$${TEST}_result.txt"; \
		RESULT_VG_LOG_PATH="$(TEST_OUTPUTS_DIR)/$${TEST}_vg_out.txt"; \
		RESULT_JUNIT_PATH="$(TEST_OUTPUTS_DIR)/$${TEST}_junit.xml"; \
		printf "\nRunning $${TEST}..."; \
		valgrind --log-file="$$RESULT_VG_LOG_PATH" "$(EXEC_DIR)/$$TEST" \
			> "$$RESULT_TXT_PATH"; \
		ERR=$$?; \
		ruby "$(UNITY_OUTPUT_PARSER)" -xml "$$RESULT_TXT_PATH" > /dev/null; \
		mv report.xml "$$RESULT_JUNIT_PATH"; \
		if [ $$ERR -ne 0 ]; then \
			RC=1; \
			printf " FAIL: check output logs."; \
		fi; \
	done; \
	printf "\nTesting complete.\n"; \
	exit $$RC;
clean:
	rm --force --recursive *.o $(EXEC_DIR) $(TEST_OUTPUTS_DIR)
.PHONY: clean debug exec_tests test
.PRECIOUS: %.o
