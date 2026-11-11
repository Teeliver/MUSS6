#include "mu/book_requirements.hpp"
#include <cassert>
int main(){std::array<std::int64_t,6> required={100,80,60,120,40,20};assert(mu::meets_book_requirements(required,required));for(unsigned i=0;i<6;i++){auto actual=required;actual[i]--;assert(!mu::meets_book_requirements(actual,required));actual[i]+=2;assert(mu::meets_book_requirements(actual,required));}assert(mu::meets_book_requirements({70000,80000,90000,100000,110000,400},required));}
