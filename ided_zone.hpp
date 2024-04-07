#ifndef IDED_ZONE_HPP
#define IDED_ZONE_HPP 1

#define NUM_TRIES (1000000)

#include <cassert>
#include <bits/stdc++.h>
#include <utility>
#include "gen_zone.hpp"
#include "utils.hpp"
#include "zone.hpp"

// Use (void) to silence unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))

template <class T>
class ided_zone: public gen_zone{
    int myid;
    std::vector<int> chids;
    timedrel::zone<T> myzone = timedrel::zone<T>::make({0,0,0,0,0,0});
  public:
  	ided_zone(timedrel::zone<T> &mz, int inmyid, int inchid1, int inchid2){
  	    this->myzone = mz;
  		this->myid   = inmyid;
        (this->chids).push_back(inchid1);
        (this->chids).push_back(inchid2);
  	}
    ided_zone(const timedrel::zone<T> &mz, int inmyid, std::vector<int> chids){
        this->myzone = mz;
        this->myid = inmyid;
        this->set_chids(chids);
    }
    const std::vector<int> get_chids() const{
        return this->chids;
    }
    void set_chids(std::vector<int> inchids){
        this->chids = inchids;
    }
  	void set_myid(int inid){
  		myid = inid;
  	}
  	timedrel::zone<T> get_myzone() const {
  	    return myzone;
  	}
  	int get_myid() const {
  		return myid;
  	}
  	virtual bool compare_less_bmin(const std::shared_ptr<gen_zone> &c){
  	    std::shared_ptr<ided_zone<T>> iz = std::dynamic_pointer_cast<ided_zone<T>>(c);
        return myzone.get_bmin() < (iz->get_myzone()).get_bmin();
    }
    virtual bool compare_less_emin(const std::shared_ptr<gen_zone> &c){
    	std::shared_ptr<ided_zone<T>> iz = std::dynamic_pointer_cast<ided_zone<T>>(c);
        return myzone.get_emin() < (iz->get_myzone()).get_emin();
    }
    virtual bool compare_less_bmax_bmin(const std::shared_ptr<gen_zone> &c){
    	std::shared_ptr<ided_zone<T>> iz = std::dynamic_pointer_cast<ided_zone<T>>(c);
        return myzone.get_bmax() < (iz->get_myzone()).get_bmin();
    }
    virtual bool compare_less_emin_bmin(const std::shared_ptr<gen_zone> &c){
    	std::shared_ptr<ided_zone<T>> iz = std::dynamic_pointer_cast<ided_zone<T>>(c);
        return myzone.get_emin() < (iz->get_myzone()).get_bmin();
    }
    virtual bool compare_less_bmax_emin(const std::shared_ptr<gen_zone> &c){
        std::shared_ptr<ided_zone<T>> iz = std::dynamic_pointer_cast<ided_zone<T>>(c);
        return myzone.get_bmax() < (iz->get_myzone()).get_emin();   
    }
    virtual bool compare_less_emax_bmin(const std::shared_ptr<gen_zone> &c){
        std::shared_ptr<ided_zone<T>> iz = std::dynamic_pointer_cast<ided_zone<T>>(c);
        return myzone.get_emax() < (iz->get_myzone()).get_bmin();
    }
    virtual bool compare_less_bmin_bmax(const std::shared_ptr<gen_zone> &c){
        std::shared_ptr<ided_zone<T>> iz = std::dynamic_pointer_cast<ided_zone<T>>(c);
        return myzone.get_bmin() < (iz->get_myzone()).get_bmax();   
    }
    virtual bool includes(const std::shared_ptr<gen_zone> &c){
    	std::shared_ptr<ided_zone<T>> iz = std::dynamic_pointer_cast<ided_zone<T>>(c);
        return timedrel::zone<T>::includes(myzone, iz->get_myzone());
    }
    virtual std::shared_ptr<gen_zone> clone(){
        return std::make_shared<ided_zone<T>>(myzone, myid, chids);
    }
    virtual std::shared_ptr<gen_zone> intersection(const std::shared_ptr<gen_zone> &gz){
    	std::shared_ptr<ided_zone<T>> iz = std::dynamic_pointer_cast<ided_zone<T>>(gz);
    	timedrel::zone<T> izres = timedrel::zone<T>::intersection(this->get_myzone(), iz->get_myzone());
    	// Put the invalid -1 for myid
    	return std::make_shared<ided_zone<T>>(izres, -1, this->get_myid(), iz->get_myid());
    }
    virtual std::shared_ptr<gen_zone> concatenation(const std::shared_ptr<gen_zone> &gz){
        std::shared_ptr<ided_zone<T>> iz = std::dynamic_pointer_cast<ided_zone<T>>(gz);
        timedrel::zone<T> izres = timedrel::zone<T>::concatenation(this->get_myzone(), iz->get_myzone());
        // Put the invalid -1 for myid
        std::vector<int> reschids1 = this->get_chids();
        std::vector<int> reschids2 = iz->get_chids();
        reschids1.insert(reschids1.end(), reschids2.begin(), reschids2.end() );
        return std::make_shared<ided_zone<T>>(izres, -1, reschids1);
    }
    virtual bool is_nonempty(){
    	return (this->get_myzone()).is_nonempty();
    }
};

template <class T>
inline std::ostream& operator<<(
    std::ostream &os, const ided_zone<T> &z){
	std::cout<<"("<<z.get_myzone()<<")-->";
	std::cout<<"("<<z.get_myid()<<")-->(";
    std::vector<int> chvec = z.get_chids();
    for(int i=0; i < chvec.size(); i++){
        if(not (i == (chvec.size()-1) ) ){
            std::cout<<chvec[i]<<",";    
        }else{
            std::cout<<chvec[i];
        }
        
    }
    std::cout<<")";
    return os;
}

// Check if interval is in zone.
template <class T>
bool zone_contains_interval(const timedrel::zone<T> &z, T t1, T t2){
    auto t1_t2 = timedrel::zone<T>::make({t1,t1,t2,t2,t2-t1,t2-t1},{1,1,1,1,1,1});
    return timedrel::zone<T>::includes(z, t1_t2);
}

// Intended to work for only float, double and mpq_class (rationals).
// Get interior point of zone.
template <class T>
std::pair<T,T> zone_interior_point(timedrel::zone<T> &z){
    assertm((std::is_same<float, T>::value or
            std::is_same<double, T>::value or 
            std::is_same<mpq_class, T>::value)
            , "Type not supported (only float, double and rationals supported).");
    srand(time(0));
    for(int i=0;i < NUM_TRIES; i++){
        // Code repetition avoidable?
        if(std::is_same<mpq_class,T>::value){
            double dr1 = (double)(rand()) / (double)(RAND_MAX);
            double dr2 = (double)(rand()) / (double)(RAND_MAX);
            auto r1 = mpq_class(dr1);
            auto r2 = mpq_class(dr2);
            auto lb = z.get_bmin().value + r1 * (z.get_bmax().value-z.get_bmin().value);
            auto ub = z.get_emin().value + r2 * (z.get_emax().value-z.get_emin().value);
            if(zone_contains_interval(z, lb, ub)){
                return std::pair<T,T>(lb, ub);
            }
        }else if(std::is_same<float,T>::value or std::is_same<double,T>::value){
            auto r1 = (T)(rand()) / (T)(RAND_MAX);
            auto r2 = (T)(rand()) / (T)(RAND_MAX);
            auto lb = z.get_bmin().value + r1 * (z.get_bmax().value-z.get_bmin().value);
            auto ub = z.get_emin().value + r2 * (z.get_emax().value-z.get_emin().value);
            if(zone_contains_interval(z, lb, ub)){
                return std::pair<T,T>(lb, ub);
            }
        }
    }
    // If we can't find an interior point return an invalid point.
    return std::pair<T,T>(-1,-1);
}

// Intended to work for only float, double and mpq_class (rationals).
// Find t2 such that (t1,t2) in z1 and (t2,t3) in z2 and (t1, t3) in zres.
template <class T>
T infer_seq_comp(const timedrel::zone<T> &zres, const timedrel::zone<T> &z1, const timedrel::zone<T> &z2, 
    std::pair<T,T> &inival){
    assertm((std::is_same<float, T>::value or
            std::is_same<double, T>::value or 
            std::is_same<mpq_class, T>::value)
            , "Type not supported (only float, double and rationals supported).");
    T t1 = inival.first;
    T t3 = inival.second;
    // TODO: Check if (t1,t3) in zres
    auto t1_t3 = timedrel::zone<T>::make({t1,t1,t3,t3,t3-t1,t3-t1},{1,1,1,1,1,1});
    assertm(timedrel::zone<T>::includes(zres, t1_t3), "Point not in the zone!");

    // TODO: This might be violated due to floating point imprecision
    if(std::is_same<mpq_class, T>::value){
        assertm(zres==timedrel::zone<T>::concatenation(z1, z2), "Inconsistency infer seq comp!");    
    }

    // Find upper bound on t2
    auto c2 =    z1.get_emax();
    auto cp1 =   z2.get_bmax();

    auto ubc3 =  z1.get_dmax();
    auto ubt1 =  timedrel::upper_bound<T>(t1, 1);
    auto ubr3 =  timedrel::upper_bound<T>::add(ubc3, ubt1);

    auto ubcp4 = timedrel::upper_bound<T>(-z2.get_dmin().value, 
                 z2.get_dmin().sign);
    auto ubt3 =  timedrel::upper_bound<T>(t3, 1);
    auto ubr4 =  timedrel::upper_bound<T>::add(ubcp4, ubt3);
    
    auto uba =   timedrel::upper_bound<T>::intersection(c2, cp1);
    auto ubb =   timedrel::upper_bound<T>::intersection(ubr3, ubr4);

    auto ubt2 =  timedrel::upper_bound<T>::intersection(uba, ubb);

    // Find lower bound on t2
    auto c5 =    z1.get_emin();
    auto cp6 =   z2.get_bmin();

    auto lbc4 =  z1.get_dmin();
    auto lbt1 =  timedrel::lower_bound<T>(t1, 1);
    auto lbr3 =  timedrel::lower_bound<T>::add(lbc4, lbt1);

    auto lbcp3 = timedrel::lower_bound<T>(-z2.get_dmax().value, 
                 z2.get_dmax().sign);
    auto lbt3 =  timedrel::lower_bound<T>(t3, 1);
    auto lbr4 =  timedrel::lower_bound<T>::add(lbcp3, lbt3);
    
    auto lba =   timedrel::lower_bound<T>::intersection(c5, cp6);
    auto lbb =   timedrel::lower_bound<T>::intersection(lbr3, lbr4);

    auto lbt2  = timedrel::lower_bound<T>::intersection(lba, lbb);

    // Return the mean of lower and upper bounds
    return (lbt2.value+ubt2.value)/2;
}


// Intended to work for only float, double and mpq_class (rationals).
// Find t2 such that (t1,t2) in z1 and (t2,t3) in z2 and (t1, t3) in zres.
template <class T>
T infer_seq_comp_debug(const timedrel::zone<T> &zres, const timedrel::zone<T> &z1, const timedrel::zone<T> &z2, 
    std::pair<T,T> &inival){
    assertm((std::is_same<float, T>::value or
            std::is_same<double, T>::value or 
            std::is_same<mpq_class, T>::value)
            , "Type not supported (only float, double and rationals supported).");
    std::cout<<">>>>>>"<<std::endl;
    T t1 = inival.first;
    std::cout<<t1<<std::endl;
    T t3 = inival.second;
    std::cout<<t3<<std::endl;
    // TODO: Check if (t1,t3) in zres
    auto t1_t3 = timedrel::zone<T>::make({t1,t1,t3,t3,t3-t1,t3-t1},{1,1,1,1,1,1});
    assertm(timedrel::zone<T>::includes(zres, t1_t3), "Point not in the zone!");

    // TODO: This might be violated due to floating point imprecision
    if(std::is_same<mpq_class, T>::value){
        assertm(zres==timedrel::zone<T>::concatenation(z1, z2), "Inconsistency infer seq comp!");    
    }

    // Find upper bound on t2
    auto c2 =    z1.get_emax();
    std::cout<<c2<<std::endl;
    auto cp1 =   z2.get_bmax();
    std::cout<<cp1<<std::endl;

    auto ubc3 =  z1.get_dmax();
    std::cout<<ubc3<<std::endl;
    auto ubt1 =  timedrel::upper_bound<T>(t1, 1);
    std::cout<<ubt1<<std::endl;
    auto ubr3 =  timedrel::upper_bound<T>::add(ubc3, ubt1);
    std::cout<<ubr3<<std::endl;

    auto ubcp4 = timedrel::upper_bound<T>(-z2.get_dmin().value, 
                 z2.get_dmin().sign);
    std::cout<<ubcp4<<std::endl;
    auto ubt3 =  timedrel::upper_bound<T>(t3, 1);
    std::cout<<ubt3<<std::endl;
    auto ubr4 =  timedrel::upper_bound<T>::add(ubcp4, ubt3);
    std::cout<<ubr4<<std::endl;

    auto uba =   timedrel::upper_bound<T>::intersection(c2, cp1);
    std::cout<<uba<<std::endl;
    auto ubb =   timedrel::upper_bound<T>::intersection(ubr3, ubr4);
    std::cout<<ubb<<std::endl;

    auto ubt2 =  timedrel::upper_bound<T>::intersection(uba, ubb);
    std::cout<<ubt2<<std::endl;

    // Find lower bound on t2
    auto c5 =    z1.get_emin();
    std::cout<<c5<<std::endl;
    auto cp6 =   z2.get_bmin();
    std::cout<<cp6<<std::endl;

    auto lbc4 =  z1.get_dmin();
    std::cout<<lbc4<<std::endl;
    auto lbt1 =  timedrel::lower_bound<T>(t1, 1);
    std::cout<<lbt1<<std::endl;
    auto lbr3 =  timedrel::lower_bound<T>::add(lbc4, lbt1);
    std::cout<<lbr3<<std::endl;

    auto lbcp3 = timedrel::lower_bound<T>(-z2.get_dmax().value, 
                 z2.get_dmax().sign);
    std::cout<<lbcp3<<std::endl;
    auto lbt3 =  timedrel::lower_bound<T>(t3, 1);
    std::cout<<lbt3<<std::endl;
    auto lbr4 =  timedrel::lower_bound<T>::add(lbcp3, lbt3);
    std::cout<<lbr4<<std::endl;
    
    auto lba =   timedrel::lower_bound<T>::intersection(c5, cp6);
    std::cout<<lba<<std::endl;
    auto lbb =   timedrel::lower_bound<T>::intersection(lbr3, lbr4);
    std::cout<<lbb<<std::endl;
    auto lbt2  = timedrel::lower_bound<T>::intersection(lba, lbb);
    std::cout<<lbt2<<std::endl;

    // Return the mean of lower and upper bounds
    std::cout<<"========================="<<std::endl;
    std::cout<<"{"<<inival.first<<","<<inival.second<<"}"<<std::endl;
    std::cout<<"zres:"<<zres<<std::endl;
    std::cout<<"z1:"<<z1<<std::endl;
    std::cout<<"z2:"<<z2<<std::endl;
    std::cout<<"["<<lbt2.value<<","<<ubt2.value<<"]"<<std::endl;
    std::cout<<"t2:"<<(lbt2.value+ubt2.value)/2<<std::endl;
    std::cout<<"========================="<<std::endl;
    // exit(0);

    return (lbt2.value+ubt2.value)/2;
}

template <class T>
std::vector<std::pair<T,T>> infer_mult_seq_comp(const timedrel::zone<T> &res, 
    std::vector<timedrel::zone<T>> &zlist, std::pair<T,T> &inival){
    assertm((std::is_same<float, T>::value or
            std::is_same<double, T>::value or 
            std::is_same<mpq_class, T>::value)
            , "Type not supported (only float, double and rationals supported).");
    T lb = inival.first;
    T ub = inival.second;

    // Check inclusion of (lb,ub) in res
    auto lb_ub = timedrel::zone<T>::make({lb,lb,ub,ub,ub-lb,ub-lb},{1,1,1,1,1,1});
    assertm(timedrel::zone<T>::includes(res, lb_ub), "Point not in the zone!");

    std::vector<std::pair<T,T>> ival_res(zlist.size(),{-1,-1});

    // Initialize accumulation vector
    auto tz = timedrel::zone<T>::make({0,0,0,0,0});
    std::vector<timedrel::zone<T>> seq_comp_acc(zlist.size(),tz);

    if(zlist.size()==0){
        return ival_res;
    }

    // Populate accumulation vector
    auto temp_acc = zlist[1];
    for(int i = 0; i < zlist.size(); i++){
        if(i == 0){
            seq_comp_acc[i] = temp_acc;
        }else{
            temp_acc = timedrel::zone<T>::concatenation(temp_acc, zlist[i]);
            seq_comp_acc[i] = temp_acc;
        }
    }

    // Check if the zones indeed lead to the result.
    // TODO: This might be violated due to floating point imprecision.
    if(std::is_same<mpq_class, T>::value){
        assertm(res == seq_comp_acc.back(),
            "Sequential composition of zones doesn't produce the result!");    
    }
    

    int j = zlist.size()-1;
    std::pair<T,T> temp_ival = inival;
    while(j >= 1){
        timedrel::zone<T> tzres = seq_comp_acc[j]; 
        timedrel::zone<T> tz1 = seq_comp_acc[j-1];
        timedrel::zone<T> tz2  = zlist[j];
        T tmid = infer_seq_comp(tzres, tz1, tz2, temp_ival);
        ival_res[j] = {tmid, temp_ival.second};
        temp_ival.second = tmid;

        j--;
    }
    ival_res[0] = temp_ival;
    return ival_res;
}

#endif