# ---- Part 1 ----
build-part1: part1.out

part1.out: part1_searcher.cpp
	g++ -g part1_searcher.cpp -o part1.out

run-part1: part1.out
	./part1.out $(ARGS)

clean-part1:
	rm -f part1.out

# ---- Part 2 ----
build-part2: part2_partitioner.out

part2_searcher.out: part2_searcher.cpp
	g++ -g part2_searcher.cpp -o part2_searcher.out

part2_partitioner.out: part2_partitioner.cpp part2_searcher.out
	g++ -g part2_partitioner.cpp -o part2_partitioner.out

run-part2: part2_partitioner.out
	./part2_partitioner.out $(ARGS)

clean-part2:
	rm -f part2_partitioner.out

# ---- Part 3 ----
build-part3: part3_partitioner.out part3_searcher.out

part3_searcher.out: part3_searcher.cpp
	g++ -g part3_searcher.cpp -o part3_searcher.out

part3_partitioner.out: part3_partitioner.cpp part3_searcher.out
	g++ -g part3_partitioner.cpp -o part3_partitioner.out

run-part3: part3_partitioner.out
	./part3_partitioner.out $(ARGS)

clean-part3:
	rm -f part3_partitioner.out part3_searcher.out

# ---- Clean all ----
clean:
	rm -f *.out
