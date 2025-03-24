import binascii
import sys
import re
import subprocess


##########################################################################################################
# The purpose of this Python script is to read the mlvclst file and generate a configuration file that   #
# specifies the number of trigger events and the modules associated with each event.                     #
#                                                                                                        #
# This file is used by the unpacker to generate the tree file and call the unpacking functions in the    #
# correct order.                                                                                         #
#                                                                                                        #
# In more detail the code does the following:                                                            #
#  A-> Read quickly the hex file                                                                         #
#  B-> Detect the number of events                                                                       #             
#  C-> Detectc all modules (with the appropriate nickname)                                               #
#  D->Combine B and C in the configuration file                                                          #
#  E-> Execute the unpacker using the configuration file.                                                #
##########################################################################################################

#All functions in this python script are centered around finding specific hex patterns in the mlvlc file and decripting it

##################################
# Definition of useful functions #
##################################

def counterNumberEvents(file_path): 
    # The first step is to inedntfy the  number of trigger events (Event0,Event1,..) inside the hex list.
    # Every declaration of a event is precede by a the text mesy_readout_num_events.
    # This code obatins the number of events by searching for all instances off  mesy_readout_num_events, in hex
    try: 
        with open(file_path, 'rb') as file: 
            binary_data = file.read() # File to read
            patron_num_events="6D6573795F726561646F75745F6E756D5F6576656E7473223A" # mesy_readout_num_events": (Pattern to search)
            counter_events=0 # Trigger vent counter 
            start_index = binary_data.find(binascii.unhexlify(patron_num_events)) # Find the index with the first instance of the hex pattern
            # Use the while loop to find all instances of the hex and add a cont for each instance 
            while start_index != -1: 
                counter_events+=1
                start_index = binary_data.find(binascii.unhexlify(patron_num_events), start_index+1) # Find the index with the next instance of the hex pattern
            print("     ")
            print("Number of events detected: ", counter_events)
            return counter_events
    except FileNotFoundError:
        print(f"Error: File not found - {file_path}") 
    except Exception as e:
        print(f"Error: {e}")


def moduleDetector(file_path):

    #This function is responsible for detecting all types of modules present in the hex. It can also detect the nicknames given by the user. 
    # However, it seems that with this search pattern, we cannot determine the event they are associated with, hence the previous function.
    # As in the previous case the trick is to look for specific hex partern in this case all modules are have the string "type": before their model and the "name" before the nick


    patron_event="2274797065223A2022"# "type": ""
    patron_name= "226E616D65223A2022"# "name": ""

    # Not all modules are useful, some are virtual modules that don´t contribute good information, others we cannot unpack yet.
    # Since all not modules are good we will use a dicitionary to filter the good form the bad.

    # Hex patterns of the modules we currently can unpack
    hex_patterns = [
        "6d64707033325f736370", # MPDDP32_scp 
        "6d64707031365f716463", # MPDDP16_qdc
        "6d64707031365f736370", # MPDDP16_scp
        "766d6d72",  # vmmr
        "6D6164633332",  # madc32
        "6361656E5F76373835",  # caen_v785
        "6D766C635F6576656E745F7374616D706572", # mvlc_stamper
        "6D766C635F7473" # mvlc_ts,
        ]

    #Create a dictionary for the good modules
    dic_hex_patterns={
        "mdpp32_scp":hex_patterns[0],
        "mdpp16_qdc": hex_patterns[1],
        "mdpp16_scp": hex_patterns[2],
        "vmmr": hex_patterns[3],
        "madc32": hex_patterns[4],
        "caen_v785": hex_patterns[5],
        "mvlc_event_stamper": hex_patterns[6],
        "mvlc_ts":hex_patterns[7]
        }
    
    claves = dic_hex_patterns.keys() 
    previous_type_index=0

    dic_modules_detected=[] # List of tuples (nick, type)

    # We use a double loop the create a a tuple with two entrecs per modules (nick,tyoe)
    try:
        with open(file_path, 'rb') as file:
            binary_data = file.read() # Read bin file

            type_index = binary_data.find(binascii.unhexlify(patron_event)) #Find the inedx in the array with the first Module type hex
            while type_index != -1:

                # Get module type (we need to decript data between the "type" declaration and the end of that line (22)
                type_hex=binary_data.find(binascii.unhexlify("22"),type_index+len(binascii.unhexlify(patron_event))) # Jump the length of the "type" declaration and read until the next double quotes
                type_module = binary_data[type_index+len(binascii.unhexlify(patron_event)):type_hex].decode('utf-8') # Decode the selected line (recovering module type)
                if type_module in claves: # Check if good module type
                    
                    #Get name
                    name_index=binary_data.rfind(binascii.unhexlify(patron_name), previous_type_index, type_index) # Find index of patron name
                    name_hex=binary_data.find(binascii.unhexlify("22"),name_index+len(binascii.unhexlify(patron_name))) # Same operation as with the type
                    name_module=binary_data[name_index+len(binascii.unhexlify(patron_name)):name_hex].decode('utf-8')# Decode the selected line (recovering module name)

                    print("Module type: ", type_module, " ,with name: ",name_module)
                    dic_modules_detected.append((name_module,type_module))
                else :
                 print("Unknown module: ", type_module)
                # Actualize  the type index
                previous_type_index=type_index
                type_index = binary_data.find(binascii.unhexlify(patron_event),type_index+1)
        
        return dic_modules_detected
            
    except FileNotFoundError:
        print(f"Error: File not found - {file_path}") 
    except Exception as e:
        print(f"Error: {e}")


def FileGenerator(numEvents,modules,file_path,Structure_file):
    #########################################################################################################
    # This function responsible for generating the configuration file  that will be used in the unpacker.   #
    # It takes the result of the previous functions and orders it in a useful way for the unpack            #
    # The text will be read in sets of 3 lines in unpack since it will have the following structure:        #
    #   First line: Event associated with the module                                                        #
    #   Second line: Module's nickname                                                                      #
    #   Third line: Module type                                                                             #
    #########################################################################################################
    
    # Check if the module is off or on
    try:
        with open(file_path, 'rb') as file:
            binary_data = file.read()
            indicesEventos=[] # Empty tuple for trigger events
            motes=[]          # Empty tuple for modules
            patron_event="6576656E74"# Hex at the start of each event
            event_index=0

            #Import number of trigger events 
            for i in range(numEvents):
                event_index=binary_data.find(binascii.unhexlify(patron_event),event_index+1) # Serch for the n instances of the trigger pattern and get its index
                indicesEventos.insert(0,event_index) # Save each index in oposit oreder, ej A file with 3 events will be (2,1,0)= (200,130,34)
            
            # Import nicks and module type in the tuple array
            for i in range(len(modules)):
                motes.append((modules[i][0],modules[i][1]))

            patron_name="206E616D653A20" # Hex at the start of each name name: 

            listaEventos=[] 
            
            for mote,tipoModulo in motes: # Read the Tuple list 
                
                patron_actual=patron_name+ mote.encode('utf-8').hex() #Create the name+type combination
                
                name_index = binary_data.find(binascii.unhexlify(patron_actual)) # Find the index with the combination

                # LA SEGUNDA CONDICIÓN DEL IF ES PARA UN CASO CONCRETO DE MODULOS APAGADOS, POR EJEMPLO SI TIENES MDPP16 Y MDPP16_1, Y EL PRIMERO ESTUVIERA
                #APAGADO SI NO AÑADIMOS LA SEGUNDA CONDICIÓN LO CONFUNDIRIA CON ENCENDIDO.
                if name_index != -1 and binary_data[name_index+len(binascii.unhexlify(patron_actual))]==0xA: # Check the module is on
                    for i in range(len(indicesEventos)): # Serch the event list
                        # Compare the index of the combination with the index of trigger if it is larger it goes benith it
                        #Ex: A combination has index 150, fie Ev2 has index 200 and ev0 index 130 then that module is associated with ev
                        if name_index>indicesEventos[i]:  
                            listaEventos.append([len(indicesEventos)-i-1,mote,tipoModulo]) # Crete a Tuple [Ev,Mod,Name]
                            break
                    
                else: #Check the module is off
                    print("Module off doesn´t give relevant info: ",mote)
            
           
            # Write the configurarion file
            with open(Structure_file, 'w') as output:
                for i in range(0,len(listaEventos)):
                    output.write(f"{listaEventos[i][0]}\n")
                    output.write(f"{listaEventos[i][1]}\n")
                    if i <len(listaEventos)-1:
                        
                        output.write(f"{listaEventos[i][2]}\n")
                    else:
                        output.write(f"{listaEventos[i][2]}")
                
        print(f"Events Structure written to {Structure_file}")
        print("     ")

    except FileNotFoundError:
        print(f"Error: File not found - {file_path}") 
    except Exception as e:
        print(f"Error: {e}")

##############################################
# Main code; import the mlvcst generate the  #
# configuration file and execute the unpacker #
##############################################

if __name__ == "__main__":
    # Check if two arguments (file path and output file) are provided
    if len(sys.argv) != 4:
        print("Usage: python script.py <file_path> <output_file>")
    else:
        # Get the file path and names for the output, module list and executable
        mlvclst_path   = sys.argv[1] # Mesytec file
        config_file    = sys.argv[2] # Config file
        RootFile_path  = sys.argv[3] # Output file
        cpp_executable_path = "./Unpack_mvme.out"  

        # Obatin numbre of events
        events=counterNumberEvents(mlvclst_path)
        print("Events: ",events)

        # Obtain modules and their nicks
        modules=moduleDetector(mlvclst_path)

        # Sitch together the events modules and names ito  a configuration file
        FileGenerator(events,modules,mlvclst_path,config_file)

        # Call the Unpakcer
        subprocess.run([cpp_executable_path,config_file,mlvclst_path,RootFile_path])
