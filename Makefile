build: test_simple test_simple_opt test_ubsan

test_simple: tests/deque_test.cpp src/deque.h
	clang++ -std=c++20 -gdwarf-4 -O0 -Wall -Wextra -Werror -o ./test_simple tests/deque_test.cpp

test_simple_opt: tests/deque_test.cpp src/deque.h
	clang++ -std=c++20 -O2 -Wall -Wextra -Werror -o ./test_simple_opt tests/deque_test.cpp

test_ubsan: tests/deque_test.cpp src/deque.h
	clang++ -std=c++20 -g -O0 -Wall -Wextra -Werror -fsanitize=undefined -o ./test_ubsan tests/deque_test.cpp

info:
	clang++ --version
	clang-tidy --version
	clang-format --version
	valgrind --version

run: build
	@echo 'Run tests (simple)'
	time ./test_simple
	@echo 'Run tests (simple_opt)'
	time ./test_simple_opt
	@echo 'Run tests (ubsan)'
	time ./test_ubsan
	@echo 'Run tests (valgrind)'
	time valgrind --leak-check=yes --error-exitcode=1 ./test_simple 

lint:
	@echo 'Check code is formatted'
	bash -c "diff -u <(cat src/*.h src/*.cpp) <(clang-format --style=file --Werror src/*.h src/*.cpp)"
	@echo 'Run linter'
	clang-tidy --config "$(shell cat .clang-tidy)" --warnings-as-errors="*" tests/deque_test.cpp '-header-filter=.*' -- -std=c++20 -g -O0 -Wall -Wextra -Werror
	@echo 'Check NOLINT is not used'
	! grep NOLINT src/deque.h
	@echo 'Check std::deque is not used'
	! grep std::deque src/deque.h
	@echo 'Check all TODOs are removed'
	! grep TODO src/deque.h

test: info run lint
	@echo 'Great job!'

format:
	@echo 'Apply linter fixes'
	clang-tidy --config "$(shell cat .clang-tidy)" --fix tests/deque_test.cpp '-header-filter=.*' -- -std=c++20 -g -O0 -Wall -Wextra -Werror
	@echo 'Apply formatter'
	clang-format --style=file -i src/*.h src/*.cpp

clean:
	rm test_simple test_simple_opt test_ubsan
