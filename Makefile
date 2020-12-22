all:
	g++ -Wall -c common.cpp
	g++ -Wall client.cpp common.o -o cliente
	g++ -Wall server-mt.cpp common.o -lpthread -o servidor

tester: $(TESTS)
	g++ -Wall -I third-party test_common.cpp common.o -o test_common.out
	@./test_common.out
	$(RM) test_*.out

clean:
	rm common.o cliente servidor
