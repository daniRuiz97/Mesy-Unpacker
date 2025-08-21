//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Daughter class to translate data from a MDPP32 module:                                                                       //
//                                                                                                                              //
// Inherits structure from the ModuleFather.h                                                                                   //
//  Internal methods designed for the MDPP32 modules (info:https://www.mesytec.com/products/datasheets/MDPP-32_SCP.pdf page 10)  //                                                                      
// The methods need to be summoned in the CORRECT OREDER, the Unpacker takes care of this thanks to the config file             //
// The output is:                                                                                                               //
//    1-> Root branches containing relevant info that will be sitched to a TTree                                                 //
//    2-> Histograms done with the info of the branches, used to compare with Mesytec as a sanity check                        //
// DFR                                                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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


#include <thread>
#include <chrono>

using std::ifstream;
using namespace std;
class MDPP32 : public ModuleFather {
public:

int module;                                                           
string nick; 
string event;
///////////////////////// MDPP32 SCP ////////////////////////////
// this module have 32 channels but data follows this scheme:  //
//   ADC Value               - channels [0-31]                 //
//   TDC time difference     - channels [32-63]                //
//   Trigger time difference - channels [64(T0),65(T1)]        //
/////////////////////////////////////////////////////////////////

// Declare internal variables 
Int_t   MDPP32_1_Multiplicity;
Int_t   MDPP32_1_Channel[32];
ULong_t MDPP32_1_Value[32];
Int_t   MDPP32_1_TDC_time_diff_mul;
Int_t   MDPP32_1_TDC_time_diff_Channel[32];
ULong_t MDPP32_1_TDC_time_diff_Value[32];
Int_t   MDPP32_1_Trigger_time_diff_mul;
Int_t   MDPP32_1_Trigger_time_diff_Channel[32];
ULong_t MDPP32_1_Trigger_time_diff_Value[32];
ULong_t MDPP32_1_Extended_timestamp;
ULong_t MDPP32_1_Event_timestamp;

TH1F *MDPP32_1_histo[32];
char MDPP32_1_name[64], MDPP32_1_histo_name[64];


int ctr=0;
int contadorEntries=0;

// Constructor de MyClass
MDPP32(int indexModule, string Mote, string evet){
    module=indexModule;
    nick=Mote;
    event=evet;
    /*
    MDPP32_1_Multiplicity = 0;                  
    MDPP32_1_TDC_time_diff_mul = 0;              
    MDPP32_1_Trigger_time_diff_mul = 0;          
    MDPP32_1_Extended_timestamp = 0;              
    MDPP32_1_Event_timestamp= 0 ;                    

    for (int i=0; i<1024*3; i++) {

        if (i<32){                                               
            MDPP32_1_Channel[i] = 0;                                    
            MDPP32_1_Value[i] = 0;                                     
            MDPP32_1_TDC_time_diff_Channel[i] = 0;    
            MDPP32_1_TDC_time_diff_Value[i] = 0;        

        }
        if (i<32){
            MDPP32_1_Trigger_time_diff_Channel[i] = 0;   
            MDPP32_1_Trigger_time_diff_Value[i] = 0;     
        }
 
    }*/
}
// Initialize variables
void initEvent() override {
    MDPP32_1_Multiplicity = 0;                  
    MDPP32_1_TDC_time_diff_mul = 0;              
    MDPP32_1_Trigger_time_diff_mul = 0;          
    MDPP32_1_Extended_timestamp = 0;              
    MDPP32_1_Event_timestamp= 0 ;                    

    for (int i=0; i<1024*3; i++) {

        if (i<32){                                               
            MDPP32_1_Channel[i] = 0;                                    
            MDPP32_1_Value[i] = 0;                                     
            MDPP32_1_TDC_time_diff_Channel[i] = 0;    
            MDPP32_1_TDC_time_diff_Value[i] = 0;        

        }
        if (i<32){
            MDPP32_1_Trigger_time_diff_Channel[i] = 0;   
            MDPP32_1_Trigger_time_diff_Value[i] = 0;     
        }
 
    }
}

// Method to transcribe Data
// For a frame that has been validadted, this method transcribes relevant parameteres from hex to numerical
void readData(ifstream *f) override {
unsigned char mdpp32_scp_data[4];
  f->read((char*) mdpp32_scp_data, 4);
    contadorEntries++;
  unsigned char mdpp32_scp_data_check = (mdpp32_scp_data[3] >> 4) & 0b1111;
   ctr++; 
  if (mdpp32_scp_data_check == 0b0001) { 

    if (((mdpp32_scp_data[0] + (mdpp32_scp_data[1] << 8) + (mdpp32_scp_data[2] << 16) + (mdpp32_scp_data[3] << 24)) != 0)) {
            
      unsigned int channel = mdpp32_scp_data[2] & 0b01111111;
      unsigned long value = ( mdpp32_scp_data[0] + (mdpp32_scp_data[1] << 8) ) & 0b1111111111111111;
            
      if ((channel >= 0) && (channel <= 31) && (MDPP32_1_Multiplicity <=32)){
        // For the root branches
        MDPP32_1_Channel[MDPP32_1_Multiplicity] = channel;
        MDPP32_1_Value[MDPP32_1_Multiplicity] = value;
        MDPP32_1_Multiplicity++; 
        MDPP32_1_histo[channel] -> Fill(value);
        //if (MDPP32_1_Multiplicity > 32-1) { MDPP32_1_Multiplicity = 32-1; }
      }
      if ((channel >= 32) && (channel < 64) && (MDPP32_1_TDC_time_diff_mul <=32)){
        // For the root branches
        MDPP32_1_TDC_time_diff_Channel[MDPP32_1_TDC_time_diff_mul] = channel - 32;
        MDPP32_1_TDC_time_diff_Value[MDPP32_1_TDC_time_diff_mul] = value;
        MDPP32_1_TDC_time_diff_mul++;
        //if (MDPP32_1_TDC_time_diff_mul > 32-1) { MDPP32_1_TDC_time_diff_mul = 32-1; }
      }
      if((channel >= 64) && (channel < 66) && (MDPP32_1_Trigger_time_diff_mul <=32)){
        // For the root branches
        MDPP32_1_Trigger_time_diff_Channel[MDPP32_1_Trigger_time_diff_mul] = channel-64;
        MDPP32_1_Trigger_time_diff_Value[MDPP32_1_Trigger_time_diff_mul] = value;
        MDPP32_1_Trigger_time_diff_mul++;
        //if (MDPP32_1_Trigger_time_diff_mul > 32-1) { MDPP32_1_Trigger_time_diff_mul = 32-1; }
      }
    } // != 0
  }
  
  if(mdpp32_scp_data_check == 0b0010){
    MDPP32_1_Extended_timestamp = (mdpp32_scp_data[0] + (mdpp32_scp_data[1] << 8) ) & 0b1111111111111111;
  }
  
  if (((mdpp32_scp_data[3] >> 6) & 0b11) == 3){ // Reading timestamp in the end of event
        MDPP32_1_Event_timestamp = (mdpp32_scp_data[0] + (mdpp32_scp_data[1] << 8) + (mdpp32_scp_data[2] << 16) + ((mdpp32_scp_data[3] & 0b00111111) << 24));

    }
}

// Method to filter make sure the frame is good (NOTE: I should have though of a better name, change for final version)
void read(ifstream *f, Int_t &broken_event_count) override {
  unsigned char block_read_header[4];
  f->read((char*) block_read_header, 4); // should be Type = 0xf5
  unsigned short module_event_length = (block_read_header[0] + (block_read_header[1] << 8) ) & 0b0001111111111111;
  int moduloID=(block_read_header[2])& 0b11111111;
  //cout << "MDPP32 Legnth" << module_event_length << endl;
  acarreo= module_event_length+1;
    
  if (module_event_length > 1) {
    ctr=0;
    for (int i=0; i < module_event_length; i++) { //module_event_length-1
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
                                                
    TString MultName = Form("MDPP32_SCP_%i_Multiplicity", d ); TString MultNameI = Form("MDPP32_SCP_%i_Multiplicity/I", d );// Generate a unique name for each histogram
    TString MultChan = Form("MDPP32_SCP_%i_Channels", d );     TString MultChanI = Form("MDPP32_SCP_%i_Channels[MDPP32_SCP_%i_Multiplicity]/I", d,d);
    TString MultValu = Form("MDPP32_SCP_%i_Values", d );       TString MultValuL = Form("MDPP32_SCP_%i_Values[MDPP32_SCP_%i_Multiplicity]/L", d,d );                                                                                                  
                                                                                    
    TString TDCtimeDiffMul = Form("MDPP32_SCP_%i_TDC_time_diff_mul", d );      TString TDCtimeDiffMulI = Form("MDPP32_SCP_%i_TDC_time_diff_mul/I", d );// Generate a unique name for each histogram
    TString TDCtimeDiffCha = Form("MDPP32_SCP_%i_TDC_time_diff_Channels", d ); TString TDCtimeDiffChaI = Form("MDPP32_SCP_%i_TDC_time_diff_Channels[MDPP32_SCP_%i_TDC_time_diff_mul]/I", d,d );
    TString TDCtimeDiffVal = Form("MDPP32_SCP_%i_TDC_time_diff_Values", d );   TString TDCtimeDiffValL = Form("MDPP32_SCP_%i_TDC_time_diff_Values[MDPP32_SCP_%i_TDC_time_diff_mul]/L", d,d );
    
    TString TriggertimeDiffMul = Form("MDPP32_SCP_%i_Trigger_time_diff_mul", d );       TString TriggertimeDiffMulI  = Form("MDPP32_SCP_%i_Trigger_time_diff_mul/I", d );// Generate a unique name for each histogram
    TString TriggertimeDiffCha = Form("MDPP32_SCP_%i_Trigger_time_diff_Channels", d );  TString TriggertimeDiffChaI  = Form("MDPP32_SCP_%i_Trigger_time_diff_Channels[MDPP32_SCP_%i_Trigger_time_diff_mul]/I", d,d );                                                    
    TString TriggertimeDiffVal = Form("MDPP32_SCP_%i_Trigger_time_diff_Values", d );    TString TriggertimeDiffValL  = Form("MDPP32_SCP_%i_Trigger_time_diff_Values[MDPP32_SCP_%i_Trigger_time_diff_mul]/L", d,d );
    
    TString ExtendedTimestamps = Form("MDPP32_SCP_%i_Extended_timestamps", d ); TString ExtendedTimestampsI  = Form("MDPP32_SCP_%i_Extended_timestamps/L", d );
    TString EventTimestamps    = Form("MDPP32_SCP_%i_Event_timestamps", d );    TString EventTimestampsL     = Form("MDPP32_SCP_%i_Event_timestamps/L", d );
    

    EventTree->Branch(MultName, &MDPP32_1_Multiplicity, MultNameI);
    EventTree->Branch(MultChan,  MDPP32_1_Channel,      MultChanI);
    EventTree->Branch(MultValu,  MDPP32_1_Value,        MultValuL);
                                                                                                      
    EventTree->Branch(TDCtimeDiffMul, &MDPP32_1_TDC_time_diff_mul,     TDCtimeDiffMulI);
    EventTree->Branch(TDCtimeDiffCha,  MDPP32_1_TDC_time_diff_Channel, TDCtimeDiffChaI);
    EventTree->Branch(TDCtimeDiffVal,  MDPP32_1_TDC_time_diff_Value,   TDCtimeDiffValL);
                                                                                                     
    EventTree->Branch(TriggertimeDiffMul, &MDPP32_1_Trigger_time_diff_mul,     TriggertimeDiffMulI);
    EventTree->Branch(TriggertimeDiffCha,  MDPP32_1_Trigger_time_diff_Channel, TriggertimeDiffChaI);
    EventTree->Branch(TriggertimeDiffVal,  MDPP32_1_Trigger_time_diff_Value,   TriggertimeDiffValL);
                                                                                                 
    EventTree->Branch(ExtendedTimestamps, &MDPP32_1_Extended_timestamp,  ExtendedTimestampsI);
    EventTree->Branch(EventTimestamps,    &MDPP32_1_Event_timestamp,     EventTimestampsL);
    
}  

// Sanity check histograms
void histoLOOP(TFile *treeFile,std::vector<TDirectory*>& refereciasFolders) override{
    int d=module;

    TDirectory *MDPP32_1_dir = refereciasFolders[stoi(event)]->mkdir(Form("%s_%i",nick.c_str(), d));//treeFile->mkdir(Form("MDPP32_SCP_%i", d));
    
    for (Int_t i=0; i<1024; i++) {
    if (i<32) {
      MDPP32_1_dir->cd();
      snprintf(MDPP32_1_name, sizeof(MDPP32_1_name), "MDPP32_SCP_%i_%i",d,i);
      snprintf(MDPP32_1_histo_name, sizeof(MDPP32_1_histo_name), "MDPP32_SCP_%i_%i ; Channel ; Counts",d,i);
      MDPP32_1_histo[i] = new TH1F(MDPP32_1_name, MDPP32_1_histo_name, 65536, 0, 65536);
      gDirectory->cd("..");
    }
  }
    gDirectory->cd("..");
}

};

