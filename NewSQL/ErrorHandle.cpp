//
//  ErrorHandle.cpp
//  NewSQL
//
//  Created by xiaotao Nie on 29/05/2017.
//  Copyright © 2017 xiaotao Nie. All rights reserved.
//

#include "ErrorHandle.hpp"

void printError(string first){
    cout<<"Grammer error near:";
    
    cout<<" "<<first;
    
    cout<<endl;
}
void printError(string first, string second){
    cout<<"Grammer error near:";
    
    cout<<" "<<first<<" "<<second;
    
    cout<<endl;
}
void printError(string first, string second, string third){
    cout<<"Grammer error near:";
    
    cout<<" "<<first<<" "<<second<<" "<<third;
    
    cout<<endl;
}

void notEnoughParam(){
    
    cout<<"The passing paramter are insufficient."<<endl;
    
}

void overFlowParam(){
    
    cout<<"The number opf paramtes is overflow";
    
}

void overFlowParam(string first,string second,string third){
    
    cout<<"The number opf paramtes is overflow near:";
    
    cout<<" "<<first<<" "<<second<<" "<<third;
    
    cout<<endl;

}

