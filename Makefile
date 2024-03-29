all:
	g++ -Wall -g -c common.cpp
	g++ -Wall client.cpp common.o -lpthread -o cliente
	g++ -Wall server-mt.cpp common.o -lpthread -o servidor

tester: $(TESTS)
	g++ -Wall -I third-party -g test_common.cpp common.o -o test_common.out
	@./test_common.out
	$(RM) test_*.out

clean:
	rm common.o cliente servidor
