using namespace std;

#ifndef HELPER_DTRE_HPP
#define HELPER_DTRE_HPP 1

#include <fstream>

void read_signal_file(char *fName, vector<string> &timeVec, vector<vector<string>> &valueVec){
    ifstream infile(fName);
    string delimiter = ",";
    string line;
    int lineNum=0;
    while (std::getline(infile, line))
    {
        size_t pos = 0;
        std::string token;
        int num=0;
        string s=line;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            if(num==0){
                timeVec.push_back(s);
            }else{
                if(lineNum==0){
                    vector<string> tcVec;
                    tcVec.push_back(token);
                    valueVec.push_back(tcVec);
                }else{
                    valueVec[num-1].push_back(token);
                }
                
            }
            s.erase(0, pos + delimiter.length());
            num++;
        }
        if(lineNum==0){
            vector<string> tcVec;
            tcVec.push_back(s);
            valueVec.push_back(tcVec);
        }else{
            valueVec[num-1].push_back(s);
        }
        lineNum++;
    }
    infile.close();
}

int placesAfterDot(string sf){
    int count=0;
    int i=0;
    int dotFlag=0;
    while(sf[i]!=0){
        if(sf[i]=='.'){
            count = 0;
            dotFlag = 1;
        }
        count++;
        i++;
    }
    if(dotFlag){
        return count-1;
    }else{
        return 0;
    }
}

int exponentOf10(int expo){
    int exp10=1;
    for(int i=0;i<expo;i++){
        exp10*=10;
    }
    return exp10;
}

int removeDot(string fnum){
    char dup[100];
    int i=0,j=0;
    while(fnum[i]!=0){
        if(fnum[i]!='.'){
            dup[j]=fnum[i];
            j++;
        }
        i++;
    }
    dup[i]=0;
    return atoi(dup);
}

vector<string> get_bool_from_porv(vector<string> fvec, int compop, string compval){
    vector<string> tfbool;

    mpq_class ftrat, fvrat;
    int ftexp10 = exponentOf10(placesAfterDot(compval));
    int fndot = removeDot(compval);
    ftrat = mpq_class(fndot, ftexp10);

    for(int i=0; i<fvec.size(); i++){
        ftexp10 = exponentOf10(placesAfterDot(fvec[i]));
        fndot = removeDot(fvec[i]);
        fvrat = mpq_class(fndot, ftexp10);

        if(compop == 1){ // <=
            if(fvrat <= ftrat){
                tfbool.push_back("1");
            }else{
                tfbool.push_back("0");
            }
            
        }else if(compop == 3){ // >=
            if(fvrat >= ftrat){
                tfbool.push_back("1");
            }else{
                tfbool.push_back("0");
            }
        }else{
            cout<<"Comparison operator not supported!"<<endl;
        }
    }

    return tfbool;
}

vector<timedrel::zone<mpq_class>> bool_to_edge_zone(vector<string> timeVec,
    vector<string> fvec, int prevOneIn){
    vector<timedrel::zone<mpq_class>> zlr;

    int prevOne = 0;

    for(int i=0;i<timeVec.size()-1;i++){
        mpq_class timerat;
        int timeexp10 = exponentOf10(placesAfterDot(timeVec[i]));
        int timendot = removeDot(timeVec[i]);
        timerat = mpq_class(timendot, timeexp10);

        int exp10 = exponentOf10(placesAfterDot(fvec[i]));
        int fvectemp = removeDot(fvec[i]);
        if(i==0){
            prevOne = fvectemp;
        }else{
            if(fvectemp != prevOne){
                if( (prevOneIn == 1 && fvectemp > prevOne) || 
                    (prevOneIn == 0 && fvectemp < prevOne) ){
                    timedrel::zone<mpq_class> ztemp = timedrel::zone<mpq_class>::make_from_period_both_anchor(timerat, 
                        timerat);
                    zlr.push_back(ztemp);
                }
                prevOne = fvectemp;
            }
        }
    }
    return zlr;
}

vector<timedrel::zone<mpq_class>> bool_to_zone(vector<string> timeVec, 
    vector<string> fvec, int prevOneIn){
    vector<timedrel::zone<mpq_class>> zlr;

    mpq_class ztstart = 0, ztend = 0;
    int prevOne = 1-prevOneIn;
    
    for(int i=0;i<timeVec.size()-1;i++){
        mpq_class timerat, timeratnext;
        int timeexp10 = exponentOf10(placesAfterDot(timeVec[i]));
        int timendot = removeDot(timeVec[i]);
        timerat = mpq_class(timendot, timeexp10);
        timeexp10 = exponentOf10(placesAfterDot(timeVec[i+1]));
        timendot = removeDot(timeVec[i+1]);
        timeratnext = mpq_class(timendot, timeexp10);

        int exp10 = exponentOf10(placesAfterDot(fvec[i]));
        int fvectemp = removeDot(fvec[i]);
        if(fvectemp==1-prevOneIn){
            if(prevOne == prevOneIn){
                timedrel::zone<mpq_class> ztemp = 
                    timedrel::zone<mpq_class>::make_from({ztstart, ztend, ztstart, ztend, 0, ztend-ztstart},
                        {1,1,1,1,1,1});
                zlr.push_back(ztemp);
            }
            prevOne = 1-prevOneIn;
        }else if(fvectemp==prevOneIn){
            if(prevOne == prevOneIn){
                ztend = timeratnext;
            }else{
                prevOne = prevOneIn;
                ztstart = timerat;
                ztend   = timeratnext;
            }
        }else{
            cerr<<"Error:Non-Boolean predicate"<<endl;
            exit(0);
        }
    }
    if(prevOne == prevOneIn){
        timedrel::zone<mpq_class> ztemp = 
                    timedrel::zone<mpq_class>::make_from({ztstart, ztend, ztstart, ztend, 0, ztend-ztstart},
                        {1,1,1,1,1,1});
                zlr.push_back(ztemp);
    }
    return zlr;
}

#endif // HELPER_DTRE_HPP