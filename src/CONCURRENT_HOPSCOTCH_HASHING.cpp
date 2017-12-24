//============================================================================
// Name        : CONCURRENT_HOPSCOTCH_HASHING.cpp
// Author      : Stetskevich
// Version     :
// Copyright   : Stetskevich
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <pthread.h>
#include <string>
#include <stdio.h>
#include "ConcurrentHopscotchHashSet.h"

int main() {
	ConcurrentHopscotchHashSet<std::string,std::string> a;
	std::string *s = new std::string("123");
	std::string *d = new std::string("1545");
	a.add(s, d);
	std::cout<<a.get(s);
	return 0;
}
