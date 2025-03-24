#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <stdio.h>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"

#include <string.h>
#include <fstream>

#include <sstream>
#include <cstdio>

#include "include/ModuleFather.h"
#include "src/MDPP16_qdc.cpp"
#include "src/MDPP16_scp.cpp"
#include "src/MDPP32.cpp"
#include "src/VMMR8.cpp"
#include "src/Caen785.cpp"
#include "src/Caen785N.cpp"
#include "src/MADC32.cpp"
#include "src/mvlc_event_stamper.cpp"
#include "src/mvlc_ts.cpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

using std::ifstream;
using namespace std;

char discard[4 * 65536];

int counterEvent0=0;
int counterEvent1=0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The purpose of this code is to unpack a Mesytec list file and present the data in a TTree of ROOT.               //
// The code requires a configuration file that specifies the number of events and the modules associated with each   //
// event.                                                                                                            //
//                                                                                                                   //
// Data in Mesytec files is stored in listfile mode: a collection of hex lines grouped in frames. Each frame starts //
// with a header (FX) that summarizes the information of that frame (frames can contain sub-frames). The code reads  //
// the file frame by frame (F3, F5, and F9 frames are the ones that contain data).                                   //
//                                                                                                                   //
// The configuration file is used to create a TTree to fill and determine the order in which modules store           //
// information.                                                                                                      //
//                                                                                                                   //
// To unpack the data, N objects of the virtual class ModulesFather are created. The virtual class contains the      //
// necessary method hierarchy to read the modules. These classes are inherited by individual daughter classes used   //
// to unpack the frames of each specific module. Daughter objects are initialized from the father class.             //
//                                                                                                                   //
// More info on the listfile structure can be found in:                                                              //
// https://github.com/flueke/mesytec-mvlc/blob/22d88cdd9cea341d6bdb5b455c5681ea2a14ad19/src/mesytec-mvlc/mvlc_constants.h#L140 //
// DFR                                                                                                               //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Code structure:                                                                                                      //
//  Part I: Classes used to read the file (specific unpacking routines for each module can be found in the src folder). //
//  Part II: The main read routine that initializes the objects and calls the reading methods described in Part I.      //
//                                                                                                                      //
// Code inputs:                                                                                                         //
//  Configuration file: gives the structure of the file                                                                 //
//  File to Unpack: this file will be transformed to Root                                                               //
//                                                                                                                      //
//Code output:                                                                                                          //
//   Root file                                                                                                          //
//                                                                                                                      //
//To compile:                                                                                                           //
// g++ Unpack_mvme.cpp `root-config --cflags --libs` -o Unpack_mvme.out                                                 //
//                                                                                                                      //
//To execute:                                                                                                           //
// ./Unpack_mvme.out                                                                                                    //
//                                                                                                                      //
//Compilation and excution automatized in Run_Unpack.sh                                                                 //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//   Part I: Declaration of methods   //
////////////////////////////////////////


//Function to read a txt file and return a vector with its content
vector<string> readTxt(ifstream *file){

    if (!file->is_open()) {
        std::cerr << "Error opening file." << std::endl;
        exit(0);
    }

    std::vector<std::string> file_names;
    std::string line;

    // Read file names into a vector
    while (std::getline(*file, line)) {
        file_names.push_back(line);
    }
    file->close();

    return file_names;

}

// Function to create the TDirectorys for all detected events-------------------------------------------------------------------------------------------------------
// It is necessary to save them in a list and return the dirs so that data form each detector can later be placed inside an appropriate folder. 
// Position 0 corresponds to the folder of Event 0, position 1 to Event 1, and so on...
// Since each module knows which event it is associated with, it knows which position in the vector to look at to find the folder where it should be placed
// (all this is done in the histoLoop function)

std::vector<TDirectory*> createEventsFolder(TFile *treeFile,int numEvents){
    std::vector<TDirectory*> eventsFolders;

    for (int i=0;i<numEvents;i++){
        eventsFolders.push_back( treeFile->mkdir(Form("Event_%i", i)));
    }

    return eventsFolders;

}

// Function to jump a useless frame----------------------------------------------------------------------------------------------------------------------------------
// As explained not all frames contain useful info, threfore we must have a function to ignore ("ie jump") frames.
// The header of a frame contains information of its length [ie how many bytes are bteween the First byte (FX) of the frame and the next] 
// This information in the final 13 bits of the headers.
// This function gets the length of a frmae and moves the next frame
void jumpFrame(ifstream *f, unsigned char frame_header[4],ULong_t &bytes_counter) 
{
  // Extract 13 bit number form the last two Bytes (the code is read in little endiang)  
  int words_length = (frame_header[0] + (frame_header[1] << 8) ) & 0b0001111111111111;
  //cout << "Words to jump " << words_length << endl;
  if (words_length != 0) {
    f->read(discard, words_length * 4);
  }
  bytes_counter += (words_length * 4);
}

// Function to decrypt a frame----------------------------------------------------------------------------------------------------------------------------------
// This function identifies if the frame is good or not (F3,F5 or F9).
// If it is not we call the jump frame method
// If it is good we unpack it.
// Thanks to the config file the code knows which methods to call

void readFrame(ifstream *f,ULong_t &bytes_counter, Int_t &DATAFRAME_marker, Int_t &EOF_MARKER,ULong_t &totalEvents,Int_t &continue_frame_counter,TTree *EventTree, TFile *treeFile,vector<ModuleFather*>& array,Int_t &broken_event_count) 
{
    // Get header from the frame
    unsigned char header[4];
    f->read((char*) header, 4);
    unsigned char frameType = header[3];

    bytes_counter += 4;
    DATAFRAME_marker = 0;  

    
    if(frameType == 0xF9){
        cout <<  "CONTINUATION FRAME DETECTED"<<endl;

    }
    if (frameType == 0xFA) { // FA frames have information that has already been read by Interpreter.py so they are irrelevant
        
        unsigned int frame_subtype;
        frame_subtype = (header[1] + (header[2] << 8)) >> 5 & 0b1111111;

        // Those frames don't have relevant info, but EOF_MARKER, so we jump it always.
        if (frame_subtype == 0x77) { // End of frame (EOF_MARKER) subtype
            cout <<  "EOF_MARKER " << header << endl;
            EOF_MARKER = 1;
        } 
        // Jump frame
        else {
            jumpFrame(f, header,bytes_counter);
        }
    }

    // The 0xF3 frame marks the start of an event, so keep reading. 
    // Always followed by 0xF5 frame. The F5 contains the info being sent by a module
    // Sometimes if data contained in this frame are too large, 0xF3 frames split in some more, so 
    // we have to check the Continue bit. If Continue bit is set, the following frames will be 
    // StackContinuation Frames (0xF9). The last frame in the series will have Continue bit cleared.
    else if (frameType == 0xF3) {
       
        // Get frame
        
        unsigned char lower_4_bits = header[2] & 0b00001111;
        if(lower_4_bits==1){
            DATAFRAME_marker = 1;
            counterEvent0++;

             // Move to the next frame
            int frame_F3_length = (header[0] + (header[1] << 8) ) & 0b0001111111111111;
        
            bytes_counter += frame_F3_length * 4; // Move to the next frame

            int auxiliar=0;
            
            // Every F3 contains the F5 of the modules that are sending info.
            // The order in which the modules send information (ie the order of the F5) is always the same (set by user when setting UP the DAQ)
            // The order of the modules is extracted from the FA frames by interpreter.py
            // Using the config file we have created a an array that aplies the reading function in the correct order. 
            for (Int_t i = 0; i < array.size(); i++) {
                // Call the Read method from the array (the read method is module-specific)
                array[i]->initEvent();
                if(array[i]->implemented){
                    array[i]->read(f,broken_event_count);
                    auxiliar=array[i]->acarreo+auxiliar;
                
                }else{
                    /*
                    cout << "Hola" <<endl;
                    unsigned char block_read_header[4];
                    f->read((char*) block_read_header, 4);
                    jumpFrame(f, block_read_header,bytes_counter);
                    */
                    int salto=frame_F3_length-auxiliar;
                    f->read(discard, salto * 4);
                }

                if (totalEvents % 1000000 == 0) {
                    cout << " Events counted: " << totalEvents/1e6 << " millions" << endl;
                }
        
                if ((header[2] & 0b10000000) == 1){//Check if there is an continue frame
                    cout << "continue frame" << endl;
                    continue_frame_counter += 1;
                }

                totalEvents++;
            }
        }
        else{
            jumpFrame(f, header,bytes_counter);
            counterEvent1++;
        }   
    }
    
    else if (!frameType){
      //cout <<(int)  frameType << endl;
      cout << "-- No EOF found so stop reading the file--" << endl;
       
      EOF_MARKER = 1;
    }

    
    else {
        cout << " ERROR: unknown frame type: '" << (int) frameType << "'" << endl;
        jumpFrame(f, header,bytes_counter);
    }
}
//////////////////////////////////////////////////////////////////////////////
//   Part II: Main part, read the config file, and call appropriate methods   //
//////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

    // Extract command-line arguments
    string map_file = argv[1];
    string mvlclstFile = argv[2];
    string output_file = argv[3];
    
    // Variables to read the binary
    Int_t broken_event_count=0;
    Int_t DATAFRAME_marker = 0;
    Int_t EOF_MARKER=0;
    Int_t continue_frame_counter = 0;
    ULong_t bytes_counter = 0;
    ULong_t totalEvents = 0;

    ///////////////////////////////////////////////////////////////////////////////
    // Use the module list to initialise objects of the appropriate daughter class
    ///////////////////////////////////////////////////////////////////////////////
    
    std::ifstream modules_events_structure(map_file);
    std::vector<std::string> module_events_vector=readTxt(&modules_events_structure);
    const int file_size = module_events_vector.size();//Creo que se puede borrar, ya no se usa

    // Vector de la clase padre abstracta de los módulos, nos permitará aplicar polimorfia
    // Vector of the N-modules length of instances of the virtual class module fahter that contains the method hiereachy
    // The specific method are initlalized from the daugheter clases using the config file
    std::vector<ModuleFather*> arrayTest;

    bool found = false; //Creo que se puede borrar
    
    // Counter for the numbe of modules of each type, use to build folders, histogrmas and distiughs modules of same type
    int counter_MDPP16_QDC=0;
    int counter_MDPP16_SCP=0;
    int counter_MDPP32=0;
    int counter_MADC32=0;
    int counter_VMMR8=0;
    int counter_CAEN789=0;
    int counter_CAEN789N=0;
    int counter_mvlc_event_stamper=0;
    int counter_mvlc_ts=0;
    int eventCounter=0;

    /*for(size_t i = 0; i < module_events_vector.size(); i++) {
        cout << module_events_vector[i] << endl;
    }
    cout <<"---------------" << endl;*/

    //Read the config file and for each module initialise a object of the appropriate class in the correct position
    for (size_t i = 0; i < module_events_vector.size(); i+=3) {
    
       
        if(eventCounter==stoi(module_events_vector[i])){ //La  stoi permite converts string to int
            eventCounter++;
        }

        if (module_events_vector[i+2] == "mdpp16_qdc") {

            arrayTest.push_back(new MDPP16_QDC(counter_MDPP16_QDC,module_events_vector[i+1],module_events_vector[i]));
            counter_MDPP16_QDC=counter_MDPP16_QDC+1;
            cout << " mdpp16_qdc" << endl;
        }
        else if (module_events_vector[i+2] == "mdpp16_scp") {

            arrayTest.push_back(new MDPP16_SCP(counter_MDPP16_SCP,module_events_vector[i+1],module_events_vector[i]));
            counter_MDPP16_SCP=counter_MDPP16_SCP+1;
            cout << " mdpp16_scp" << endl;
        }
        else if(module_events_vector[i+2] == "mdpp32_scp"){
          
            arrayTest.push_back(new MDPP32(counter_MDPP32,module_events_vector[i+1],module_events_vector[i]));
            counter_MDPP32=counter_MDPP32+1;
            cout << " mpp32_scp" << endl;
        }
        else if(module_events_vector[i+2] == "madc32"){
            
           arrayTest.push_back(new MADC32(counter_MADC32,module_events_vector[i+1],module_events_vector[i]));
            counter_MADC32=counter_MADC32+1;
            cout << "madc" << endl;
        }
        else if(module_events_vector[i+2] == "vmmr"){

            arrayTest.push_back(new VMMR8(counter_VMMR8,module_events_vector[i+1],module_events_vector[i]));
            counter_VMMR8=counter_VMMR8+1;
        }
        else if(module_events_vector[i+2] == "caen_v785"){

            arrayTest.push_back(new Caen785(counter_CAEN789,module_events_vector[i+1],module_events_vector[i]));
            counter_CAEN789=counter_CAEN789+1;
            cout << "caen_v785" << endl;
        }
        else if(module_events_vector[i+2] == "caen_v785N"){

            arrayTest.push_back(new CAEN785N(counter_CAEN789,module_events_vector[i+1],module_events_vector[i]));
            counter_CAEN789N=counter_CAEN789N+1;
             cout << "caen_v785N" << endl;
        }
        else if(module_events_vector[i+2] == "mvlc_event_stamper"){
            arrayTest.push_back(new mvlc_event_stamper());
            counter_mvlc_event_stamper=counter_mvlc_event_stamper+1;
            cout << "mvlc_event_stamper" << endl;

        }
        else if(module_events_vector[i+2] == "mvlc_ts"){
            arrayTest.push_back(new mvlc_ts());
            counter_mvlc_ts=counter_mvlc_ts+1;
            cout << "mvlc_ts" << endl;
        }

        else{
            std::cout <<"Unregistered Module:"<< module_events_vector[i+2]  << std::endl;
            std::cout <<i << std::endl;
        }
    }
    

    ifstream fp;
    
    // Create Root Tree and File
    TTree *EventTreeU; 
    TFile *treeFileU;
    EventTreeU = new TTree("EventTree", "EventTree");
    treeFileU = new TFile(argv[3], "recreate"); 

   // Creamos Foders
   std::vector<TDirectory*> refereciasFolders=createEventsFolder(treeFileU,eventCounter); 
   
    
    // Create the branches and histos for each module  in the correct order by calling the specific method through the appropriate object
    for (size_t i = 0; i < arrayTest.size(); i++) {
        arrayTest[i]->createTree(EventTreeU);
        arrayTest[i]->histoLOOP(treeFileU,refereciasFolders); //La function histoLoop ahora recibe el vector de los TDirectory de los eventos
    }
    
    ///////////////////////////////////////////////////////////////////
    // Read the Mesytec file
    /////////////////////////////////////////////////////////////////////
    
    //Open file
    const char* filename = mvlclstFile.c_str();
    printf("%s\n", filename);
    fp.open(mvlclstFile, std::ios::in | std::ios::binary);

    // Check if ti can be read
    if (!fp.is_open()) {
        std::cerr << "No se pudo abrir el archivo." << std::endl;
        return 1;
    }


     char dontcare[8];
     
    // Jump over MVLC_USB ascii and littleE marker
    fp.read(dontcare , 8);    
    bytes_counter += 8;

     cout << dontcare<< endl;
    // Use the readFame function to unapck
    arrayTest[0]->EvTree=EventTreeU;
  
    while (EOF_MARKER == 0) {
    readFrame(&fp,bytes_counter,DATAFRAME_marker,EOF_MARKER,totalEvents,continue_frame_counter,EventTreeU,treeFileU,arrayTest,broken_event_count);


     
    if(DATAFRAME_marker == 1){
            //VMMR8.PPP(EventTreeU);
            EventTreeU->Fill();
}
        if(fp.eof()){EOF_MARKER=1;}
    }
    
    ///////////////////////////////////////////////////////////////////
    // Write our results
    /////////////////////////////////////////////////////////////////////
    arrayTest[0]->write(broken_event_count);
    //EventTreeU->Print();
    
    EventTreeU->Write();
    treeFileU->Write();
    treeFileU->Close();
    fp.close(); 
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: The code is only can process Evntes 0 and 1, Good for IS690 but need to upgrade for future cases
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    std::cout <<"Event0 contados:"<< counterEvent0  << std::endl;
    std::cout <<"Event1 contados:"<< counterEvent1  << std::endl;
    std::cout <<"Broken event count:"<< broken_event_count  << std::endl;

    return 0;
}
