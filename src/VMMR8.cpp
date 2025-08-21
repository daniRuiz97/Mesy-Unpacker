//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Daughter class to translate data from a VMMR8 module:                                                                //
//                                                                                                                      //
// Inherits structure from the ModuleFather.h                                                                           //
//  Internal methods designed for the VMMR8 modules (info: https://www.mesytec.com/products/datasheets/VMMR.pdf page 3)  //                                                                      //                                                                                                                      //
// The methods need to be summoned in the CORRECT OREDER, the Unpacker takes care of this thanks to the config file     //
// The output is:                                                                                                       //
//    1-> Root branches containing relevant info that will be sitched to a TTree                                         //
//    2-> Histograms done with the info of the branches, used to compare with Mesytec as a sanity check                //
// DFR                                                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#include "../include/ModuleFather.h"
#include <iostream>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"

#include <stdio.h>
#include <string.h>
#include <fstream>

#include <sstream>

#include <sstream>
#include <cstdio>

#include <string>

using std::ifstream;
using namespace std;
class VMMR8 : public ModuleFather {
public:

int module;                                                           
string nick; 
string event;

// Declare internal variables 
Int_t   VMMR8_Multiplicity;
Int_t   VMMR8_Channel[64];
ULong_t VMMR_Value[64];
Int_t   VMMR8_time_diff_mul;
Int_t   VMMR8_Time_diff_bus[64];
ULong_t VMMR8_Time_diff_value[64];
ULong_t VMMR8_Extended_timestamp;
ULong_t VMMR8_Event_timestamp;

TH1F* VMMR8_histo[16][64];

char VMMR8_name[64], VMMR8_histo_name[64];

std::shared_ptr<int> contRaul=std::make_shared<int>(0);;


VMMR8(int indexModule, string Mote, string evet){
  module=indexModule;
  nick=Mote;
  event=evet;
}

// Initialize variables
void initEvent() override {
  VMMR8_Multiplicity = 0;
  VMMR8_time_diff_mul = 0;
  VMMR8_Extended_timestamp = 0;
  VMMR8_Event_timestamp = 0;
  

  for (int i=0; i<64; i++) {
    VMMR_Value[i] = 0;
    VMMR8_Time_diff_bus[i] = 0;
    VMMR8_Time_diff_value[i] = 0;
  }
}

// Method to transcribe Data
// For a frame that has been validadted, this method transcribes relevant parameteres from hex to numerical 
void readData(ifstream *f) override {
    unsigned char vmmr8_data[4];
    f->read((char*) vmmr8_data, 4);
    
  unsigned char vmmr8_data_check = (vmmr8_data[3] >> 4) & 0b1111; // Select header for data type check
    
  if (vmmr8_data_check == 0b0001) { // Good header 
    if (((vmmr8_data[0] + (vmmr8_data[1] << 8) + (vmmr8_data[2] << 16) + (vmmr8_data[3] << 24)) != 0)) {
      //VMMR8_Multiplicity=0;      
      unsigned long channel = (vmmr8_data[1] >> 4) + (vmmr8_data[2] << 4); // Channel number
      unsigned long value = ( vmmr8_data[0] + (vmmr8_data[1] << 8) ) & 0b0000111111111111; // Value in the channel
      Int_t bus = vmmr8_data[3] & 0b00001111; // Bus (optical link) number
      
            
      if ((channel >=0) && (channel <= 63) && (0 <= bus ) && (bus <= 8) && (VMMR8_Multiplicity< 64)  ){
        // For the root branches
        VMMR8_Channel[VMMR8_Multiplicity] = channel;
        VMMR_Value[VMMR8_Multiplicity] = value;
        VMMR8_Time_diff_bus[VMMR8_Multiplicity] = bus;
        
        // Control histogram
        VMMR8_histo[bus][channel]->Fill(value);
        VMMR8_Multiplicity++;

        
      }
      //if (64<VMMR8_Multiplicity) cout<< "HOwOla multi "<< VMMR8_Multiplicity  <<" Ch " << channel << " Buss "<< bus <<endl; 
    }
  } // != 0
  
  // Never eneters here: possible redundant info (need to check)
  if(vmmr8_data_check == 0b0011){
    unsigned int bus = vmmr8_data[3] & 0b00001111;
    unsigned long value = ( vmmr8_data[0] + (vmmr8_data[1] << 8) ) & 0b1111111111111111;
    
    if ((bus >= 0) && (bus < 16)){
    VMMR8_time_diff_mul++;
    if (VMMR8_time_diff_mul > 16*3-1) {VMMR8_time_diff_mul = 16*3-1;}
    }
  }
  
  // Time stamps
  if(vmmr8_data_check == 0b0010){
    unsigned long value = ( vmmr8_data[0] + (vmmr8_data[1] << 8) ) & 0b1111111111111111;
    VMMR8_Extended_timestamp = value;
  }
  
  if (((vmmr8_data[3] >> 6) & 0b11) == 3){ // Reading timestamp in the end of event
      VMMR8_Event_timestamp = (vmmr8_data[0] + (vmmr8_data[1] << 8) + (vmmr8_data[2] << 16) + ((vmmr8_data[3] & 0b00111111) << 24));
  }

}

// Method to filter make sure the frame is good (NOTE: I should have though of a better name, change for final version)
void read(ifstream *f, Int_t &broken_event_count) override {
  unsigned char block_read_header[4];
  f->read((char*) block_read_header, 4); // should be Type = 0xf5
  unsigned short module_event_length = (block_read_header[0] + (block_read_header[1] << 8) ) & 0b0001111111111111;
  int moduloID=(block_read_header[2])& 0b11111111;
  //cout << "VMMR8 lenght " << module_event_length << endl;
  acarreo= module_event_length+1;
  
  if (module_event_length > 1) {
    //cout << "Hola" <<endl;
    unsigned char module_header[4];
    f->read((char*) module_header, 4);
    for (int i=0; i < module_event_length-1; i++) {
      readData(f);
    }

  } else if (module_event_length == 1) {
      unsigned char dontCare[4];
      f->read((char*) dontCare, 4);
      broken_event_count++;
    }
}

 void write(Int_t &broken_event_count) override {
}

// Create branches for the Ttree
void createTree(TTree *EventTree) override{
  int d=module;
                                                            
  TString LongMultName = Form("VMMR8_%i_Multiplicity", d); TString LongMultNameI = Form("VMMR8_%i_Multiplicity/I", d );// Generate a unique name for each histogram
  TString LongMultChan = Form("VMMR8_%i_Channels", d);     TString LongMultChanI = Form("VMMR8_%i_Channels[VMMR8_%i_Multiplicity]/I", d,d); 
  TString LongMultValu = Form("VMMR8_%i_Values", d);       TString LongMultValuL = Form("VMMR_%i_Values[VMMR8_%i_Multiplicity]/L", d,d );


  TString TDCtimeDiffMul = Form("VMMR8_%i_time_diff_mul", d);   TString TDCtimeDiffMulI = Form("VMMR8_%i_time_diff_mul/I", d );// Generate a unique name for each histogram 
  //TString TDCtimeDiffbus = Form("VMMR8_%i_time_diff_buss",d);   TString TDCtimeDiffbusI = Form("VMMR8_%i_time_diff_buss[VMMR8_%i_time_diff_mul]/I", d,d );
  TString TDCtimeDiffbus = Form("VMMR8_%i_time_diff_buss",d);   TString TDCtimeDiffbusI = Form("VMMR8_%i_time_diff_buss[VMMR8_%i_Multiplicity]/I", d,d );
  TString TDCtimeDiffVal = Form("VMMR8_%i_time_diff_Values",d); TString TDCtimeDiffValL = Form("VMMR8_%i_time_diff_Values[VMMR8_%i_time_diff_mul]/L", d,d );
    
  TString ExtendTimestamp = Form("VMMR8_%i_Extended_timestamps", d );TString ExtendTimestampL = Form("VMMR8_%i_Extended_timestamps/L", d);// Generate a unique name for each histogram
  TString EventTimestamp  = Form("VMMR8_%i_Event_timestamps", d );   TString EventTimestampL  = Form("VMMR8_%i_Event_timestamps/L", d);                                                    
  
  
  EventTree->Branch(LongMultName,  &VMMR8_Multiplicity,  LongMultNameI);
  EventTree->Branch(LongMultChan,   VMMR8_Channel,       LongMultChanI);
  EventTree->Branch(LongMultValu,   VMMR_Value,          LongMultValuL);
  
  EventTree->Branch(TDCtimeDiffMul, &VMMR8_time_diff_mul,  TDCtimeDiffMulI);
  EventTree->Branch(TDCtimeDiffbus,  VMMR8_Time_diff_bus,  TDCtimeDiffbusI);
  EventTree->Branch(TDCtimeDiffVal,  VMMR8_Time_diff_value,TDCtimeDiffValL);
                                                                                                   
  EventTree->Branch(ExtendTimestamp,  &VMMR8_Extended_timestamp, ExtendTimestampL);
  EventTree->Branch(EventTimestamp,     &VMMR8_Event_timestamp,  EventTimestampL);
  
}                                                                                                                  

// Sanity check histograms
void histoLOOP(TFile *treeFile,std::vector<TDirectory*>& refereciasFolders) override{
  int d=module;
  TDirectory *VMMR8_dir = refereciasFolders[stoi(event)]->mkdir(Form("%s_%i",nick.c_str(),d));
  /**/
  TDirectory *bus_dirs[16];
  for (int i = 0; i < 16; i++) {
    bus_dirs[i]=VMMR8_dir->mkdir(Form("bus_%d", i));
  }
  for (Int_t i=0; i<16; i++) {
    VMMR8_dir->cd();
    bus_dirs[i]->cd();
    for (Int_t j=0; j<64; j++) {
      snprintf(VMMR8_name,sizeof(VMMR8_name),"VMMR_%i",j);
      snprintf(VMMR8_histo_name, sizeof(VMMR8_histo_name),"VMMR_%i ; Channel ; Counts",j);
      VMMR8_histo[i][j] = new TH1F(VMMR8_name, VMMR8_histo_name, 4096, 0, 4096);
    }
    gDirectory->cd("..");
    gDirectory->cd("..");
  }
}

};
