#include "ided_zone.hpp"
#include "algos_timed_relations.hpp"

using namespace std;
using namespace timedrel;
using namespace paramtimedrel;

int main(){
	mpq_class rat1(2,6);
	mpq_class rat2(0,1);
	mpq_class rat = rat1+rat2;
	cout<<rat.get_d()<<endl;
	zone<mpq_class> zt = zone<mpq_class>::make({rat2,rat1,rat2,rat1,0,rat1});
	cout<<zt<<endl;
	return 0;
}