
// TODO CHANGE FILE PATHS IN CODE ACCORDING TO YOUR ENVIRONMENT. THE REST OF THE CODE IS PORTABLE, JUST COPY THE PROJECT FOLDER AND RUN PREFERABLY IN CLION
// INCLUDING NECESSARY HEADER FILES
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <ctime>
#include <cstdint>
#include <map>
#include "bf/all.hpp"
#include "rapidjson/document.h"
#include "rapidjson//istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "helpers/primitives.h"
#include "cpp-datetimelite/datetimelite.h"
#include <bits/stdc++.h>
#include <cstdlib>
#include <random>

//NAMESPACE SCOPE
using namespace datetimelite;
using namespace std;
using namespace bf;
using namespace rapidjson;

//GLOBAL VARIABLES DEFINED, MOST NAMES ARE SELF EXPLANATORY
//COUNTERS FOR SANITY CHECKING, DEBUGGING AND RESULTS CALCULATION
int init=1, last_debug_time=0, last_expiry_time=0, last_clear_time=0, last_rotate_time=0, inventory_time=0, mempool_time=0;
int hour=0, rbf_missed=0, rbf_replaced=0, total_rbf, tp=0, tn=0, fp=0, fn=0, fp_entry=0, fn_entry=0, tp_entry=0, tn_entry=0,
    fp_exit=0, fn_exit=0, tp_exit=0, tp_inv=0, tn_inv=0, fp_inv=0, fn_inv=0, tp_ro=0, fp_ro=0, fn_ro=0, tn_ro=0;
int h_rbf_missed=0, h_rbf_replaced=0, h_rbf=0, h_tp=0, h_tn=0, h_fp=0, h_fn=0, h_fp_entry=0, h_fn_entry=0, h_tp_entry=0, h_tn_entry=0,
    h_fp_exit=0, h_fn_exit=0, h_tp_exit=0, h_tp_inv=0, h_tn_inv=0, h_fp_inv=0, h_fn_inv=0, h_tp_ro=0, h_fp_ro=0, h_fn_ro=0, h_tn_ro=0;
int h_unique_tx=0, h_total_tx=0, h_entry_tx=0, h_added_tx=0, h_exit_tx=0, h_block_tx=0, h_non_block=0, h_non_block_not_removed=0, h_old_tx=0, h_removed_tx=0, unique_inventory_fp=0, unique_inventory_fn=0;
int old_tx=0, entry_tx=0, exit_tx=0, current_mempool_tx=0, removed_tx=0, added_tx=0, total_tx=0, current_mempool_one=0, current_mempool_two=0, ex_hour=1, h_unique_inventory_fp=0, h_unique_inventory_fn=0;;
int ro_count=0, replace_tx=0, conflict_tx=0, evict_tx=0, reorg_tx=0, expire_tx=0, block_tx=0, non_block=0, unique_tx=0;
int max_bucket_count_cbf=0, max_bucket_count_txin=0, width=4, inventory_tx=0, min_rbf_check=0 , h_inventory_tx=0;
int rbf_check, cbf_mempool_lookup, expiry_hours=6*1, non_block_not_removed=0;
int unique_fn=0, unique_fp=0, h_unique_fn=0, h_unique_fp=0, fn_index=0, h_fn_index=0, fn_index_inv=0, fn_index_entry=0, fn_index_exit=0, h_fn_index_inv=0, h_fn_index_entry=0, h_fn_index_exit=0;
string status;
std::map <string, int> entry_positives, inventory_positives, inventory_negatives;
int fn_expiry_inv=0, fn_expiry_entry=0, fn_expiry_exit=0, fn_expiry=0, h_fn_expiry_inv=0, h_fn_expiry_entry=0, h_fn_expiry_exit=0, h_fn_expiry=0, speedup_factor=1;
int h_cm=0, h_cm_inv=0,h_cm_entry=0, h_cm_exit=0;
int cm=0, cm_inv=0, cm_entry=0, cm_exit=0;
int start_timestamp=1609459200; // 00:00 1 Jan 2021
int active_filter=1;
//int start_timestamp=1609459200+1480*3600;

bf::counting_bloom_filter cbf_mempool_one(make_hasher(14, time(0), true), 8000000, 1);
bf::counting_bloom_filter cbf_mempool_two(make_hasher(14, time(0), true), 8000000, 1);
//bf::counting_bloom_filter bf_txin(make_hasher(4,time(0), true), 4000000, width);

std::vector<size_t> tx_indices;

//DETERMINISTIC FOR PERFORMANCE
    std::map <string, int> shadow_mempool;  // tx hash, unix time
    std::map < std::pair<string, int>, int> referenced_outputs;  // (ref tx hash, index), unixtime
    std::map <string,int> shadow_inventory;
    std::map <string,int> shadow_cbf;
   // std::map <string,int> get_data;

//Function to convert date time format to unix timestamp format
std::uint64_t getUnixTimestampFromString(const std::string& ts_str){
    struct std::tm ts = time_from_string(ts_str);
    return timegm(&ts);
}

/*void copy_file( string& srce_file, string& dest_file )
{
    std::ifstream srce( srce_file, std::ios::binary ) ;
    std::ofstream dest( dest_file, std::ios::binary ) ;
    dest << srce.rdbuf() ;
    srce.close();
    dest.close();
}*/

string confusion_matrix_cbf_mempool_entry(const string& tx_type, const string& txid, int shadow, int cbfmem, std::vector<size_t> myIndices ) {
    string cm_return;
// cm METRICS UPDATED
    if (shadow>0 && cbfmem>0) {
        tp_entry++;
        h_tp_entry++;
        cm_return="TP";
    }

    if (shadow==0 && cbfmem==0) {
            tn_entry++;
            h_tn_entry++;
            cm_return="TN";

    }

    if (shadow==0 && cbfmem>0) {
        fp_entry++;
        h_fp_entry++;
        cm_return="FP";

       /*   debug_falsepositives<<hour<<","<<txid<<","<<tx_type<<",";

        for (auto it=myIndices.begin() ; it < myIndices.end(); it++ ) {

            debug_falsepositives <<*it<<",";
        }
        debug_falsepositives<<endl; */

    }
    if (shadow>0 && cbfmem==0) {
        fn_entry++;
        h_fn_entry++;
        if (shadow_cbf.count(txid)>=1) {
             fn_index_entry++;
             h_fn_index_entry++;
    /*   debug_falsenegatives<<hour<<","<<txid<<","<<tx_type<<",";

        for (auto it=myIndices.begin() ; it < myIndices.end(); it++ ) {

            debug_falsenegatives <<*it<<",";
        }*/
        }
    }

    return cm_return;
}

string confusion_matrix_cbf_mempool_exit(const string& tx_type, const string& tx_reason, const string& txid, int shadow, int cbfmem, std::vector<size_t> myIndices ) {
string cm_return;
// cm METRICS UPDATED
    if (shadow>0 && cbfmem>0) {
        tp_exit++;
        h_tp_exit++;
        cm_return="TP";
    }

    if (shadow==0 && cbfmem==0) {
        old_tx++;
        h_old_tx++;
        cm_return = "OLD";
    }
    if (shadow==0 && cbfmem>0) {
            fp_exit++;
            h_fp_exit++;
            cm_return = "FP";
        
  /*      debug_falsepositives<<hour<<","<<txid<<","<<tx_type<<",";

        for (auto it=myIndices.begin() ; it < myIndices.end(); it++ ) {

            debug_falsepositives <<*it<<",";
        }
         debug_falsepositives<<endl; */
      
    }
    if (shadow>0 && cbfmem==0) {
        fn_exit++;
        h_fn_exit++;
        if (shadow_cbf.count(txid) >= 1) {
            fn_index_exit++;
            h_fn_index_exit++;
        }
    }
      
    return cm_return;
}

string confusion_matrix_cbf_mempool_inventory(const string& tx_type, const string& txid, int shadow, int cbfmem, std::vector<size_t> myIndices ) {
    string cm_return;
// cm METRICS UPDATED
    if (shadow>0 && cbfmem>0) {
        tp_inv++;
        h_tp_inv++;
        cm_return="TP";
    }

    if (shadow==0 && cbfmem==0) {
        tn_inv++;
        h_tn_inv++;
        cm_return = "TN";
      //  get_data.insert(make_pair(txid, mempool_time));
    }

    if (shadow==0 && cbfmem>0) {
        fp_inv++;
        h_fp_inv++;
        cm_return="FP";
       if (inventory_positives.count(txid) == 0) {
             unique_inventory_fp++;
             h_unique_inventory_fp++;
   /*     debug_falsepositives<<hour<<","<<txid<<","<<tx_type<<",";

        for (auto it=myIndices.begin() ; it < myIndices.end(); it++ ) {

            debug_falsepositives <<*it<<",";
        }
       debug_falsepositives<<endl;
*/
       }
       inventory_positives.insert(make_pair(txid, inventory_time));
  //      debug_inventory_positives << txid << endl;
    }
   
    if (shadow>0 && cbfmem==0) {

      //  if (get_data.count(txid) == 1) {
      //  tp_inv++;
      //  h_tp_inv++;
      //  }
      //  else {

        fn_inv++;
        h_fn_inv++;
       
        cm_return="FN";
  if (inventory_negatives.count(txid) == 0) {
             unique_inventory_fn++;
             h_unique_inventory_fn++;
      if (shadow_cbf.count(txid)>=1) {
             fn_index_inv++;
             h_fn_index_inv++;
     /*     debug_falsenegatives<<hour<<","<<txid<<","<<tx_type<<",";

        for (auto it=myIndices.begin() ; it < myIndices.end(); it++ ) {

            debug_falsenegatives <<*it<<",";
        }
 	    debug_falsenegatives<<endl;*/
        }
       }
       inventory_negatives.insert(make_pair(txid, inventory_time));
    //}
  }

    return cm_return;
}

void initialize(int sim_time) {
    if (init == 1) {                              //initialize last expiry and debug time
        last_debug_time = sim_time;
        last_rotate_time=sim_time;
        last_clear_time=sim_time;
        init = 0;
    }
}

string confusion_matrix_bf_txin(const string& tx, int ref, int txin) {
    string cm_return;

    if (ref>0 && txin>0) {
        tp_ro++;
        h_tp_ro++;
        cm_return = "TP";
    }
    else if (ref==0 && txin==0) {
        tn_ro++;
        h_tn_ro++;
        cm_return="TN";
    }
    else if (ref==0 && txin>0) {
        fp_ro++;
        h_fp_ro++;
        cm_return="FP";
    }
    else if (ref>0 && txin==0) {
        fn_ro++;
        h_fn_ro++;
        cm_return="FN";
    }
    return cm_return;
}

vector<size_t> get_indices(const string& tx, const string& type ) {
    std::vector<size_t> indices;
   //if(num==1)
       indices = cbf_mempool_one.find_indices(wrap(tx));
  //  else
    //   indices = cbf_mempool_two.find_indices(wrap(tx));

  /* debug_forensics <<hour<<","<<tx << "," << type << ",";

 for (auto ind = indices.begin(); ind < indices.end(); ind++) {
        debug_forensics << *ind << ",";
     }
    debug_forensics << endl;*/
    return indices;
}

//MAIN FUNCTION BEGINS HERE
int main() {

//MEMPOOL REPLACEMENT ELEMENTS

    ofstream debug_hourly ("pathtodebug/debug/debug_hourly.csv" );
    ofstream debug_cumulative ("pathtodebug/debug/debug_cumulative.csv" );
  
    debug_cumulative << "hour"
                 << "," <<"inventory_time"
                 << ","<<"inventory_tx"
                 <<","<<"cm inv"
                 << ","<<"unique_tx"
                 << ","<<"unique_fp"
                 << ","<<"unique_fn"
                 <<","<<"tp_inv"
                 <<","<<"tn_inv"
                 <<","<<"fp_inv"
                 <<","<<"fn_inv"
                 <<","<<"fn_index_inv"
                 <<","<<"fn_expiry_inv"
                 << "," << "FPR inv"
                 << "," << "FNR inv"
                 << "," << "Accuracy % inv"
                 << "," << "Reprocess % inv"

                 << "," << "total_tx"
                 <<","<< "cm total"
                 << "," << "entry_tx"
                 << "," << "cm entry"
                 << "," << "added_tx"
                 << "," << "tp_entry"
                 << "," << "tn_entry"
                 << "," << "fp_entry"
                 << "," << "fn_entry"
                 << "," <<"fn_index_entry"
                 << "," << "fn_expiry_entry"
                    << "," << "FPR Entry"
                     << "," << "FNR Entry"
                     << "," << "Accuracy % Entry"
                     << "," << "Reprocess % Entry"

                 << "," << "exit_tx"
                 << "," <<"cm exit"
                 << "," << "exited_tx"
                 << "," << "block_tx"
                 << "," << "debris"
                 << "," << "tp_exit"
                 << "," << "tn_exit"
                 << "," << "fp_exit"
                 << "," << "fn_exit"
                 <<","<<"fn_index_exit"
                 << "," << "fn_expiry_exit"
                 << "," << "fn_index"
                 << "," << "fn_expiry"
                 << "," << "FPR Exit"
                 << "," << "FNR Exit"
                 << "," << "Accuracy % Exit"
                 << "," << "Reprocess % Exit"

                 << "," << "FPR"
                 << "," << "FNR"
                 << "," << "Accuracy %"
                 << "," << "Reprocess %"
                 << "," << "current_mempool_tx"
                 << "," << "shadow_mempool.size()" 
                 << "," << "(fp_entry-fn_entry+fp_exit-fn_exit)"
                 << "," << "non_block"
                 << "," << "shadow check"
                 << "," << "debris check"
                 << "," << "active filter"
                 <<endl;

    debug_hourly << "hour"
                     << "," <<"inventory_time"
                     << ","<<"inventory_tx"
                     <<","<<"cm inv"
                     << ","<<"unique_tx"
                 << ","<<"unique_fp"
                 << ","<<"unique_fn"
                     <<","<<"tp_inv"
                     <<","<<"tn_inv"
                     <<","<<"fp_inv"
                     <<","<<"fn_inv"
                     <<","<<"fn_index_inv"
                     << "," << "fn_expiry_inv"
                     << "," << "FPR inv"
                     << "," << "FNR inv"
                     << "," << "Accuracy % inv"
                     << "," << "Reprocess % inv"
                     << "," << "total_tx"
                     << "," << "cm total"
                     << "," << "entry_tx"
                     << "," << "cm entry"
                     << "," << "added_tx"
                     << "," << "tp_entry"
                     << "," << "tn_entry"
                     << "," << "fp_entry"
                     << "," << "fn_entry"
                     << "," << "fn_index_entry"
                     << "," << "fn_expiry_entry"
                     << "," << "FPR Entry"
                     << "," << "FNR Entry"
                     << "," << "Accuracy % Entry"
                     << "," << "Reprocess % Entry"

                     << "," << "exit_tx"
                     << "," << "cm exit"
                     << "," << "exited_tx"
                     << "," << "block_tx"
                     << "," << "non_block"
                     << "," << "tp_exit"
                     << "," << "tn_exit"
                     << "," << "fp_exit"
                     << "," << "fn_exit"
                     << "," << "fn_index_exit"
                     << "," << "fn_expiry_exit"
                     << "," << "fn_index"
                     << "," << "fn_expiry"
                     << "," << "FPR Exit"
                     << "," << "FNR Exit"
                     << "," << "Accuracy % Exit"
                     << "," << "Reprocess % Exit"

                     << "," << "FPR"
                     << "," << "FNR"
                     << "," << "Accuracy %"
                     << "," << "Reprocess %"
                     << "," << "current_mempool_tx"
                     << "," << "shadow_mempool.size()"
                    
                     <<endl;


  /*  if (newfile.is_open()) {   //checking whether the file is open

        getline(newfile, line); // read data from file object and put it into string.
        inventory_time = stoi(line.substr(0, 10));
        inv_type = line.substr(11, 1);
        inv_txhash = line.substr(13, 64);
        while(inventory_time<=start_timestamp) {
                 getline(newfile, line);
                 inventory_time = stoi(line.substr(0, 10));
                 inv_type = line.substr(11, 1);
                 inv_txhash = line.substr(13, 64);

             }
    }*/

    fstream newfile;
    string file_inv = string("pathtodebug/inventory/inventory.csv");
    newfile.open(file_inv,ios::in); //open a file to perform read operation using file object

    string line, inv_type, inv_txhash;

    getline(newfile, line); // read data from file object and put it into string.
    inventory_time = stoi(line.substr(0, 10));

    while (inventory_time<=start_timestamp){
        getline(newfile, line); // read data from file object and put it into string.
        inventory_time = stoi(line.substr(0, 10));
    }

        for (int j = 1; j <= 183; j++) {
            // for (int j = 1; j <= 26; j++) {

            string file_name = string("pathtodebug/mem/" + to_string(j + 71) + ".json");
            cout << file_name << endl;

            ifstream ifs(file_name);

            //CHECK IF OPEN SUCCESSFUL
            if (!ifs.is_open() && !newfile.is_open()) {
                std::cerr << "Could not open file for reading!\n";
                return EXIT_FAILURE;
            }

            //READING JSON FILE
            IStreamWrapper isw{ifs};
            Document doc{};
            doc.ParseStream(isw);

            //ITERATE THROUGH THE FILE ONE TRANSACTION AT A TIME (EACH TRANSACTION IS IN JSON FORMAT)
            for (auto itr = doc.Begin(); itr != doc.End(); ++itr) {

                /*    if (mempool_time - 3600 > last_clear_time) {
                        last_clear_time = mempool_time;
                        bf_txin.clear();
                        max_bucket_count_txin=0;
                    }*/

                if (mempool_time - (3600 * 12) > last_expiry_time) {

                    auto ro = referenced_outputs.begin();
                    while (ro != referenced_outputs.end()) {
                        if (ro->second <
                            (mempool_time - (3600 * 6)))   // Check if value of this entry matches with given value
                            referenced_outputs.erase(ro->first); // erase

                        ro++;
                    }

                    /*    auto si = shadow_inventory.begin();
                        while (si != shadow_inventory.end()) {
                            if (si->second < (mempool_time - (3600 * expiry_hours * 5 )))   // Check if value of this entry matches with given value
                                shadow_inventory.erase(si->first); // erase
                            si++;
                        }*/

                    auto sm = shadow_mempool.begin();
                    while (sm != shadow_mempool.end()) {
                        if (sm->second < (mempool_time - 3600 * 24 *14))   // Check if value of this entry matches with given value
                            shadow_mempool.erase(sm->first); // erase

                        sm++;
                    }

                    last_expiry_time = mempool_time;

                }

                if (mempool_time >= (last_rotate_time + 3600)) {  // rotate
                    last_rotate_time = mempool_time;

                    if ((hour % (expiry_hours * 2) == 0)) {


                        cbf_mempool_one.clear();
                        current_mempool_one = 0;

                        auto sc = shadow_cbf.begin();
                        while (sc != shadow_cbf.end()) {
                            if (sc->second == 1)   // Check if value of this entry matches with given value
                                shadow_cbf.erase(sc->first); // erase
                            sc++;
                        }

                        active_filter = 1;


                    } else if ((hour % expiry_hours) == 0) {

                        cbf_mempool_two.clear();
                        current_mempool_two = 0;

                        auto sc = shadow_cbf.begin();
                        while (sc != shadow_cbf.end()) {
                            if (sc->second == 2)   // Check if value of this entry matches with given value
                                shadow_cbf.erase(sc->first); // erase
                            sc++;
                        }

                        active_filter = 2;
                    }

                }

/*            if ((mempool_time - last_rotate_time)> 3600) {  // rotate
                last_rotate_time = mempool_time;
                ex_hour++;

                if (ex_hour==expiry_hours) {            //15
                    cbf_mempool_two.clear();
                    current_mempool_two=0;
                     
                auto sc = shadow_cbf.begin();
                while (sc != shadow_cbf.end()) {
                    if (sc->second==2)   // Check if value of this entry matches with given value
                        shadow_cbf.erase(sc->first); // erase

                    sc++;
                }
                    
                }
                else if (ex_hour==2*expiry_hours) {      //29
                    cbf_mempool_one.clear();
                    current_mempool_one=0;
                    shadow_cbf.clear();
                    ex_hour = 0;
 
                auto sc = shadow_cbf.begin();
                	while (sc != shadow_cbf.end()) {
                    		if (sc->second==1)   // Check if value of this entry matches with given value
                        		shadow_cbf.erase(sc->first); // erase
                    		sc++;
               		}
                }

            }

       
		if (current_mempool_one>= 200000 || current_mempool_two>=200000) {  // rotate
                last_rotate_time = mempool_time;
             //   ex_hour++;

                if (current_mempool_one>= 200000) {            //15
                    cbf_mempool_two.clear();
                    current_mempool_two=0;
                    ex_hour=expiry_hours;
                     
                auto sc = shadow_cbf.begin();
                while (sc != shadow_cbf.end()) {
                    if (sc->second==2)   // Check if value of this entry matches with given value
                        shadow_cbf.erase(sc->first); // erase

                    sc++;
                }
                    
                }
                else if (current_mempool_two>= 200000 ) {      //29
                    cbf_mempool_one.clear();
                    current_mempool_one=0;
                    shadow_cbf.clear();
                    ex_hour = 0;
 
                auto sc = shadow_cbf.begin();
                while (sc != shadow_cbf.end()) {
                    if (sc->second==1)   // Check if value of this entry matches with given value
                        shadow_cbf.erase(sc->first); // erase

                    sc++;
                }
                }

            }


*/
                const Value &type = (*itr)["type"];           //TRANSACTION TYPE: EXIT OR ENTRY

                if (type == "ENTRY") {

                    //READ EACH TRANSACTION FIELD
                    const Value &time = (*itr)["time"];
                    const Value &hash = (*itr)["transaction_hash"];
                    const Value &TxIns = (*itr)["TxIn"];

                    Tx *tx = new Tx(type.GetString(), hash.GetString(), time.GetString()), txin;

                    std::cout << tx->getHash().ToString() << "   " << tx->getType() << "  " << tx->getTime()
                              << endl; //CONSOLE OUTPUT: ENTRY TRANSACTION ID, TYPE, UNIX TIMESTAMP

                    mempool_time = getUnixTimestampFromString(tx->getTime());  //get simulation time

                    //initialize simulation time, run once


                        if (mempool_time > start_timestamp) {

                    initialize(mempool_time);

                while (inventory_time <= mempool_time) {
                            //checking whether the file is open
                            getline(newfile, line); // read data from file object and put it into string.
                            inventory_time = stoi(line.substr(0, 10));

                                inventory_tx++;
                                h_inventory_tx++;
                                inv_type = line.substr(11, 1);
                                inv_txhash = line.substr(13, 64);

                                int cbf_mempool_lookup_one = cbf_mempool_one.lookup(inv_txhash);
                                int cbf_mempool_lookup_two = cbf_mempool_two.lookup(inv_txhash);

                                if (cbf_mempool_lookup_one >= cbf_mempool_lookup_two)
                                    cbf_mempool_lookup = cbf_mempool_lookup_one;
                                else
                                    cbf_mempool_lookup = cbf_mempool_lookup_two;

                                int shadow_mempool_lookup = shadow_mempool.count(inv_txhash);
                                int shadow_inventory_lookup = shadow_inventory.count(inv_txhash);

                                tx_indices = get_indices(inv_txhash, "i");  //GET INDEXES FOR FORENSICS


                                if (shadow_inventory_lookup == 0) {
                                    shadow_inventory.insert(make_pair(inv_txhash, inventory_time));
                                    unique_tx++;
                                    h_unique_tx++;
                                }

                                confusion_matrix_cbf_mempool_inventory(inv_type, inv_txhash, shadow_mempool_lookup,
                                                                       cbf_mempool_lookup, tx_indices);
                }

                    //INCREMENT ENTRY TRANSACTIONS COUNTER

                    if (shadow_inventory.count(tx->getHash().ToString()) == 1 &&
                        inventory_positives.count(tx->getHash().ToString()) ==
                        0 /*&& shadow_mempool.count(tx->getHash().ToString())==0*/) {

                        total_tx++;
                        h_total_tx++;
                        entry_tx++;
                        h_entry_tx++;

                        int cbf_mempool_lookup_one = cbf_mempool_one.lookup(tx->getHash().ToString());
                        int cbf_mempool_lookup_two = cbf_mempool_two.lookup(tx->getHash().ToString());

                        if (cbf_mempool_lookup_one >= cbf_mempool_lookup_two)
                            cbf_mempool_lookup = cbf_mempool_lookup_one;
                        else
                            cbf_mempool_lookup = cbf_mempool_lookup_two;

                        int shadow_mempool_lookup = shadow_mempool.count(tx->getHash().ToString());

                        tx_indices = get_indices(tx->getHash().ToString(), tx->getType());  //GET INDEXES FOR FORENSICS

                        confusion_matrix_cbf_mempool_entry(tx->getType(), tx->getHash().ToString(), shadow_mempool_lookup,
                                                           cbf_mempool_lookup, tx_indices);
                        /*    if (cbf_mempool_lookup > max_bucket_count_cbf) {
                                max_bucket_count_cbf = cbf_mempool_lookup;
                                debug_overflow << "max cbf, " << max_bucket_count_cbf << endl;
                            }*/

                        shadow_mempool.insert(make_pair(tx->getHash().ToString(), mempool_time));

  //                      int min_bucket_count_txin = 15;

                        //ITERATE THROUGH THE REFERENCED OUTPUTS
                        /*        for (auto itr2 = TxIns.GetArray().Begin(); itr2 != TxIns.GetArray().End(); ++itr2) {
                                    const Value &outpoint = (*itr2)["outpoint"];

                                    uint64_t txin_index;
                                    std::istringstream iss(outpoint["index"].GetString());
                                    iss >> txin_index;

                                    TxIn *txin = new TxIn(outpoint["Prev_out_hash"].GetString(), txin_index);
                                    tx->getvTxIn().push_back(txin);

                                    int bf_txin_lookup = bf_txin.lookup(
                                            to_string(txin->getIndex()) + txin->getHash().ToString());

                                    min_rbf_check = 10;
                                    rbf_check = referenced_outputs.count(
                                            make_pair(txin->getHash().ToString(), txin->getIndex()));

                                    if (rbf_check < min_rbf_check) {
                                        min_rbf_check = rbf_check;
                                    }
                                    // CHECK IF REFERENCED OUTPUT ALREADY EXISTS REFERENCED_OUTPUTS
                                    confusion_matrix_bf_txin(tx->getHash().ToString(), referenced_outputs.count(
                                            make_pair(txin->getHash().ToString(), txin->getIndex())),
                                                                  bf_txin_lookup);

                                    if (bf_txin_lookup > max_bucket_count_txin) {
                                        max_bucket_count_txin = bf_txin_lookup;
                                        debug_overflow << "max bf txin, " << max_bucket_count_txin << endl;

                                    }

                                    if (bf_txin_lookup < min_bucket_count_txin) {
                                        min_bucket_count_txin = bf_txin_lookup;
                                    }

                                    referenced_outputs.insert(
                                            make_pair(make_pair(txin->getHash().ToString(), txin->getIndex()), mempool_time));

                                    ro_count++;
                                }

                                if (min_rbf_check > 0) {
                                    total_rbf++;
                                    h_rbf++;
                                }

                                if (min_bucket_count_txin > 2) {
                                    rbf_missed++;
                                    h_rbf_missed++;
                                }*/
                        //double_spend++;

                        if (cbf_mempool_lookup == 0) {

                            if (active_filter == 1) {                  //14
                                cbf_mempool_one.add(tx->getHash().ToString());
                                current_mempool_one++;
                                shadow_cbf.insert(make_pair(tx->getHash().ToString(), 1));

                            } else if (active_filter == 2) {        //14
                                cbf_mempool_two.add(tx->getHash().ToString());
                                current_mempool_two++;
                                shadow_cbf.insert(make_pair(tx->getHash().ToString(), 2));
                            }

                            /*                        if (ex_hour < expiry_hours) {                //14
                                                       cbf_mempool_one.add(tx->getHash().ToString());
                                                       current_mempool_one++;
                                       shadow_cbf.insert(make_pair(tx->getHash().ToString(), 1));

                                                   } else if (ex_hour >= expiry_hours) {         //14
                                                       cbf_mempool_two.add(tx->getHash().ToString());
                                                       current_mempool_two++;
                                                       shadow_cbf.insert(make_pair(tx->getHash().ToString(), 2));
                                                   }*/
                            added_tx++;
                            h_added_tx++;
                            //current_mempool_tx++;

                            //ADD EACH REFERENCED OUTPUT TO
                            /*   for (auto itr2 = TxIns.GetArray().Begin(); itr2 != TxIns.GetArray().End(); ++itr2) {
                                   const Value &outpoint = (*itr2)["outpoint"];

                                   uint64_t txin_index;
                                   std::istringstream iss(outpoint["index"].GetString());
                                   iss >> txin_index;

                                   TxIn *txin = new TxIn(outpoint["Prev_out_hash"].GetString(), txin_index);
                                   tx->getvTxIn().push_back(txin);

                                   int bf_txin_lookup = bf_txin.lookup(
                                           to_string(txin->getIndex()) + txin->getHash().ToString());

                                   if (bf_txin_lookup == 0) {

                                       bf_txin.add(to_string(txin->getIndex()) + txin->getHash().ToString());
                                   }
                               }*/
                            }
                        }
                    }
                }

                if (type == "EXIT") {

                    const Value &reason = (*itr)["reason"];
                    const Value &time = (*itr)["time"];
                    const Value &hash = (*itr)["transaction_hash"];

                    auto *tx = new Exit_Tx(type.GetString(), reason.GetString(), hash.GetString(), time.GetString());
                    std::cout << tx->getHash().ToString() << "   " << tx->getType() << "  " << tx->getTime() << endl;

                    mempool_time = getUnixTimestampFromString(tx->getTime());

                       if (mempool_time > start_timestamp) {

                           initialize(mempool_time);

                           while (inventory_time <= mempool_time) {

                               getline(newfile, line); // read data from file object and put it into string.
                               inventory_tx++;
                               h_inventory_tx++;
                               inventory_time = stoi(line.substr(0, 10));
                               inv_type = line.substr(11, 1);
                               inv_txhash = line.substr(13, 64);


                               int cbf_mempool_lookup_one = cbf_mempool_one.lookup(inv_txhash);
                               int cbf_mempool_lookup_two = cbf_mempool_two.lookup(inv_txhash);

                               if (cbf_mempool_lookup_one >= cbf_mempool_lookup_two)
                                   cbf_mempool_lookup = cbf_mempool_lookup_one;
                               else
                                   cbf_mempool_lookup = cbf_mempool_lookup_two;

                               int shadow_mempool_lookup = shadow_mempool.count(inv_txhash);
                               int shadow_inventory_lookup = shadow_inventory.count(inv_txhash);

                               if (shadow_inventory_lookup == 0) {
                                   shadow_inventory.insert(make_pair(inv_txhash, inventory_time));
                                   unique_tx++;
                                   h_unique_tx++;
                               }

                               tx_indices = get_indices(inv_txhash, "i");  //GET INDEXES FOR FORENSICS

                               confusion_matrix_cbf_mempool_inventory(inv_type, inv_txhash, shadow_mempool_lookup,
                                                                      cbf_mempool_lookup, tx_indices);
                           }

                           total_tx++;
                           h_total_tx++;
                           exit_tx++;
                           h_exit_tx++;

                       //    if (exit_tx % speedup_factor == 0) {

                               if (tx->getReason() == "REPLACED") {
                                   if (shadow_mempool.count(tx->getHash().ToString()) > 0) {
                                       non_block++;//replace_tx++;
                                       h_non_block++;
                                   }
                               } else if (tx->getReason() == "CONFLICT") {
                                   if (shadow_mempool.count(tx->getHash().ToString()) > 0) {
                                       non_block++;//conflict_tx++
                                       h_non_block++;
                                   }
                               } else {
                                   h_block_tx++;
                                   block_tx++;

                                   int cbf_mempool_lookup_one = cbf_mempool_one.lookup(tx->getHash().ToString());
                                   int cbf_mempool_lookup_two = cbf_mempool_two.lookup(tx->getHash().ToString());

                                   if (cbf_mempool_lookup_one >= cbf_mempool_lookup_two)
                                       cbf_mempool_lookup = cbf_mempool_lookup_one;
                                   else
                                       cbf_mempool_lookup = cbf_mempool_lookup_two;

                                   int shadow_mempool_lookup = shadow_mempool.count(tx->getHash().ToString());
                                   tx_indices = get_indices(tx->getHash().ToString(),
                                                            tx->getType());  //GET INDEXES FOR FORENSICS

                                   tx_indices = get_indices(tx->getHash().ToString(),
                                                            tx->getType());  //GET INDEXES FOR FORENSICS

                                   confusion_matrix_cbf_mempool_exit(tx->getType(), tx->getReason(),
                                                                     tx->getHash().ToString(),
                                                                     shadow_mempool_lookup, cbf_mempool_lookup,
                                                                     tx_indices);

                                 /*  if (cbf_mempool_lookup_one > 0) {

                                       cbf_mempool_one.remove(tx->getHash().ToString());
                                       current_mempool_one--;
                                       removed_tx++;
                                       h_removed_tx++;

                                   } else if (cbf_mempool_lookup_two > 0) {

                                       cbf_mempool_two.remove(tx->getHash().ToString());
                                       current_mempool_two--;
                                       removed_tx++;
                                       h_removed_tx++;

                                   } */

                                   shadow_mempool.erase(tx->getHash().ToString());

                                   /*   if (cbf_mempool_lookup > max_bucket_count_cbf) {
                                          max_bucket_count_cbf = cbf_mempool_lookup;
                                          debug_overflow << "max bucket count cbf, " << max_bucket_count_cbf << endl;
                                      }*/


                               }
                           }
                      // }
                    }

                if (mempool_time > start_timestamp + 3600) {

                    if (mempool_time - last_debug_time >= 3600) {

                        tp = tp_entry + tp_exit + tp_inv;
                        h_tp = h_tp_entry + h_tp_exit + h_tp_inv;

                        tn = tn_entry + tn_inv;
                        h_tn = h_tn_entry + h_tn_inv;

                        fp = fp_entry + fp_exit + fp_inv;
                        h_fp = h_fp_entry + h_fp_exit + h_fp_inv;

                        fn = fn_entry + fn_exit + fn_inv;
                        h_fn = h_fn_entry + h_fn_exit + h_fn_inv;
                        current_mempool_tx = current_mempool_two + current_mempool_one;

                        unique_fp = unique_inventory_fp + fp_entry + fp_exit;
                        unique_fn = unique_inventory_fn + fn_entry + fn_exit;

                        cm_inv = tp_inv + tn_inv + fp_inv + fn_inv;
                        cm_entry = tp_entry + tn_entry + fp_entry + fn_entry + tp_exit + fp_exit + fn_exit;
                        cm_exit = tp_exit + fp_exit + fn_exit;
                        cm = cm_inv + cm_entry + cm_exit;

                        h_cm_inv = h_tp_inv + h_tn_inv + h_fp_inv + h_fn_inv;
                        h_cm_entry =
                                h_tp_entry + h_tn_entry + h_fp_entry + h_fn_entry + h_tp_exit + h_fp_exit + h_fn_exit;
                        h_cm_exit = h_tp_exit + h_fp_exit + h_fn_exit;
                        h_cm = h_cm_inv + h_cm_entry + h_cm_exit;

                        fn_index = fn_index_inv + fn_index_exit + fn_index_entry;
                        fn_expiry_inv = unique_inventory_fn - fn_index_inv;
                        fn_expiry_entry = fn_entry - fn_index_entry;
                        fn_expiry_exit = fn_exit - fn_index_exit;
                        fn_expiry = fn - fn_index;

                        fn_expiry = fn_expiry_entry + fn_expiry_exit + fn_expiry_inv;

                        h_fn_index = h_fn_index_inv + h_fn_index_exit + h_fn_index_entry;
                        h_fn_expiry_inv = h_unique_inventory_fn - h_fn_index_inv;
                        h_fn_expiry_entry = h_fn_entry - h_fn_index_entry;
                        h_fn_expiry_exit = h_fn_exit - h_fn_index_exit;
                        h_fn_expiry = h_fn - h_fn_index;


                        h_unique_fp = h_unique_inventory_fp + h_fp_entry + h_fp_exit;
                        h_unique_fn = h_unique_inventory_fn + h_fn_entry + h_fn_exit;

                        last_debug_time = mempool_time;
                        hour++;
                        debug_cumulative << hour << "," << (start_timestamp + 3600 * hour)
                                         << "," << inventory_tx
                                         << "," << cm_inv
                                         << "," << unique_tx
                                         << "," << unique_inventory_fp
                                         << "," << unique_inventory_fn
                                         << "," << tp_inv
                                         << "," << tn_inv
                                         << "," << fp_inv
                                         << "," << fn_inv
                                         << "," << fn_index_inv
                                         << "," << fn_expiry_inv
                                         << "," << float(fp_inv) / float(cm_inv)  //fpr
                                         << "," << float(fn_inv) / float(cm_inv)//fnr
                                         << "," << float(cm_inv - fp_inv) / float(cm_inv) * 100// accuracy
                                         << "," << float(fn_inv) / float(cm_inv) * 100//reprocess

                                         << "," << total_tx
                                         << "," << cm_entry
                                         << "," << entry_tx
                                         << "," << tp_entry + tn_entry + fp_entry + fn_entry
                                         << "," << added_tx
                                         << "," << tp_entry
                                         << "," << tn_entry
                                         << "," << fp_entry
                                         << "," << fn_entry
                                         << "," << fn_index_entry
                                         << "," << fn_expiry_entry
                                         << "," << float(fp_entry) / float(cm_entry) //fpr
                                         << "," << float(fn_entry) / float(cm_entry)//fnr
                                         << "," << float(cm_entry - fp_entry) / float(cm_entry) * 100// accuracy
                                         << "," << float(fn_entry) / float(cm_entry) * 100//reprocess

                                         << "," << exit_tx
                                         << "," << cm_exit
                                         << "," << removed_tx
                                         << "," << block_tx
                                         << "," << non_block
                                         << "," << tp_exit
                                         << "," << old_tx
                                         << "," << fp_exit
                                         << "," << fn_exit
                                         << "," << fn_index_exit
                                         << "," << fn_expiry_exit
                                         << "," << fn_index
                                         << "," << fn_expiry
                                         << "," << float(fp_exit) / float(cm_exit) //fpr
                                         << "," << float(fn_exit) / float(cm_exit) //fnr
                                         << "," << float(cm_exit - fp_exit) / float(cm_exit) * 100// accuracy
                                         << "," << float(fn_exit) / float(cm_exit) * 100//reprocess

                                         << "," << float(fp) / float(cm)  //fpr
                                         << "," << float(fn) / float(cm) //fnr
                                         << "," << float(cm - fp_inv - fp_entry) / float(cm) * 100// accuracy
                                         << "," << float(fn_inv + fn_entry) / float(cm_inv + cm_entry) * 100//reprocess

                                         << "," << current_mempool_tx
                                         << "," << shadow_mempool.size()
                                         << "," << (fp_entry - fn_entry + fp_exit - fn_exit)
                                         << "," << non_block
                                         << ","
                                         << current_mempool_tx - non_block + (fp_entry - fn_entry + fp_exit - fn_exit)
                                         << "," << current_mempool_tx - shadow_mempool.size() +
                                                   (fp_entry - fn_entry + fp_exit - fn_exit)
                                         << "," << active_filter
                                         << "," << current_mempool_one
                                         << "," << current_mempool_two
                                         << endl;

                        rbf_replaced = total_rbf - rbf_missed;
                        h_rbf_replaced = h_rbf - h_rbf_missed;

                        /*	    debug_ro << hour
                         << "," << mempool_time
                         << "," << ro_count
                         << "," << tp_ro
                         << "," << tn_ro << ","
                                 << fp_ro << "," << fn_ro << "," << h_tp_ro << "," << h_tn_ro << "," << h_fp_ro << ","
                                 << h_fn_ro << endl; */
                        /*     debug_reason << block_tx << "," << evict_tx << "," << reorg_tx << "," << replace_tx << ","
                                          << expire_tx
                                          << "," << conflict_tx << endl; */
                        /*     debug_rbf << total_rbf << "," << rbf_missed << "," << rbf_replaced << ","
                                       << float(rbf_replaced) / float(total_rbf) * 100 << "," << h_rbf << "," << h_rbf_missed
                                       << "," << h_rbf_replaced << "," << float(h_rbf_replaced) / float(h_rbf) * 100 << endl;*/

                        ro_count = 0;

                        debug_hourly << hour << "," << (start_timestamp + 3600 * hour)
                                     << "," << inventory_tx
                                     << "," << h_tp_inv + h_tn_inv + h_fp_inv + h_fn_inv
                                     << "," << h_unique_tx
                                     << "," << h_unique_inventory_fp
                                     << "," << h_unique_inventory_fn
                                     << "," << h_tp_inv
                                     << "," << h_tn_inv
                                     << "," << h_fp_inv
                                     << "," << h_fn_inv
                                     << "," << h_fn_index_inv
                                     << "," << h_fn_expiry_inv
                                     << "," << float(h_fp_inv) / float(h_cm_inv)  //fpr
                                     << "," << float(h_fn_inv) / float(h_cm_inv)//fnr
                                     << "," << float(h_cm_inv - h_fp_inv) / float(h_cm_inv) * 100// accuracy
                                     << "," << float(h_fn_inv) / float(h_cm_inv) * 100//reprocess

                                     << "," << h_total_tx
                                     << "," << cm_entry + cm_exit
                                     << "," << h_entry_tx
                                     << "," << cm_entry
                                     << "," << h_added_tx
                                     << "," << h_tp_entry
                                     << "," << h_tn_entry
                                     << "," << h_fp_entry
                                     << "," << h_fn_entry
                                     << "," << h_fn_index_entry
                                     << "," << h_fn_expiry_entry
                                     << "," << float(h_fp_entry) / float(h_cm_entry) //fpr
                                     << "," << float(h_fn_entry) / float(h_cm_entry)//fnr
                                     << "," << float(h_cm_entry - h_fp_entry) / float(h_cm_entry) * 100// accuracy
                                     << "," << float(h_fn_entry) / float(h_cm_entry) * 100//reprocess

                                     << "," << h_exit_tx
                                     << "," << h_cm_exit
                                     << "," << h_removed_tx
                                     << "," << h_block_tx
                                     << "," << h_non_block
                                     << "," << h_tp_exit
                                     << "," << h_old_tx
                                     << "," << h_fp_exit
                                     << "," << h_fn_exit
                                     << "," << h_fn_index_exit
                                     << "," << h_fn_expiry_exit
                                     << "," << h_fn_index
                                     << "," << h_fn_expiry
                                     << "," << float(h_fp_exit) / float(h_cm_exit) //fpr
                                     << "," << float(h_fn_exit) / float(h_cm_exit) //fnr
                                     << "," << float(h_cm_exit - h_fp_exit) / float(h_cm_exit) * 100// accuracy
                                     << "," << float(h_fn_exit) / float(h_cm_exit) * 100//reprocess


                                     << "," << float(h_fp) / float(h_cm_inv + h_cm_entry + h_cm_exit)  //fpr
                                     << "," << float(h_fn) / float(h_cm_inv + h_cm_entry + h_cm_exit) //fnr
                                     << ","
                                     << float(h_cm - h_fp_inv - h_fp_entry) / float(h_cm) * 100// accuracy
                                     << ","
                                     << float(h_fn_inv + h_fn_entry) / float(h_cm_inv + h_cm_entry) * 100//reprocess
                                     << "," << current_mempool_tx
                                     << "," << shadow_mempool.size()
                                     << endl;

                        h_rbf_missed = 0, h_rbf_replaced = 0, h_rbf = 0, h_tp = 0, h_tn = 0, h_fp = 0, h_fn = 0, h_fp_entry = 0, h_fn_entry = 0, h_tp_entry = 0, h_tn_entry = 0,
                        h_fp_exit = 0, h_fn_exit = 0, h_tp_exit = 0, h_tp_inv = 0, h_tn_inv = 0, h_fp_inv = 0, h_fn_inv = 0, h_tp_ro = 0, h_fp_ro = 0, h_fn_ro = 0, h_tn_ro = 0,
                        h_unique_tx = 0, h_total_tx = 0, h_old_tx = 0, h_entry_tx = 0, h_added_tx = 0, h_entry_tx = 0, h_added_tx = 0, h_exit_tx = 0, h_inventory_tx = 0,
                        h_block_tx = 0, h_removed_tx = 0, h_exit_tx = 0, h_removed_tx = 0, h_old_tx = 0, h_non_block = 0, h_non_block_not_removed = 0, h_fn_index = 0;
                        h_fn_expiry_inv = 0, h_fn_expiry_entry = 0, h_fn_expiry_exit = 0, h_fn_expiry = 0;

                        h_fn_index = 0, h_fn_index_inv = 0, h_fn_index_exit = 0, h_fn_index_entry = 0;
                        h_fn_expiry_inv = 0, h_unique_inventory_fn = 0, h_fn_index_inv = 0;
                        h_fn_expiry_entry = 0, h_fn_entry = 0, h_fn_index_entry = 0;
                        h_fn_expiry_exit = 0, h_fn_exit = 0, h_fn_index_exit = 0;
                        h_fn_expiry = 0, h_fn = 0, h_fn_index = 0;
                        h_cm = 0, h_cm_inv = 0, h_cm_entry = 0, h_cm_exit = 0;

                        h_unique_fp = 0, h_unique_inventory_fp = 0, h_fp_entry = 0, h_fp_exit = 0;
                        h_unique_fn = 0, h_unique_inventory_fn = 0, h_fn_entry = 0, h_fn_exit = 0;
                    }
                }


                if (doc.HasParseError()) {                                      // IF JSON FILE FORMAT HAS ANY SYNTAX ERRORS, THROW AN ERROR
                    std::cout << "Error  : " << doc.GetParseError() << '\n'
                              << "Offset : " << doc.GetErrorOffset() << '\n';
                    return EXIT_FAILURE;
                }
            }

            // close files one by one
            ifs.close();
            ifs.clear();
            doc.Clear();
        }

    return 0;
}
