/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Daughter class to translate data from a MDPP16_qdc module:                                                                          //
//                                                                                                                                 //
// Inherits structure from the ModuleFather.h                                                                                      //
//  Internal methods designed for the MDPP16 modules (info:https://www.mesytec.com/products/datasheets/MDPP-16_SCP-RCP.pdf page 10) //                                                                      
// The methods need to be summoned in the CORRECT OREDER, the Unpacker takes care of this thanks to the config file                //
// The output is:                                                                                                                  //
//    1-> Root branches containing relevant info that will be sitched to a TTree                                                    //
//    2-> Histograms done with the info of the branches, used to compare with Mesytec as a sanity check                           //
// DFR                                                                                                                             //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "../include/ModuleFather.h"
#include <iostream>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TDirectory.h"
#include "TString.h"


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
class MDPP16_QDC : public ModuleFather {
public:

int module;                                                           
string nick; 
string event;

// Declare internal variable
Int_t   MDPP16_1_ilong_Multiplicity;
Int_t   MDPP16_1_ilong_Channel[16*3];
ULong_t MDPP16_1_ilong_Value[16*3];
Int_t   MDPP16_1_ishort_Multiplicity;
Int_t   MDPP16_1_ishort_Channel[16*3];
ULong_t MDPP16_1_ishort_Value[16*3];
Int_t   MDPP16_1_TDC_time_diff_mul;
Int_t   MDPP16_1_TDC_time_diff_Channel[16*3];
ULong_t MDPP16_1_TDC_time_diff_Value[16*3];
Int_t   MDPP16_1_Trigger_time_diff_mul;
Int_t   MDPP16_1_Trigger_time_diff_Channel[16];
ULong_t MDPP16_1_Trigger_time_diff_Value[16];
ULong_t MDPP16_1_Extended_timestamp;
ULong_t MDPP16_1_Event_timestamp;

int contadorChannel0 =0;
int longitudEventoAnterior =0;
int overCont =0;
int saltador = 0;
unsigned int amplitud=0;

TH1F *MDPP16_1_ilong_histo[16], *MDPP16_1_ishort_histo[16];
//TH1D *MDPP16_1_ilong_histo[16], *MDPP16_1_ishort_histo[16];
char MDPP16_1_ilong_name[64], MDPP16_1_ilong_histo_name[64];
char MDPP16_1_ishort_name[64], MDPP16_1_ishort_histo_name[64];

std::vector<TDirectory*> MDPP16_1_ilong_dirs;
std::vector<TDirectory*> MDPP16_1_ishort_dirs;

MDPP16_QDC(int indexModule, string Mote, string evet){
    module=indexModule;
    nick=Mote;
    event=evet;
}

// Initialize variables
void initEvent() override {
  MDPP16_1_ilong_Multiplicity = 0;   
    MDPP16_1_ishort_Multiplicity = 0;       
    MDPP16_1_TDC_time_diff_mul = 0;         
    MDPP16_1_Trigger_time_diff_mul = 0;     
    MDPP16_1_Extended_timestamp = 0;        
    MDPP16_1_Event_timestamp = 0;           
    for (int i=0; i<1024*3; i++) {
        if (i<16*3){
            MDPP16_1_ilong_Channel[i] = 0;                 
            MDPP16_1_ilong_Value[i] = 0;                
            MDPP16_1_ishort_Channel[i] = 0;                
            MDPP16_1_ishort_Value[i] = 0;               
            MDPP16_1_TDC_time_diff_Channel[i] = 0;  
            MDPP16_1_TDC_time_diff_Value[i] = 0;        
        }
        if (i<16){                             
            MDPP16_1_Trigger_time_diff_Channel[i] = 0;  
            MDPP16_1_Trigger_time_diff_Value[i] = 0;    
        }
    }
}

// Method to transcribe Data
// For a frame that has been validadted, this method transcribes relevant parameteres from hex to numerical
void readData(ifstream *f) override {
unsigned char mdpp16_qdc_data[4];
  f->read((char*) mdpp16_qdc_data, 4);
  
  unsigned char mdpp16_qdc_data_check = (mdpp16_qdc_data[3] >> 4) & 0b1111;

  if (mdpp16_qdc_data_check == 0b0001) {

    if (((mdpp16_qdc_data[0] + (mdpp16_qdc_data[1] << 8) + (mdpp16_qdc_data[2] << 16) + (mdpp16_qdc_data[3] << 24)) != 0)) {
            
      unsigned int channel = mdpp16_qdc_data[2] & 0b00111111;
      unsigned long value = ( mdpp16_qdc_data[0] + (mdpp16_qdc_data[1] << 8) ) & 0b1111111111111111;
      unsigned int overflow = (mdpp16_qdc_data[2] >> 6) & 0b01;
    
      //cout<<"canal " << channel<< " ,valor: "<< value<<endl;
      //if (triggerChannel==0){
       // saltador+=1;
        //if(saltador%1000==0){
          //cout<<saltador<<" "<<channel<<endl;
       // }
      //}

    if ( (channel >= 0) && (channel < 16)){
      if(overflow==1){
        overCont+=value;
        //cout<<overCont<<endl;
    }
        MDPP16_1_ilong_Channel[MDPP16_1_ilong_Multiplicity] = channel;
        
        MDPP16_1_ilong_Value[MDPP16_1_ilong_Multiplicity] = value;
        MDPP16_1_ilong_Multiplicity++;
       
        //MDPP16_1_ilong_histo[channel] -> Fill(value);
        

        if (channel ==0){
          contadorChannel0+=1;
          MDPP16_1_ilong_histo[channel] -> Fill(value);
        }
        else{
          MDPP16_1_ilong_histo[channel] -> Fill(value);
        }
        
        if (MDPP16_1_ilong_Multiplicity > 16*3-1) { MDPP16_1_ilong_Multiplicity = 16*3-1; }
      }
      /**/
      if ((channel >= 16) && (channel < 32)){
        MDPP16_1_TDC_time_diff_Channel[MDPP16_1_TDC_time_diff_mul] = channel - 16;
        MDPP16_1_TDC_time_diff_Value[MDPP16_1_TDC_time_diff_mul] = value;
        MDPP16_1_TDC_time_diff_mul++;
        if(MDPP16_1_TDC_time_diff_mul > 16*3-1){MDPP16_1_TDC_time_diff_mul = 16*3-1;}
      }

      if( (channel >= 32) && (channel < 34)){
        MDPP16_1_Trigger_time_diff_Channel[MDPP16_1_Trigger_time_diff_mul] = channel-32;
        MDPP16_1_Trigger_time_diff_Value[MDPP16_1_Trigger_time_diff_mul] = value;
        MDPP16_1_Trigger_time_diff_mul++;
        //cout<<"Time stamp "<< value<< endl;
        if(MDPP16_1_Trigger_time_diff_mul > 16-1){MDPP16_1_Trigger_time_diff_mul = 16-1;}
      }
    
            
     if ((channel >= 48) && (channel < 64)){
        MDPP16_1_ishort_Channel[MDPP16_1_ishort_Multiplicity] = channel-48;
        
        MDPP16_1_ishort_Value[MDPP16_1_ishort_Multiplicity] = value;
        MDPP16_1_ishort_Multiplicity++;
        MDPP16_1_ishort_histo[channel-48] -> Fill(value);
        if (MDPP16_1_ishort_Multiplicity > 16*3-1) { MDPP16_1_ishort_Multiplicity = 16*3-1; }
      }
      
    }

    }else if(mdpp16_qdc_data_check == 0b0100){
      unsigned int moduloID = (mdpp16_qdc_data[2]) & 0b11111111;
      amplitud = (mdpp16_qdc_data[0] + (mdpp16_qdc_data[1] << 8) ) & 0b0000001111111111;
      //cout<<"Longitud  "<< amplitud<<endl;

    }

      if(mdpp16_qdc_data_check == 0b0010){// AQUI NUNCA ENTRA

        MDPP16_1_Extended_timestamp = (mdpp16_qdc_data[0] + (mdpp16_qdc_data[1] << 8) ) & 0b1111111111111111;
   
      }
  
  if (((mdpp16_qdc_data[3] >> 6) & 0b11) == 3){// Reading timestamp in the end of event AQUI SI LLEGA A ENTRAR
     MDPP16_1_Event_timestamp = (mdpp16_qdc_data[0] + (mdpp16_qdc_data[1] << 8) + (mdpp16_qdc_data[2] << 16) + ((mdpp16_qdc_data[3] & 0b00111111) << 24));
     //cout<<"Event counter/time stamp "<<MDPP16_1_Event_timestamp <<endl;
   
    }

}
// Method to filter make sure the frame is good (NOTE: I should have though of a better name, change for final version)
void read(ifstream *f, Int_t &broken_event_count) override {
  unsigned char block_read_header[4];
  f->read((char*) block_read_header, 4); // shoulds be Type = 0xf5
  unsigned short module_event_length = (block_read_header[0] + (block_read_header[1] << 8) ) & 0b0001111111111111;


  acarreo= module_event_length+1;
  if (module_event_length > 1) {
    unsigned char module_header[4];

    //f->read((char*) module_header, 4);
    //unsigned short module_header_length = (module_header[0] + (module_header[1] << 8) ) & 0b0000001111111111;
    //cout << "Longitud F5:  " << module_event_length<<" ";
    //cout << "Longitug Header " << module_header_length <<endl;
    //int longitud = (module_header[0] + (module_header[1] << 8) ) & 0b0000001111111111;
    //unsigned int moduloID = (module_header[2]) & 0b11111111;
    //if (moduloID!=2){
   //cout << "moduloID: " << moduloID <<endl;
  //}
  //cout<<"--------------"<<endl;
    for (int i=0; i < module_event_length; i++) {
    //  for (int i=0; i <module_header_length; i++) {
      //  if(longitud!=longitudEventoAnterior){
          readData(f);
       // }else{
       //   f->read((char*) block_read_header, 4);
       // }
    
  }
    //longitudEventoAnterior = longitud;
  }else if (module_event_length == 1) {
  //else {//if (module_event_length == 1) {//AQUI JAMAS ESTA ENTRANDO
      unsigned char dontCare[4];
      f->read((char*) dontCare, 4);
      broken_event_count++;
    

      std::cout << "SALIO" << std::endl;
    }
}

 void write(Int_t &broken_event_count) override {
}

// Create branches for the Ttree
void createTree(TTree *EventTree) override{
    // MDPP16_QDC 1 
    int d=module;
                                                                   
    TString LongMultName       = Form("MDPP16_QDC_%i_ilong_Multiplicity", d );           
    TString LongMultNameI         = Form("MDPP16_QDC_%i_ilong_Multiplicity/I", d );// Generate a unique name for each histogram
    
    TString LongMultChan       = Form("MDPP16_QDC_%i_ilong_Channels", d );               
    TString LongMultChanI         = Form("MDPP16_QDC_%i_ilong_Channels[MDPP16_QDC_%i_ilong_Multiplicity]/I", d,d);
    
    TString LongMultValu       = Form("MDPP16_QDC_%i_ilong_Values", d );                 
    TString LongMultValuL         = Form("MDPP16_QDC_%i_ilong_Values[MDPP16_QDC_%i_ilong_Multiplicity]/L", d,d );
                                                                                                             
    TString ShortMultName       = Form("MDPP16_QDC_%i_ishort_Multiplicity", d );         
    TString ShortMultNameI        = Form("MDPP16_QDC_%i_ishort_Multiplicity/I", d );// Generate a unique name for each histogram
    
    TString ShortMultChan       = Form("MDPP16_QDC_%i_ishort_Channels", d );
    TString ShortMultChanI        = Form("MDPP16_QDC_%i_ishort_Channels[MDPP16_QDC_%i_ishort_Multiplicity]/I", d,d );
    
    TString ShortMultValu       = Form("MDPP16_QDC_%i_ishort_Values", d );
    TString ShortMultValuL        = Form("MDPP16_QDC_%i_ishort_Values[MDPP16_QDC_%i_ishort_Multiplicity]/L", d,d );
                                                                                    
    TString TDCtimeDiffMul      = Form("MDPP16_QDC_%i_TDC_time_diff_mul", d );
    TString TDCtimeDiffMulI       = Form("MDPP16_QDC_%i_TDC_time_diff_mul/I", d );// Generate a unique name for each histogram
    
    TString TDCtimeDiffCha      = Form("MDPP16_QDC_%i_TDC_time_diff_Channels", d );
    TString TDCtimeDiffChaI       = Form("MDPP16_QDC_%i_TDC_time_diff_Channels[MDPP16_QDC_%i_TDC_time_diff_mul]/I", d,d );
    
    TString TDCtimeDiffVal      = Form("MDPP16_QDC_%i_TDC_time_diff_Values", d );
    TString TDCtimeDiffValL       = Form("MDPP16_QDC_%i_TDC_time_diff_Values[MDPP16_QDC_%i_TDC_time_diff_mul]/L", d,d );
    
    TString TriggertimeDiffMul  = Form("MDPP16_QDC_%i_Trigger_time_diff_mul", d );
    TString TriggertimeDiffMulI  = Form("MDPP16_QDC_%i_Trigger_time_diff_mul/I", d );// Generate a unique name for each histogram
    
    TString TriggertimeDiffCha  = Form("MDPP16_QDC_%i_Trigger_time_diff_Channels", d );
    TString TriggertimeDiffChaI  = Form("MDPP16_QDC_%i_Trigger_time_diff_Channels[MDPP16_QDC_%i_Trigger_time_diff_mul]/I", d,d );                                                    
    
    TString TriggertimeDiffVal  = Form("MDPP16_QDC_%i_Trigger_time_diff_Values", d );
    TString TriggertimeDiffValL  = Form("MDPP16_QDC_%i_Trigger_time_diff_Values[MDPP16_QDC_%i_Trigger_time_diff_mul]/L", d,d );
    
    TString ExtendedTimestamps  = Form("MDPP16_QDC_%i_Extended_timestamps", d );          
    TString ExtendedTimestampsI  = Form("MDPP16_QDC_%i_Extended_timestamps/L", d );
    
    TString EventTimestamps     = Form("MDPP16_QDC_%i_Event_timestamps", d );
    TString EventTimestampsL     = Form("MDPP16_QDC_%i_Event_timestamps/L", d );
    

    EventTree->Branch(LongMultName,        &MDPP16_1_ilong_Multiplicity,         LongMultNameI);
    EventTree->Branch(LongMultChan,         MDPP16_1_ilong_Channel,              LongMultChanI);
    EventTree->Branch(LongMultValu,         MDPP16_1_ilong_Value,                LongMultValuL);
    
    EventTree->Branch(ShortMultName,        &MDPP16_1_ishort_Multiplicity,       ShortMultNameI);
    EventTree->Branch(ShortMultChan,         MDPP16_1_ishort_Channel,            ShortMultChanI);
    EventTree->Branch(ShortMultValu,         MDPP16_1_ishort_Value,              ShortMultValuL);
                                                                                                      
    EventTree->Branch(TDCtimeDiffMul,      &MDPP16_1_TDC_time_diff_mul,          TDCtimeDiffMulI);
    EventTree->Branch(TDCtimeDiffCha,       MDPP16_1_TDC_time_diff_Channel,      TDCtimeDiffChaI);
    EventTree->Branch(TDCtimeDiffVal,       MDPP16_1_TDC_time_diff_Value,        TDCtimeDiffValL);
                                                                                                     
    EventTree->Branch(TriggertimeDiffMul,  &MDPP16_1_Trigger_time_diff_mul,      TriggertimeDiffMulI);
    EventTree->Branch(TriggertimeDiffCha,   MDPP16_1_Trigger_time_diff_Channel,  TriggertimeDiffChaI);
    EventTree->Branch(TriggertimeDiffVal,   MDPP16_1_Trigger_time_diff_Value,    TriggertimeDiffValL);
                                                                                                     
    EventTree->Branch(ExtendedTimestamps,  &MDPP16_1_Extended_timestamp,         ExtendedTimestampsI);
    EventTree->Branch(EventTimestamps,     &MDPP16_1_Event_timestamp,            EventTimestampsL);
} 

// Sanity check histograms
void histoLOOP(TFile *treeFile,std::vector<TDirectory*>& refereciasFolders) override{

    int d=module;

    TDirectory *MDPP16_1_ilong_dir  = refereciasFolders[stoi(event)]->mkdir(Form("%s_%i_ilong", nick.c_str(),d));//treeFile->mkdir(Form("MDPP16_QDC_%i_ilong", d));
    TDirectory *MDPP16_1_ishort_dir = refereciasFolders[stoi(event)]->mkdir(Form("%s_%i_ishort",nick.c_str() ,d));//treeFile->mkdir(Form("MDPP16_QDC_%i_ishort", d));
    
    
    for (Int_t i=0; i<1024; i++) {

    if (i<16) {
        MDPP16_1_ilong_dir->cd();
        snprintf(MDPP16_1_ilong_name, sizeof(MDPP16_1_ilong_name), "MDPP16_QDC_%i_ilong_%i",d,i);
        snprintf(MDPP16_1_ilong_histo_name, sizeof(MDPP16_1_ilong_histo_name), "MDPP16_QDC_%i_ilong_%i ; Channel ; Counts",d,i);
        MDPP16_1_ilong_histo[i] = new TH1F(MDPP16_1_ilong_name, MDPP16_1_ilong_histo_name, 4096, 0, 65536);
        gDirectory->cd("..");
    
       
        MDPP16_1_ishort_dir->cd();
        snprintf(MDPP16_1_ishort_name, sizeof(MDPP16_1_ishort_name), "MDPP16_QDC_%i_ishort_%i",d,i);
        snprintf(MDPP16_1_ishort_histo_name, sizeof(MDPP16_1_ishort_histo_name), "MDPP16_QDC_%i_ishort_%i ; Channel ; Counts",d,i);
        MDPP16_1_ishort_histo[i] = new TH1F(MDPP16_1_ishort_name, MDPP16_1_ishort_histo_name, 4096, 0, 65536);
        gDirectory->cd("..");
        }
    }

    gDirectory->cd("..");
    
   }
};

