//
//  main.cpp
//  HW6
//
//  Created by 王吉朗 on 2023/5/20.
//

#include <iostream>
#include "unordered_map"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "istream"
#include <vector>
#include "queue"
#include "math.h"

using namespace std;

class Sentence {
public:
    unordered_map<string, int> srored;
    unordered_map<string, double> tf;
    double idf = 0;
    int id;
    int word_num = 0;
};

struct Query {
    string query;
    double idf;
    double tf;
};

vector<string> tokenizer (string str) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool isCapital (char ch) {
    if(ch >= 'A' && ch <= 'Z') {
        return true;
    }
    else return false;
}

struct IsNotAlpha {
    bool operator()(char c) {
        return !std::isalpha(static_cast<unsigned char>(c));
    }
};

struct CompElement {
    bool operator()(const Sentence& a, const Sentence& b) {
        if (a.idf != b.idf) {
            return a.idf < b.idf;
        }
        return a.id > b.id;
    }
};

struct compforidf {
    bool operator() (const Query &a, const Query &b) {
        if (a.idf != b.idf) {
            return a.idf < b.idf;
        }
        return a.tf < b.tf;
    }
};


int main(int argc, const char * argv[]) {
    ifstream corpusFile(argv[1]);
    static int total_words = 0;
    string line;
    int k = atoi(argv[3]);
    vector<Sentence> AllSentence;
    while (getline(corpusFile,line)) {
        total_words++;
        string number;
        istringstream tempss(line);
        getline(tempss, number, ',');
        vector<string> tempstr;
        tempstr = tokenizer(line);
        for (string &str : tempstr) {
            str.erase(remove_if(str.begin(), str.end(), IsNotAlpha()),str.end()); //delete all nonalphabet
            for (char &ch : str) {
                if (isCapital(ch)) {
                    ch = tolower(ch);
                }
            }
            //upper to lower
        }
        Sentence tempsen;
        int sentnum = stoi(number);
        for (string &str : tempstr) {
            tempsen.word_num++;
            if (tempsen.srored.find(str) != tempsen.srored.end()) {
                tempsen.srored[str] = tempsen.srored.at(str) + 1;
            }
            else {
                tempsen.srored[str] = 1;
            }
            
        }
        for (string &str : tempstr) {
            tempsen.tf[str] = tempsen.srored.at(str) / (double)tempsen.word_num;
        }
        tempsen.id = sentnum;
        AllSentence.push_back(tempsen);
    }
    
    //read query
    vector<string> keywords;
    ifstream queryFile(argv[2]);
    string query;
    static unordered_map<string, double> stored_idf;
    while (getline(queryFile,query)) {
        keywords = tokenizer(query);
        for (auto &str : keywords) {
            for (char &ch : str) {
                if (isCapital(ch)) {
                    ch = tolower(ch);
                }
            }
        }
        
        //caculate keyword idf
        for (auto &str : keywords) {
            try {
                int idf = stored_idf.at(str);
            } catch (const out_of_range& e) {
                int appear_count = 0;
                for(auto &temp_sen : AllSentence) {
                    if (temp_sen.srored.find(str) != temp_sen.srored.end()) {
                        appear_count++;
                    }
                }
                stored_idf[str] = log10((double)total_words/(double)appear_count);
            }
        }
        
        vector<Query> AllKeyword;
        
        for (auto &str : keywords) {
            Query tempquery;
            tempquery.query = str;
            tempquery.idf = stored_idf.at(str);
            AllKeyword.push_back(tempquery);
        }
        
        //store idf_sum in allsen
        //to be edited
        //find topk idf
        for(auto &temp_sent : AllSentence) {
            //store tf
            for (auto &temp : AllKeyword) {
                try {
                    temp.tf = temp_sent.tf.at(temp.query);
                } catch (const out_of_range& e) {
                    temp.tf = 0;
                }
                //problem
            }
            //find topk idf*tf
            priority_queue<Query, vector<Query>, compforidf> topidf;
            for (auto &temp : AllKeyword) {
                if (topidf.size() < k) {
                    topidf.push(temp);
                }
                else if (topidf.top().idf <= temp.idf) {
                    topidf.push(temp);
                    topidf.pop();
                }
            }
            //sum rank
            for (int i = 0; i < k; i++) {
                temp_sent.idf += (topidf.top().idf * topidf.top().tf);
                topidf.pop();
            }
        }
        
        
        
        
        //find top k
        priority_queue<Sentence, vector<Sentence>, CompElement> topk;
        for (auto &tempsen : AllSentence) {
            if (topk.size() < k) {
                topk.push(tempsen);
            }
            else {
                if (topk.top().idf <= tempsen.idf) {
                    topk.push(tempsen);
                    topk.pop();
                }
            }
        }
        //print k
        vector<Sentence> result;
        for (int i = 0; i < k; i++) {
            result.push_back(topk.top());
            topk.pop();
        }
        for (int i = k-1; i >= 0; i--) {
            if (result[i].idf == 0) {
                printf("-1");
            }
            else {
                printf("%d", result[i].id);
            }
            
            if (i != 0) {
                printf(" ");
            }
        }
        printf("\n");
        result.clear();
        
    }
    return 0;
}
