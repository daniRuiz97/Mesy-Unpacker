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
class MDPP16_SCP : public ModuleFather {
public:

int module;                                                           
string nick; 
string event;
///////////////////////// MDPP16 SCP ////////////////////////////
// this module have 32 channels but data follows this scheme:  //
//   ADC Value               - channels [0-15]                 //
//   TDC time difference     - channels [16-31]                //
//   Trigger time difference - channels [32(T0),33(T1)]        //
/////////////////////////////////////////////////////////////////

// Declare internal variables 
Int_t   MDPP16_scp_Multiplicity;
Int_t   MDPP16_scp_Channel[16];
ULong_t MDPP16_scp_Value[16];
Int_t   MDPP16_scp_TDC_time_diff_mul;
Int_t   MDPP16_scp_TDC_time_diff_Channel[16];
ULong_t MDPP16_scp_TDC_time_diff_Value[16];
Int_t   MDPP16_scp_Trigger_time_diff_mul;
Int_t   MDPP16_scp_Trigger_time_diff_Channel[16];
ULong_t MDPP16_scp_Trigger_time_diff_Value[16];
ULong_t MDPP16_scp_Extended_timestamp;
ULong_t MDPP16_scp_Event_timestamp;

TH1F *MDPP16_scp_histo[16];
char MDPP16_scp_name[32], MDPP16_scp_histo_name[32];


int ctr=0;
int contadorEntries=0;
// Constructor de MyClass
MDPP16_SCP (int indexModule, string Mote, string evet){
    module=indexModule;
    nick=Mote;
    event=evet;
}
// Initialize variables
void initEvent() override {
    MDPP16_scp_Multiplicity = 0;                  
    MDPP16_scp_TDC_time_diff_mul = 0;              
    MDPP16_scp_Trigger_time_diff_mul = 0;          
    MDPP16_scp_Extended_timestamp = 0;              
    MDPP16_scp_Event_timestamp= 0 ;                    

    for (int i=0; i<1024*3; i++) {

        if (i<16){                                               
            MDPP16_scp_Channel[i] = 0;                                    
            MDPP16_scp_Value[i] = 0;                                     
            MDPP16_scp_TDC_time_diff_Channel[i] = 0;    
            MDPP16_scp_TDC_time_diff_Value[i] = 0;        

        }
        if (i<16){
            MDPP16_scp_Trigger_time_diff_Channel[i] = 0;   
            MDPP16_scp_Trigger_time_diff_Value[i] = 0;     
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
            
      if ((channel >= 0) && (channel <= 15) && (MDPP16_scp_Multiplicity <=16)){
        // For the root branches
        MDPP16_scp_Channel[MDPP16_scp_Multiplicity] = channel;
        MDPP16_scp_Value[MDPP16_scp_Multiplicity] = value;
        
        MDPP16_scp_histo[channel] -> Fill(value);
        MDPP16_scp_Multiplicity++; 
        
        //if (MDPP16_scp_Multiplicity > 32-1) { MDPP16_scp_Multiplicity = 32-1; }
      }
      if ((channel >= 16) && (channel < 32) && (MDPP16_scp_TDC_time_diff_mul <=1632)){
        // For the root branches
        MDPP16_scp_TDC_time_diff_Channel[MDPP16_scp_TDC_time_diff_mul] = channel - 32;
        MDPP16_scp_TDC_time_diff_Value[MDPP16_scp_TDC_time_diff_mul] = value;
        MDPP16_scp_TDC_time_diff_mul++;
        //if (MDPP16_scp_TDC_time_diff_mul > 32-1) { MDPP16_scp_TDC_time_diff_mul = 32-1; }
      }
      if((channel >= 32) && (channel < 33) && (MDPP16_scp_Trigger_time_diff_mul <=16)){
        // For the root branches
        MDPP16_scp_Trigger_time_diff_Channel[MDPP16_scp_Trigger_time_diff_mul] = channel-32;
        MDPP16_scp_Trigger_time_diff_Value[MDPP16_scp_Trigger_time_diff_mul] = value;
        MDPP16_scp_Trigger_time_diff_mul++;
        //if (MDPP16_scp_Trigger_time_diff_mul > 32-1) { MDPP16_scp_Trigger_time_diff_mul = 32-1; }
      }
    } // != 0
  }
  
  if(mdpp32_scp_data_check == 0b0010){
    MDPP16_scp_Extended_timestamp = (mdpp32_scp_data[0] + (mdpp32_scp_data[1] << 8) ) & 0b1111111111111111;
  }
  
  if (((mdpp32_scp_data[3] >> 6) & 0b11) == 3){ // Reading timestamp in the end of event
        MDPP16_scp_Event_timestamp = (mdpp32_scp_data[0] + (mdpp32_scp_data[1] << 8) + (mdpp32_scp_data[2] << 16) + ((mdpp32_scp_data[3] & 0b00111111) << 24));

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
                                                
    TString MultName = Form("MDPP16_spc_%i_Multiplicity", d ); TString MultNameI = Form("MDPP16_spc_%i_Multiplicity/I", d );// Generate a unique name for each histogram
    TString MultChan = Form("MDPP16_spc_%i_Channels", d );     TString MultChanI = Form("MDPP16_spc_%i_Channels[MDPP16_spc_%i_Multiplicity]/I", d,d);
    TString MultValu = Form("MDPP16_spc_%i_Values", d );       TString MultValuL = Form("MDPP16_spc_%i_Values[MDPP16_spc_%i_Multiplicity]/L", d,d );                                                                                                  
                                                                                    
    TString TDCtimeDiffMul = Form("MDPP16_spc_%i_TDC_time_diff_mul", d );      TString TDCtimeDiffMulI = Form("MDPP16_spc_%i_TDC_time_diff_mul/I", d );// Generate a unique name for each histogram
    TString TDCtimeDiffCha = Form("MDPP16_spc_%i_TDC_time_diff_Channels", d ); TString TDCtimeDiffChaI = Form("MDPP16_spc_%i_TDC_time_diff_Channels[MDPP16_spc_%i_TDC_time_diff_mul]/I", d,d );
    TString TDCtimeDiffVal = Form("MDPP16_spc_%i_TDC_time_diff_Values", d );   TString TDCtimeDiffValL = Form("MDPP16_spc_%i_TDC_time_diff_Values[MDPP16_spc_%i_TDC_time_diff_mul]/L", d,d );
    
    TString TriggertimeDiffMul = Form("MDPP16_spc_%i_Trigger_time_diff_mul", d );       TString TriggertimeDiffMulI  = Form("MDPP16_spc_%i_Trigger_time_diff_mul/I", d );// Generate a unique name for each histogram
    TString TriggertimeDiffCha = Form("MDPP16_spc_%i_Trigger_time_diff_Channels", d );  TString TriggertimeDiffChaI  = Form("MDPP16_spc_%i_Trigger_time_diff_Channels[MDPP16_spc_%i_Trigger_time_diff_mul]/I", d,d );                                                    
    TString TriggertimeDiffVal = Form("MDPP16_spc_%i_Trigger_time_diff_Values", d );    TString TriggertimeDiffValL  = Form("MDPP16_spc_%i_Trigger_time_diff_Values[MDPP16_spc_%i_Trigger_time_diff_mul]/L", d,d );
    
    TString ExtendedTimestamps = Form("MDPP16_spc_%i_Extended_timestamps", d ); TString ExtendedTimestampsI  = Form("MDPP16_spc_%i_Extended_timestamps/L", d );
    TString EventTimestamps    = Form("MDPP16_spc_%i_Event_timestamps", d );    TString EventTimestampsL     = Form("MDPP16_spc_%i_Event_timestamps/L", d );
    

    EventTree->Branch(MultName, &MDPP16_scp_Multiplicity, MultNameI);
    EventTree->Branch(MultChan,  MDPP16_scp_Channel,      MultChanI);
    EventTree->Branch(MultValu,  MDPP16_scp_Value,        MultValuL);
                                                                                                      
    EventTree->Branch(TDCtimeDiffMul, &MDPP16_scp_TDC_time_diff_mul,     TDCtimeDiffMulI);
    EventTree->Branch(TDCtimeDiffCha,  MDPP16_scp_TDC_time_diff_Channel, TDCtimeDiffChaI);
    EventTree->Branch(TDCtimeDiffVal,  MDPP16_scp_TDC_time_diff_Value,   TDCtimeDiffValL);
                                                                                                     
    EventTree->Branch(TriggertimeDiffMul, &MDPP16_scp_Trigger_time_diff_mul,     TriggertimeDiffMulI);
    EventTree->Branch(TriggertimeDiffCha,  MDPP16_scp_Trigger_time_diff_Channel, TriggertimeDiffChaI);
    EventTree->Branch(TriggertimeDiffVal,  MDPP16_scp_Trigger_time_diff_Value,   TriggertimeDiffValL);
                                                                                                 
    EventTree->Branch(ExtendedTimestamps, &MDPP16_scp_Extended_timestamp,  ExtendedTimestampsI);
    EventTree->Branch(EventTimestamps,    &MDPP16_scp_Event_timestamp,     EventTimestampsL);
    
}  

// Sanity check histograms
void histoLOOP(TFile *treeFile,std::vector<TDirectory*>& refereciasFolders) override{
    int d=module;

    TDirectory *MDPP16_scp_dir = refereciasFolders[stoi(event)]->mkdir(Form("%s_%i",nick.c_str(), d));//treeFile->mkdir(Form("MDPP16_spc_%i", d));
    
    for (Int_t i=0; i<1024; i++) {
    if (i<16) {
      MDPP16_scp_dir->cd();
      snprintf(MDPP16_scp_name, sizeof(MDPP16_scp_name), "MDPP16_%i_%i",d,i);
      snprintf(MDPP16_scp_histo_name, sizeof(MDPP16_scp_histo_name), "MDPP16_%i_%i ; Channel ; Counts",d,i);
      MDPP16_scp_histo[i] = new TH1F(MDPP16_scp_name, MDPP16_scp_histo_name, 65536, 0, 65536);
      gDirectory->cd("..");
    }
  }
  gDirectory->cd("..");
}

/**/

};

