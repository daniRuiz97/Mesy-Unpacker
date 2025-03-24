#ifndef MODULE_FATHER_H
#define MODULE_FATHER_H

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include <fstream>
using std::ifstream;

class ModuleFather {
public:
    int acarreo=0;
    virtual void read(ifstream *f,Int_t &broken_event_count)=0;
    virtual void readData(ifstream *f)=0;
    virtual void write(Int_t &broken_event_count)=0;
    virtual void createTree(TTree *EventTree)=0;
    virtual void histoLOOP(TFile *treeFile,std::vector<TDirectory*>& refereciasFolders)=0; 
    virtual void initEvent()=0;
    TTree *EvTree; 

//protected:
    
    bool implemented=true;

    
};

#endif // IO_INTERFACE_H
