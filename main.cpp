#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "instruction.h"
#include "hitmiss.h"
#include "parameter.h"
unsigned int iDisk[DISK_SIZE/4];
unsigned int dDisk[DISK_SIZE/4];
unsigned int sp;
unsigned int iSize, dSize;
unsigned int reg[32];
unsigned int PC, cycle;
unsigned int PC_init;
bool pcChanged;
bool error_halt;
FILE *snapshot, *report;



TLB iTLB[1030], dTLB[1030];
PageTable iPT[1030], dPT[1030];
Cache iCache[1030], dCache[1030];
Mem iMem[1030], dMem[1030];


int iMemSize, dMemSize;
int iPageSize, dPageSize;
int iCacheSize, iBlockSize, iSetAssoc;
int dCacheSize, dBlockSize, dSetAssoc;

int iTLBn, dTLBn, iPTn, dPTn;
int iCachen, dCachen, iMemn, dMemn;
int iTLBHit, iTLBMiss, iPTHit, iPTMiss, iCacheHit, iCacheMiss;
int dTLBHit, dTLBMiss, dPTHit, dPTMiss, dCacheHit, dCacheMiss;

int load_store_times;

bool load_image(void);
void init(void);
void print_cycle(void);
unsigned int convert(unsigned char*);
void print_report(void);
int main(int argc, char** argv)
{

	iMemSize   = argc>1? atoi(argv[1]) : 64;
	dMemSize   = argc>1? atoi(argv[2]) : 32;
	iPageSize  = argc>1? atoi(argv[3]) : 8;
	dPageSize  = argc>1? atoi(argv[4]) : 16;
	iCacheSize = argc>1? atoi(argv[5]) : 16;
	iBlockSize = argc>1? atoi(argv[6]) : 4;
	iSetAssoc  = argc>1? atoi(argv[7]) : 4;
	dCacheSize = argc>1? atoi(argv[8]) : 16;
	dBlockSize = argc>1? atoi(argv[9]) : 4;
	dSetAssoc  = argc>1 ? atoi(argv[10]) : 1;

	iPTn = DISK_SIZE/iPageSize;
	dPTn = DISK_SIZE/dPageSize;
	iTLBn = iPTn/4;
	dTLBn = dPTn/4;
	iCachen = iCacheSize/iBlockSize;
	dCachen = dCacheSize/dBlockSize;
	iMemn = iMemSize/iPageSize;
	dMemn = dMemSize/dPageSize;

    if(!load_image()){
        puts("Cannot load the images");
        return 0;
    }
    init();

    cycle = 0;
    error_halt = false;
    print_cycle();
    int i = PC/4;
    load_store_times = 0;
    while( i < DISK_SIZE/4 ){
       // printf("0x%08x\n",iDisk[i]);
        int opcode = iDisk[i] >> 26;
        pcChanged = false;
        cycle++;//先增值因為error message會用到
       // printf("\nPC:%d\n",PC);
        hit_miss(true, PC, iPageSize, iTLBn, iPTn, iCachen, iMemn, iTLB, iPT, iCache, iMem);
        cacheProcess(true, PC, iPageSize, iBlockSize, iSetAssoc, iTLBn, iCachen, iTLB, iCache);
        //hit_miss()
        //cacheProcess(true, PC, iPageSize, iBlockSize, iSetAssoc, iTLBn, iCachen, iTLB, iCache);
       // i_hit_miss(PC);
       // i_cacheProcess(PC);


		if(opcode==HALT){
			break;
		}

		if(opcode==0x00){
            R_type(iDisk[i]);
        }
        else if(opcode == J || opcode == JAL){
            J_type(iDisk[i]);
        }
        else{
            I_type(iDisk[i]);
        }
        /*if(error_halt){
            break;
        }*/

        if(!pcChanged)
            PC = PC + 4;
        print_cycle();
        //printf("cycle %d  %08x  0x%02x \n",cycle, iDisk[i], opcode);

        //system("pause");

        i = PC/4;
       // printf("i: %d\n",i);
        //system("pause");
    }

    //printf("total load/store: %d\n",load_store_times);
    //printf("iTLBN:%d iPTN:%d\n", iTLBn , iPTn);
    print_report();


    fclose(snapshot);
    fclose(report);

    return 0;
}
void print_report(void)
{
    fprintf(report, "ICache :\n");
    fprintf(report, "# hits: %u\n", iCacheHit );
    fprintf(report, "# misses: %u\n\n", iCacheMiss);
    fprintf(report, "DCache :\n");
    fprintf(report, "# hits: %u\n", dCacheHit );
    fprintf(report, "# misses: %u\n\n", dCacheMiss);
    fprintf(report, "ITLB :\n");
    fprintf(report, "# hits: %u\n", iTLBHit );
    fprintf(report, "# misses: %u\n\n", iTLBMiss);
    fprintf(report, "DTLB :\n");
    fprintf(report, "# hits: %u\n", dTLBHit );
    fprintf(report, "# misses: %u\n\n", dTLBMiss);
    fprintf(report, "IPageTable :\n");
    fprintf(report, "# hits: %u\n", iPTHit);
    fprintf(report, "# misses: %u\n\n", iPTMiss );
    fprintf(report, "DPageTable :\n");
    fprintf(report, "# hits: %u\n", dPTHit);
    fprintf(report, "# misses: %u\n\n", dPTMiss);
}

void init(void)
{
    for(int i=0; i<32; i++)
        reg[i] = ZERO;
    reg[29] = sp;

	for(int i=0; i<1030; i++){
		  iTLB[i].valid =   dTLB[i].valid = false;
		   iPT[i].valid =    dPT[i].valid = false;
		iCache[i].valid = dCache[i].valid = false;
		  iMem[i].valid =   dMem[i].valid = false;
	}

	iTLBHit = iTLBMiss = iPTHit = iPTMiss = iCacheHit = iCacheMiss = 0;
	dTLBHit = dTLBMiss = dPTHit = dPTMiss = dCacheHit = dCacheMiss = 0;

    snapshot = fopen("snapshot.rpt", "w");
	report = fopen("report.rpt", "w");
}
void print_cycle(void)
{
    fprintf(snapshot, "cycle %d\n",cycle);
    for(int i=0; i<32; i++)
        fprintf(snapshot, "$%02d: 0x%08X\n",i,reg[i]);
    fprintf(snapshot,"PC: 0x%08X\n\n\n",PC);
}
unsigned int convert(unsigned char bytes[])
{
    return bytes[3] | (bytes[2]<<8) | (bytes[1]<<16) | (bytes[0]<<24);
}
bool load_image(void)
{
    FILE *fi = fopen("iimage.bin", "rb");
    FILE *fd = fopen("dimage.bin", "rb");
    if(!fi || !fd) return false;

    for(int i = 0; i<DISK_SIZE/4; i++)
        iDisk[i] = dDisk[i] = ZERO;

    unsigned char bytes[4];
    int i = 0;
    while( fread(bytes, 4, 1, fi) != 0 ){
        if(i==0){
            PC = convert(bytes);
        }
        else if(i==1){
            iSize = convert(bytes);
        }
        else{
            if(i-2 >= iSize) break;
            iDisk[i-2 + PC/4] = convert(bytes);
           // printf("%08x \n", iDisk[i-2]);
        }

        i++;
    }
   // puts("");

    i = 0;
    while( fread(bytes, 4, 1, fd) != 0 ){
        if(i==0){
            sp = convert(bytes);
        }
        else if(i==1){
            dSize = convert(bytes);
        }
        else{
            if(i-2 >= dSize) break;
            dDisk[i-2] = convert(bytes);
            //printf("%08x \n", dDisk[i-2]);
        }
        i++;
    }
    fclose(fi);
    fclose(fd);
    return true;
}

