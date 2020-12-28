#include <stdio.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <iomanip>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <ctime>
#include <fstream>

#include "CalcTables.h"
#include "PBN.h"

#include "dll.h"

#define BATCHSIZE 3
#define AVERAGE_SIZE 10

//Trump order: NT-S-H-D-C
const int trump_list[] = {4, 0, 1, 2, 3};
//Player oder: S-E-N-W
const int player_list[] = {1, 0, 3, 2};

ddTableResults SolvePBN(const char *cards) {
    ddTableDealPBN dealpbn;
    ddTableResults results;
    std::string pbnstr = cards;

    memcpy ( dealpbn.cards, cards, pbnstr.length() );

    int err = CalcDDtablePBN(dealpbn, &results);

    return results;
}

std::string ResultsString(ddTableResults results) {
    std::stringstream retval;

    for (int i = 0; i < DDS_STRAINS; ++i)
        {
        for (int j = 0; j < DDS_HANDS; ++j)
        {
            int tricks = results.resTable[trump_list[i]][player_list[j]];

            if (player_list[j] == 1 | player_list[j] == 3)//E or W
                tricks = 13-tricks;

            retval << std::uppercase << std::hex << tricks;
        }
    }

    return retval.str();
}

std::string SolvePBNString(const char *cards) {
    return ResultsString(SolvePBN(cards));
}

int main2(int argc, char** argv)
{
 SetMaxThreads(0);

 if (argc > 2)
 {
    std::cerr << "Please give only one argument for the deal, or zero arguments for interactive mode." << std::endl;
    return -1;
 } else if (argc == 2){
    std::cout << SolvePBNString(argv[1]) << std::endl;
 } else {
    while (true) {
        std::string deal;
        std::getline(std::cin, deal);
        std::cout << SolvePBNString(deal.c_str()) << std::endl;
    }
 }

 return 0;
}

const char ranks[] = {'2','3','4','5','6','7','8','9','T','J','Q','K','A'};


std::string GenerateRandomAnalysis(){

    std::vector<int> deck;
    for (int i=0; i<52; ++i) deck.push_back(i);
    std::random_shuffle ( deck.begin(), deck.end() );

    std::vector<int> hands[4][4];
    for (std::vector<int>::iterator it=deck.begin(); it!=deck.end(); ++it){
        int id = std::distance(deck.begin(), it);
        int val = *it;

        int player = id/13;
        int rank = val%13;
        int suit = val/13;

        hands[player][suit].push_back(rank);
    }
    
    ddTableDeal deal;
    std::stringstream pbn;
    for (int p = 0; p < 4; ++p)
    {
        for (int s = 0; s < 4; ++s)
        {
            std::sort(hands[p][s].begin(), hands[p][s].end());
            int hand = 0;
            for (std::vector<int>::iterator it=hands[p][s].begin(); it!=hands[p][s].end(); ++it){
                pbn << ranks[*it];
                hand |= 4 << *it;
            }
            deal.cards[(p+3)%4][s] = hand;

            if (s<3)
            {
                pbn << '.';
            }
        }
        if (p<3)
        {
            pbn << ' ';
        }
    }

    ddTableResults res;

    int err = CalcDDtable(deal, &res);

    if (err <= 0)
        return "Error";

    return pbn.str()+":"+ResultsString(res);
}

int trumpFilter[] = {0,0,0,0,0};

void GenerateRandomAnalysisPar(std::ofstream& outfile) {
    std::vector<int> deck[BATCHSIZE];
    std::vector<int> hands[BATCHSIZE][AVERAGE_SIZE][4][4]; //updated

    ddTableDeals deal;
    deal.noOfTables = BATCHSIZE*AVERAGE_SIZE; //updated
    std::stringstream pbn[BATCHSIZE][AVERAGE_SIZE]; //updated
    srand(time(0));
    for (int b = 0; b < BATCHSIZE; ++b)
    {

	std::vector<int> ew_hands; 
        for (int i=0; i<52; ++i){
        	deck[b].push_back(i);
        	}
        std::random_shuffle ( deck[b].begin(), deck[b].end() );
	

        for (std::vector<int>::iterator it=deck[b].begin(); it!=deck[b].end(); ++it){
            int id = std::distance(deck[b].begin(), it);
            
            int val = *it;
            int player = id/13;
            int rank = val%13;
            int suit = val/13;
            
            // my part
	    if(player==1 || player==3){
	    	for(int a=0; a<AVERAGE_SIZE; ++a) hands[b][a][player][suit].push_back(rank);
	    	}
            else {
            	ew_hands.push_back(val);
            	hands[b][0][player][suit].push_back(rank);
            }
            // my part
        }
        // my part
	for (int a=1; a<AVERAGE_SIZE; ++a){
		std::random_shuffle ( ew_hands.begin(), ew_hands.end() );
		for (std::vector<int>::iterator it=ew_hands.begin(); it!=ew_hands.end(); ++it){
		    int id = std::distance(ew_hands.begin(), it);
		    int val = *it;
		    int player = id/13*2;
		    int rank = val%13;
		    int suit = val/13;
		    hands[b][a][player][suit].push_back(rank);
		    }
	}
	// my part
	for (int a=0; a<AVERAGE_SIZE; ++a){ 
		for (int p = 0; p < 4; ++p)
		{
		    for (int s = 0; s < 4; ++s)
		    {
		        std::sort(hands[b][a][p][s].begin(), hands[b][a][p][s].end());
		        std::reverse(hands[b][a][p][s].begin(), hands[b][a][p][s].end());

		        int hand = 0;
		        for (std::vector<int>::iterator it=hands[b][a][p][s].begin(); it!=hands[b][a][p][s].end(); ++it){
		            pbn[b][a] << ranks[*it];
		            hand |= 4 << *it;
		        }
	
		        deal.deals[b*AVERAGE_SIZE+a].cards[(p+3)%4][s] = hand;

		        if (s<3)
		        {
		            pbn[b][a] << '.';
		        }
		    }
		    if (p<3)
		    {
		        pbn[b][a]<< ' ';
		    }
		}
	}
    }

    ddTablesRes res;
    allParResults apr;

    int err = CalcAllTables(&deal, -1, trumpFilter, &res, &apr);

    if (err <= 0) {
        std::cout << "Error: " << err << std::endl;
        return;
    }

    for (int b = 0; b < BATCHSIZE; ++b)
    {
    	for(int a=0; a < AVERAGE_SIZE ; ++a) { 
        	outfile << pbn[b][a].str()+":"+ResultsString(res.results[b*AVERAGE_SIZE+a])+"\n"; }
    }
    outfile << std::flush;

}

int main(int argc, char** argv)
{
    if (argc > 3 || argc == 0)
    {
        std::cerr << "Please give one arg for the output." << std::endl;
        return -1;
    }

    SetMaxThreads(0);
    std::srand ( unsigned ( std::time(0) ) );

    std::ofstream outfile;
    outfile.open(argv[1]);

    //for (int i = 0; i < 512/BATCHSIZE; ++i)
    //{
    while (true) {
        GenerateRandomAnalysisPar(outfile);
        
    }
    //}

 return 0;
}
