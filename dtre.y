%{
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <chrono>
#include <map>
#include <gmpxx.h>

#include "zone_set.hpp"
#include "gen_zone.hpp"
#include "algos_timed_relations.hpp"
#include "ided_zone.hpp"
#include "utils.hpp"
#include "helper_dtre.hpp"
#include "diag_tree.hpp"

using namespace std::chrono;

using namespace std;

using namespace timedrel;

using namespace paramtimedrel;

extern int yylex();
extern int yyparse();
extern FILE* yyin;

vector<string> time_vec;
vector<vector<string>> value_vec;

vector<vector<shared_ptr<gen_zone>>> dtre_stack;
vector<string> sexp_stack;
vector<shared_ptr<diag_tree>> dt_stack;

void yyerror(const char* s);
%}

%union {
	int ival;
	double fval;
    char* fstr;
}

%token<ival> X_STRING
%token<fstr> T_FLOAT
%token<ival> GEQ GE LEQ LE
%token K_PLUS T_MINUS T_NOT T_LEFT T_RIGHT
%token T_SLEFT T_SRIGHT
%token T_EPS
%token<ival> T_RISE T_FALL
%token T_AND T_OR T_SEQCOMP
%left T_PLUS T_MINUS

%type<ival> compop edgeop
%type<fstr> magconst

%start dtre;

%%

magconst: T_FLOAT {string tfstr($1); free($1); $$ = strdup(tfstr.c_str());}
      | T_MINUS T_FLOAT {
        string ntfstr($2);
        free($2);
        ntfstr = "-"+ntfstr;
        $$ = strdup(ntfstr.c_str());
      }
      ;

dtre: T_EPS {
            vector<shared_ptr<gen_zone>> zp_res;
            timedrel::zone<mpq_class> ztemp = 
                    timedrel::zone<mpq_class>::make_from({0, timedrel::zone<mpq_class>::infinity(), 
                    0, timedrel::zone<mpq_class>::infinity(),
                    0, 0},
                    {1,1,1,1,1,1});
            zp_res.push_back(make_shared<ided_zone<mpq_class>>(ztemp, 0, vector<int>()));
            dtre_stack.push_back(zp_res);

            sexp_stack.push_back("EPS");

            shared_ptr<diag_tree> dtnode = 
                make_shared<diag_tree>(op_type::atomic, sexp_stack.back(), dtre_stack.back(),
                    nullptr, nullptr);
            dt_stack.push_back(dtnode);
      }
      | edgeop X_STRING{
            vector<shared_ptr<gen_zone>> zp_res;
            vector<timedrel::zone<mpq_class>> leaf_zones = bool_to_edge_zone(time_vec, value_vec[$2], $1);
            for(int i=0; i<leaf_zones.size(); i++){
                zp_res.push_back(make_shared<ided_zone<mpq_class>>(leaf_zones[i], i, vector<int>()));
            }
            dtre_stack.push_back(zp_res);

            string edge_str = ($1==1)?string("+@"):string("-@");
            sexp_stack.push_back(edge_str+"x"+to_string($2));

            shared_ptr<diag_tree> dtnode = 
                make_shared<diag_tree>(op_type::atomic, sexp_stack.back(), dtre_stack.back(),
                    nullptr, nullptr);
            dt_stack.push_back(dtnode);
      }
      | X_STRING{
            vector<shared_ptr<gen_zone>> zp_res;
            vector<timedrel::zone<mpq_class>> leaf_zones = bool_to_zone(time_vec, value_vec[$1], 1);
            for(int i=0; i<leaf_zones.size(); i++){
                zp_res.push_back(make_shared<ided_zone<mpq_class>>(leaf_zones[i], i, vector<int>()));
            }
            dtre_stack.push_back(zp_res);

            sexp_stack.push_back("x"+to_string($1));

            shared_ptr<diag_tree> dtnode = 
                make_shared<diag_tree>(op_type::atomic, sexp_stack.back(), dtre_stack.back(),
                    nullptr, nullptr);
            dt_stack.push_back(dtnode);
      }
      | T_NOT X_STRING{
            vector<shared_ptr<gen_zone>> zp_res;
            vector<timedrel::zone<mpq_class>> leaf_zones = bool_to_zone(time_vec, value_vec[$2], 0);
            for(int i=0; i<leaf_zones.size(); i++){
                zp_res.push_back(make_shared<ided_zone<mpq_class>>(leaf_zones[i], i, vector<int>()));
            }
            dtre_stack.push_back(zp_res);

            sexp_stack.push_back("NOT x"+to_string($2));

            shared_ptr<diag_tree> dtnode = 
                make_shared<diag_tree>(op_type::atomic, sexp_stack.back(), dtre_stack.back(),
                    nullptr, nullptr);
            dt_stack.push_back(dtnode);
      }
      | porv
      | dtre K_PLUS {
            vector<shared_ptr<gen_zone>> imp;
            vector<shared_ptr<gen_zone>> imp_copy;
            imp = dtre_stack.back();
            dtre_stack.pop_back();

            for(int i=0; i<imp.size(); i++){
                shared_ptr<gen_zone> zppclone = imp[i]->clone();
                shared_ptr<ided_zone<mpq_class>> zpptemp = 
                    dynamic_pointer_cast<ided_zone<mpq_class>>(zppclone);
                zpptemp->set_chids({i});
                imp_copy.push_back(zppclone);
            }

            vector<shared_ptr<gen_zone>> zp_res = 
                gen_transitive_closure(imp_copy);

            for(int i=0; i<zp_res.size(); i++){
                shared_ptr<ided_zone<mpq_class>> zpptemp = 
                    dynamic_pointer_cast<ided_zone<mpq_class>>(zp_res[i]);
                    zpptemp->set_myid(i);
            }
            dtre_stack.push_back(zp_res);

            string exp_str = sexp_stack.back();
            sexp_stack.pop_back();
            sexp_stack.push_back(exp_str+"^+");

            shared_ptr<diag_tree> spdt = dt_stack.back();
            dt_stack.pop_back();

            shared_ptr<diag_tree> dtnode = 
                make_shared<diag_tree>(op_type::kplus, sexp_stack.back(), dtre_stack.back(),
                    spdt, nullptr);
            dt_stack.push_back(dtnode);

      }
      | T_LEFT dtre T_RIGHT {
            string exp_str = sexp_stack.back();
            sexp_stack.pop_back();
            sexp_stack.push_back("("+exp_str+")");

            shared_ptr<diag_tree> spdt = dt_stack.back();
            dt_stack.pop_back();

            shared_ptr<diag_tree> dtnode = 
                make_shared<diag_tree>(op_type::brac, sexp_stack.back(), dtre_stack.back(),
                    spdt, nullptr);
            dt_stack.push_back(dtnode);
      }
      | dtre T_SEQCOMP dtre{
            vector<shared_ptr<gen_zone>> imp1, imp2;
            vector<shared_ptr<gen_zone>> imp1_copy, imp2_copy;

            imp2 = dtre_stack.back();
            dtre_stack.pop_back();
            imp1 = dtre_stack.back();
            dtre_stack.pop_back();

            for(int i=0; i<imp1.size(); i++){
                shared_ptr<gen_zone> zppclone = imp1[i]->clone();
                shared_ptr<ided_zone<mpq_class>> zpptemp = 
                    dynamic_pointer_cast<ided_zone<mpq_class>>(zppclone);
                zpptemp->set_chids({i});
                imp1_copy.push_back(zppclone);
            }

            for(int i=0; i<imp2.size(); i++){
                shared_ptr<gen_zone> zppclone = imp2[i]->clone();
                shared_ptr<ided_zone<mpq_class>> zpptemp = 
                    dynamic_pointer_cast<ided_zone<mpq_class>>(zppclone);
                zpptemp->set_chids({i});
                imp2_copy.push_back(zppclone);
            }

            vector<shared_ptr<gen_zone>> zp_res = 
                gen_concatenation(imp1_copy, imp2_copy);

            for(int i=0; i<zp_res.size(); i++){
                shared_ptr<ided_zone<mpq_class>> zpptemp = 
                    dynamic_pointer_cast<ided_zone<mpq_class>>(zp_res[i]);
                zpptemp->set_myid(i);
            }
            dtre_stack.push_back(zp_res);

            string exp_str2 = sexp_stack.back();
            sexp_stack.pop_back();
            string exp_str1 = sexp_stack.back();
            sexp_stack.pop_back();
            sexp_stack.push_back(exp_str1+" DOT "+exp_str2);

            shared_ptr<diag_tree> spdt2 = dt_stack.back();
            dt_stack.pop_back();
            shared_ptr<diag_tree> spdt1 = dt_stack.back();
            dt_stack.pop_back();

            shared_ptr<diag_tree> dtnode = 
                make_shared<diag_tree>(op_type::concat, sexp_stack.back(), dtre_stack.back(),
                    spdt1, spdt2);
            dt_stack.push_back(dtnode);
      }
      | dtre T_AND dtre{
            vector<shared_ptr<gen_zone>> imp1, imp2;

            imp2 = dtre_stack.back();
            dtre_stack.pop_back();
            imp1 = dtre_stack.back();
            dtre_stack.pop_back();

            vector<shared_ptr<gen_zone>> zp_res = 
                gen_intersection(imp1, imp2);

            for(int i=0; i<zp_res.size(); i++){
                shared_ptr<ided_zone<mpq_class>> zpptemp = 
                    dynamic_pointer_cast<ided_zone<mpq_class>>(zp_res[i]);
                zpptemp->set_myid(i);
            }
            dtre_stack.push_back(zp_res);

            string exp_str2 = sexp_stack.back();
            sexp_stack.pop_back();
            string exp_str1 = sexp_stack.back();
            sexp_stack.pop_back();
            sexp_stack.push_back(exp_str1+" AND "+exp_str2);

            shared_ptr<diag_tree> spdt2 = dt_stack.back();
            dt_stack.pop_back();
            shared_ptr<diag_tree> spdt1 = dt_stack.back();
            dt_stack.pop_back();

            shared_ptr<diag_tree> dtnode = 
                make_shared<diag_tree>(op_type::sinter, sexp_stack.back(), dtre_stack.back(),
                    spdt1, spdt2);
            dt_stack.push_back(dtnode);
      }
      | dtre T_OR dtre{
            vector<shared_ptr<gen_zone>> imp1, imp2;
            vector<shared_ptr<gen_zone>> zp_res;
            
            imp2 = dtre_stack.back();
            dtre_stack.pop_back();
            imp1 = dtre_stack.back();
            dtre_stack.pop_back();

            for(int i=0; i<imp1.size(); i++){
                shared_ptr<gen_zone> zppclone = imp1[i]->clone();
                shared_ptr<ided_zone<mpq_class>> zpptemp = 
                    dynamic_pointer_cast<ided_zone<mpq_class>>(zppclone);
                zpptemp->set_chids({i,-1});

                zp_res.push_back(zppclone);
            }

            for(int i=0; i<imp2.size(); i++){
                shared_ptr<gen_zone> zppclone = imp2[i]->clone();
                shared_ptr<ided_zone<mpq_class>> zpptemp = 
                    dynamic_pointer_cast<ided_zone<mpq_class>>(zppclone);
                zpptemp->set_chids({-1,i});

                zp_res.push_back(zppclone);
            }

            vector<shared_ptr<gen_zone>> zp_filtered = gen_filter(zp_res);

            for(int i=0; i<zp_filtered.size(); i++){
                shared_ptr<ided_zone<mpq_class>> zpptemp = 
                    dynamic_pointer_cast<ided_zone<mpq_class>>(zp_filtered[i]);
                zpptemp->set_myid(i);
            }
            dtre_stack.push_back(zp_filtered);

            string exp_str2 = sexp_stack.back();
            sexp_stack.pop_back();
            string exp_str1 = sexp_stack.back();
            sexp_stack.pop_back();
            sexp_stack.push_back(exp_str1+" OR "+exp_str2);

            shared_ptr<diag_tree> spdt2 = dt_stack.back();
            dt_stack.pop_back();
            shared_ptr<diag_tree> spdt1 = dt_stack.back();
            dt_stack.pop_back();

            shared_ptr<diag_tree> dtnode = 
                make_shared<diag_tree>(op_type::sunion, sexp_stack.back(), dtre_stack.back(),
                    spdt1, spdt2);
            dt_stack.push_back(dtnode);
      }
      | dtre T_SLEFT T_FLOAT T_FLOAT T_SRIGHT{
            vector<shared_ptr<gen_zone>> imp;
            imp = dtre_stack.back();
            dtre_stack.pop_back();

            int ftexp10 = exponentOf10(placesAfterDot(string($3)));
            int fndot = removeDot(string($3));
            mpq_class dmin1 = mpq_class(fndot, ftexp10);
            ftexp10 = exponentOf10(placesAfterDot(string($4)));
            fndot = removeDot(string($4));
            mpq_class dmax1 = mpq_class(fndot, ftexp10);

            vector<shared_ptr<gen_zone>> zp_res;
            int count = 0;
            for(int i=0; i<imp.size(); i++){
                shared_ptr<ided_zone<mpq_class>> zpptemp = 
                    dynamic_pointer_cast<ided_zone<mpq_class>>(imp[i]);

                timedrel::zone<mpq_class> ztemp = zpptemp->get_myzone();
                ztemp = timedrel::zone<mpq_class>::duration_restriction(ztemp, 
                    timedrel::lower_bound<mpq_class>::closed(dmin1), 
                    timedrel::upper_bound<mpq_class>::closed(dmax1)
                );
                if(ztemp.is_nonempty()){
                    vector<int> id_vec = {i};
                    zp_res.push_back(make_shared<ided_zone<mpq_class>>(ztemp, count, id_vec));
                    count++;
                }
            }

            dtre_stack.push_back(zp_res);

            string exp_str = sexp_stack.back();
            sexp_stack.pop_back();
            sexp_stack.push_back(exp_str+" ["+string($3)+" "+string($4)+"]");

            shared_ptr<diag_tree> spdt = dt_stack.back();
            dt_stack.pop_back();

            shared_ptr<diag_tree> dtnode = 
                make_shared<diag_tree>(op_type::durest, sexp_stack.back(), dtre_stack.back(),
                    spdt, nullptr);
            dt_stack.push_back(dtnode);

            free($3);
            free($4);
      }
;

compop: GEQ| LEQ;
edgeop: T_RISE | T_FALL;

porv: X_STRING compop magconst {
            vector<string> tfvec = value_vec[$1];
            vector<string> tfbool;

            tfbool = get_bool_from_porv(tfvec, $2, string($3));

            vector<shared_ptr<gen_zone>> zp_res;
            vector<zone<mpq_class>> leaf_zones = bool_to_zone(time_vec, tfbool, 1);
            for(int i=0; i<leaf_zones.size(); i++){
                zp_res.push_back(make_shared<ided_zone<mpq_class>>(leaf_zones[i], i, vector<int>()));
            }
            dtre_stack.push_back(zp_res);

            string comp_str = ($2 == 1)?string("<="):string(">=");
            sexp_stack.push_back("x"+to_string($1)+comp_str+string($3));

            shared_ptr<diag_tree> dtnode = 
                make_shared<diag_tree>(op_type::atomic, sexp_stack.back(), dtre_stack.back(),
                    nullptr, nullptr);
            dt_stack.push_back(dtnode);

            free($3);
      }
      | edgeop X_STRING compop magconst {
            vector<string> tfvec = value_vec[$2];
            vector<string> tfbool;

            tfbool = get_bool_from_porv(tfvec, $3, string($4));

            vector<shared_ptr<gen_zone>> zp_res;
            vector<timedrel::zone<mpq_class>> leaf_zones = bool_to_edge_zone(time_vec, tfbool, $1);
            for(int i=0; i<leaf_zones.size(); i++){
                zp_res.push_back(make_shared<ided_zone<mpq_class>>(leaf_zones[i], i, vector<int>()));
            }
            dtre_stack.push_back(zp_res);

            string edge_str = ($1 == 1)?string("+@"):string("+@");
            string comp_str = ($3 == 1)?string("<="):string(">=");
            sexp_stack.push_back(edge_str+"x"+to_string($2)+comp_str+string($4));

            shared_ptr<diag_tree> dtnode = 
                make_shared<diag_tree>(op_type::atomic, sexp_stack.back(), dtre_stack.back(),
                    nullptr, nullptr);
            dt_stack.push_back(dtnode);

            free($4);
      }
;

%%

void print_interval(pair<mpq_class,mpq_class> iv){
    cout<<"{"<<iv.first<<","<<iv.second<<"}"<<endl;
}

int main(int argc, char** argv) {

    read_signal_file(argv[2], time_vec, value_vec);
    cout<<"time_vec size:"<<time_vec.size()<<endl;
    cout<<"value_vec size:"<<value_vec.size()<<endl;

    // Do timed pattern matching
    if(argc == 3){
        yyin = fopen(argv[1],"r");
        do {
            yyparse();
        } while(!feof(yyin));
    }

    cout<<"Expression:"<<sexp_stack.back()<<endl;
    vector<shared_ptr<gen_zone>> mset = dtre_stack.back();
    cout<<"Indexed zones:"<<endl;
    for(auto spzone : mset){
        shared_ptr<ided_zone<mpq_class>> spidz = 
                    dynamic_pointer_cast<ided_zone<mpq_class>>(spzone);
        ided_zone<mpq_class> idz(spidz->get_myzone(), spidz->get_myid(), spidz->get_chids());
        cout<<idz<<endl;
    }

    /*
    vector<string> formula_list;
    vector<zone<mpq_class>> zone_list;
    dt_stack.back()->explore_tree(0, formula_list, zone_list);
    cout<<"Formulae and corresponding zones:"<<endl;
    for(int i=0; i<formula_list.size(); i++){
        cout<<formula_list[i]<<endl;
        cout<<zone_list[i]<<endl;
        cout<<"-------------------------"<<endl;
    }
    */

    vector<string> formula_list;
    vector<zone<mpq_class>> zone_list;
    vector<pair<mpq_class,mpq_class>> interval_list;
    mpq_class istart(5,2),iend(15,2);
    pair<mpq_class,mpq_class> top_interval(istart, iend);
    cout<<"Infer intervals:"<<endl;
    print_interval(top_interval);
    cout<<"-------------------------"<<endl;
    dt_stack.back()->explore_tree_interval(0, top_interval, formula_list, zone_list, interval_list);
    for(int i=0; i<formula_list.size(); i++){
        cout<<formula_list[i]<<endl;
        cout<<zone_list[i]<<endl;
        print_interval(interval_list[i]);
        cout<<"-------------------------"<<endl;
    }

	return 0;
}

void yyerror(const char* s) {
    fprintf(stderr, "Parse error: %s\n", s);
    exit(1);
}