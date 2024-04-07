#ifndef DIAG_TREE_HPP
#define DIAG_TREE_HPP 1

#include <gmpxx.h>
#include "ided_zone.hpp"

enum op_type { atomic, sinter, concat, kplus, sunion, brac, durest};

class diag_tree
{
    op_type node_type;
    string exp_text;
    std::vector<std::shared_ptr<gen_zone>> indexed_zone;
    std::shared_ptr<diag_tree> left = nullptr;
    std::shared_ptr<diag_tree> right = nullptr;
public:
    diag_tree(op_type nt, string et,
        std::vector<std::shared_ptr<gen_zone>> iz_in, 
        std::shared_ptr<diag_tree> l,
        std::shared_ptr<diag_tree> r): node_type(nt), exp_text(et), indexed_zone(iz_in), left(l), right(r){}

    std::vector<std::shared_ptr<gen_zone>> get_indexed_zones(){
        return indexed_zone;
    }
    void explore_tree(int index, std::vector<string> &for_names, std::vector<timedrel::zone<mpq_class>> &zone_list){
        if( (index < 0) or (index >= indexed_zone.size()) ){
            return ;
	    }

        shared_ptr<ided_zone<mpq_class>> zpptemp = 
                dynamic_pointer_cast<ided_zone<mpq_class>>(indexed_zone[index]);

        for_names.push_back(exp_text);
        zone_list.push_back(zpptemp->get_myzone());

        std::vector<int> chids = zpptemp->get_chids();

        if(node_type == op_type::atomic){

        }else if(node_type == op_type::sinter || node_type == op_type::concat){
            if(chids.size() != 2){
                return ;
            }
            if(left != nullptr){
                left->explore_tree(chids[0], for_names, zone_list);    
            }
            if(right != nullptr){
                right->explore_tree(chids[1], for_names, zone_list);    
            }
        }else if(node_type == op_type::kplus){
            if(left != nullptr){
                for(auto id : chids){
                    left->explore_tree(id, for_names, zone_list);
                }
            }
        }else if(node_type == op_type::sunion){
            if(left != nullptr && chids[0] != -1){
                left->explore_tree(chids[0], for_names, zone_list);
            }
            if(right != nullptr && chids[1] != -1){
                right->explore_tree(chids[1], for_names, zone_list);
            }
        }else if(node_type == op_type::brac){
            if(left != nullptr){
                left->explore_tree(index, for_names, zone_list);
            }
        }else if(node_type == op_type::durest){
            if(left != nullptr){
                left->explore_tree(chids[0], for_names, zone_list);
            }
        }

        return ;
    }

    void explore_tree_interval(int index, std::pair<mpq_class,mpq_class> iasp,
            std::vector<string> &for_names, std::vector<timedrel::zone<mpq_class>> &zone_list,
            std::vector<std::pair<mpq_class,mpq_class>> &interval_list){
        if( (index < 0) or (index >= indexed_zone.size()) ){
            return ;
        }

        shared_ptr<ided_zone<mpq_class>> zpptemp = 
                dynamic_pointer_cast<ided_zone<mpq_class>>(indexed_zone[index]);

        if(not zone_contains_interval(zpptemp->get_myzone(), iasp.first, iasp.second)){
            std::cerr<<"Expression:"<<exp_text<<endl;
            std::cerr<<"Zone:"<<zpptemp->get_myzone()<<endl;
            std::cerr<<"Pair:{"<<iasp.first<<","<<iasp.second<<"}"<<endl;
            std::cerr<<"Zone does not contain point"<<endl;
            exit(0);
        }
        for_names.push_back(exp_text);
        zone_list.push_back(zpptemp->get_myzone());
        interval_list.push_back(iasp);

        std::vector<int> chids = zpptemp->get_chids();

        if(node_type == op_type::atomic){

        }else if(node_type == op_type::sinter){
            if(chids.size() != 2){
                return ;
            }
            if(left != nullptr){
                left->explore_tree_interval(chids[0], iasp, for_names, zone_list, interval_list);    
            }
            if(right != nullptr){
                right->explore_tree_interval(chids[1], iasp, for_names, zone_list, interval_list);    
            }
        }else if(node_type == op_type::concat){
            if(chids.size() != 2){
                return ;
            }
            /* Infer sub intervals concatenated together */
            std::vector<std::shared_ptr<gen_zone>> left_zones = left->get_indexed_zones();
            std::vector<std::shared_ptr<gen_zone>> right_zones = right->get_indexed_zones();
            shared_ptr<ided_zone<mpq_class>> zpp_left = 
                dynamic_pointer_cast<ided_zone<mpq_class>>(left_zones[chids[0]]);
            shared_ptr<ided_zone<mpq_class>> zpp_right = 
                dynamic_pointer_cast<ided_zone<mpq_class>>(right_zones[chids[1]]);

            /* VIMP: TODO: Check the correctness of infer_seq_comp */
            mpq_class t2 = infer_seq_comp(zpptemp->get_myzone(), zpp_left->get_myzone(),
                zpp_right->get_myzone(), iasp);

            std::pair<mpq_class,mpq_class> linterval(iasp.first, t2);
            std::pair<mpq_class,mpq_class> rinterval(t2, iasp.second);

            if(left != nullptr){
                left->explore_tree_interval(chids[0], linterval, for_names, zone_list, interval_list);    
            }
            if(right != nullptr){
                right->explore_tree_interval(chids[1], rinterval, for_names, zone_list, interval_list);    
            }
        }else if(node_type == op_type::kplus){
            if(left != nullptr){
                /* Infer sub intervals concatenated together */
                std::vector<std::shared_ptr<gen_zone>> left_zones = left->get_indexed_zones();
                std::vector<timedrel::zone<mpq_class>> zlist;
                for(auto id : chids){
                    shared_ptr<ided_zone<mpq_class>> zpp_child = 
                        dynamic_pointer_cast<ided_zone<mpq_class>>(left_zones[id]);
                    zlist.push_back(zpp_child->get_myzone());
                }

                /* VIMP: TODO: Check the correctness of infer_mult_seq_comp */
                std::vector<std::pair<mpq_class,mpq_class>> plist = infer_mult_seq_comp(zpptemp->get_myzone(),
                    zlist, iasp);

                for(int i=0; i<chids.size(); i++){
                    left->explore_tree_interval(chids[i], plist[i], for_names, zone_list, interval_list);
                }
            }
        }else if(node_type == op_type::sunion){
            if(left != nullptr && chids[0] != -1){
                left->explore_tree_interval(chids[0], iasp, for_names, zone_list, interval_list);
            }
            if(right != nullptr && chids[1] != -1){
                right->explore_tree_interval(chids[1], iasp, for_names, zone_list, interval_list);
            }
        }else if(node_type == op_type::brac){
            if(left != nullptr){
                left->explore_tree_interval(index, iasp, for_names, zone_list, interval_list);
            }
        }else if(node_type == op_type::durest){
            if(left != nullptr){
                left->explore_tree_interval(chids[0], iasp, for_names, zone_list, interval_list);
            }
        }

        return ;
    }
    ~diag_tree(){}
	
};

#endif