#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parameter.h"
#include "hitmiss.h"
void hit_miss(bool is_i, int VA, int pageSize, int TLBn, int PTn, int Cachen, int Memn, TLB TLB[], PageTable PT[], Cache Cache[], Mem Mem[])
{
    int VPN = VA/pageSize;

    //printf("PC:%2d addr:%2d VPN:%2d psize:%2d\n",PC,VA, VPN, dPageSize);

	int i;
	for(i=0; i<TLBn; i++) {
		if(TLB[i].valid && TLB[i].VPN==VPN) {
			is_i ? iTLBHit++ : dTLBHit++;
			TLB[i].ref = cycle;
			break;
		}
	}
	if(i==TLBn){ //TLB miss
		is_i ? iTLBMiss ++ : dTLBMiss++;
		if(PT[VPN].valid) { //page in memory
			is_i ? iPTHit++ : dPTHit++;
			//update the LRU of memory
			for(i=0; i<Memn; i++) {
                if(Mem[i].valid && Mem[i].VPN==VPN){
                    Mem[i].ref = cycle;
                    break;
                }
			}
			if(i==Memn) {
                puts("mem error");
                printf("%s VA:%d VPN:%d\n", is_i?"imem ":"dmem ", VA, VPN  );
                for(i=0; i<Memn; i++) {
                    printf("PPN:%d v:%d VPN:%d\n", i, Mem[i].valid, Mem[i].VPN);

                }
                puts("");
                for(i=0; i<iPTn; i++) {
                    if(iPT[i].valid)
                    printf("VPN:%d v:%d PPN:%d\n",i,iPT[i].valid,iPT[i].PPN);
                }
			}
		}
		else { //page fault
			is_i ? iPTMiss++ : dPTMiss++;
			//swap
			for(i=0; i<Memn; i++) {
                if(!Mem[i].valid) {
                    Mem[i].valid = true;
                    Mem[i].VPN = VPN;
                    Mem[i].ref = cycle;
                    PT[VPN].valid = true;
                    PT[VPN].PPN = i;
                    break;
                }
			}
			if(i==Memn) { //mem is full => LRU
                int minRef = MAX_INT;
                int mini = 0;
                for(i=0; i<Memn; i++) {
                    if(Mem[i].ref < minRef) {
                        minRef = Mem[i].ref;
                        mini = i;
                    }
                }
                Mem[mini].VPN = VPN;
                Mem[mini].ref = cycle;

                //update PageTable
                for(i=0; i<PTn; i++){
                    if(PT[i].valid && PT[i].PPN==mini) {
                        PT[i].valid = false;
                    }
                }
                PT[VPN].valid = true;
                PT[VPN].PPN = mini;

                //update TLB
                for(i=0; i<TLBn; i++) {
                    if(TLB[i].valid && TLB[i].PPN==mini) {
                        TLB[i].valid = false;
                    }
                }
                //update cache
                for(i=0; i<Cachen; i++) {
                    if(Cache[i].valid && Cache[i].PPN==mini) {
                        Cache[i].valid = false;
                    }
                }
			}
		}

        //update TLB
		for(i=0; i<TLBn; i++) {
			if(!TLB[i].valid){
				TLB[i].valid = true;
				TLB[i].ref = cycle;
				TLB[i].VPN = VPN;
				TLB[i].PPN = PT[VPN].PPN;

				break;
			}
		}
		if(i==TLBn) { //no invalid page in TLB => LRU
			int minRef = MAX_INT;
			int mini = 0;
			for(i=0; i<TLBn; i++) {
				if(TLB[i].ref < minRef) {
					minRef = TLB[i].ref;
					mini = i;
				}
			}
			TLB[mini].ref = cycle;
			TLB[mini].VPN = VPN;
			TLB[mini].PPN = PT[VPN].PPN;

		}

	}
}
void cacheProcess(bool is_i, int VA, int pageSize, int blockSize, int setAssoc, int TLBn, int Cachen, TLB TLB[], Cache Cache[])
{
    int VPN = VA/pageSize;
    int PPN, PA;
    int i;
    //translate from VA to PA
    for(i=0; i<TLBn; i++) {
        /*if(is_i) {
            printf("idx:%d PC:%2d  v:%2d  VPN:%2d  itlb.VPN:%2d \n",i, VA, TLB[i].valid, VPN, TLB[i].VPN);
        }*/
        if(TLB[i].valid && TLB[i].VPN==VPN){
            PPN = TLB[i].PPN; break;
        }
    }
    if(i==TLBn) {
        printf("TLB error VPN:%d VA:%d, pSize:%d\n", VPN, VA, pageSize);
    }
    PA = PPN*pageSize + VA%pageSize;

    int block_addr = PA/blockSize;
	int setN = Cachen/setAssoc;
	int theSet = block_addr % setN;

	for(i=0; i<Cachen; i++) {
		if(i/setAssoc == theSet) {
			//printf("%d %d %d %d\n",i, iCache[i].valid, iCache[i].block_addr, block_addr);
			if(Cache[i].valid && Cache[i].block_addr==block_addr) {
				is_i ? iCacheHit++ : dCacheHit++;
				Cache[i].ref = cycle;
				if(is_i) {
                    printf("iCache VPN:%d PPN:%d\n",VPN, PPN);
                    /*for(i=0; i<Cachen; i++) {
                        printf("%d %d %d",i);
                    }*/

				}
				break;
			}
		}
	}
	if(i==Cachen) { //cache miss
		is_i ? iCacheMiss++ : dCacheMiss++;
		for(i=0; i<Cachen; i++) {
			if(i/setAssoc == theSet && !Cache[i].valid) {
				Cache[i].valid = true;
				Cache[i].ref = cycle;
				Cache[i].block_addr = block_addr;
				Cache[i].PPN = PPN;
				Cache[i].VPN = VPN;
				break;
			}
		}
		if(i==Cachen) { // no invalid entry in the set => LRU method
			int minRef = MAX_INT;
			int mini = 0;
			for(i=0; i<Cachen; i++) {
				if(i/setAssoc == theSet) {
					if(Cache[i].ref < minRef) {
						minRef = Cache[i].ref;
						mini = i;
					}
				}
			}
			Cache[mini].ref = cycle;
			Cache[mini].block_addr = block_addr;
			Cache[mini].PPN = PPN;
            Cache[mini].VPN = VPN;
		}
	}

}
