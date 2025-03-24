

class mvlc_ts :public ModuleFather {
public:

mvlc_ts(){}

void initEvent() override {
    implemented=false;
}

void readData(ifstream *f) override {}

void read(ifstream *f, Int_t &broken_event_count) override {}

void write(Int_t &broken_event_count) override {}

void createTree(TTree *EventTree) override{}

void histoLOOP(TFile *treeFile,std::vector<TDirectory*>& refereciasFolders) override{}

};
