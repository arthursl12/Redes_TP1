all:
	g++ -Wall -c common.c
	g++ -Wall client.c common.o -o cliente
	g++ -Wall server-mt.c common.o -lpthread -o servidor

tester: $(TESTS)
	g++ -Wall -I third-party test_common.cpp -o test_common.out
	@./test_common.out
	$(RM) test_*.out

clean:
	rm common.o cliente servidor
