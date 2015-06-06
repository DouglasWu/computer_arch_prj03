#ifndef PARAMETER_H
#define PARAMETER_H

#define ADD 0x20
#define SUB 0x22
#define AND 0x24
#define OR 0x25
#define XOR 0x26
#define NOR 0x27
#define NAND 0x28
#define SLT 0x2A
#define SLL 0x00
#define SRL 0x02
#define SRA 0x03
#define JR 0x08

#define ADDI 0x08
#define LW 0x23
#define LH 0x21
#define LHU 0x25
#define LB 0x20
#define LBU 0x24
#define SW 0x2B
#define SH 0x29
#define SB 0x28
#define LUI 0x0F
#define ANDI 0x0C
#define ORI 0x0D
#define NORI 0x0E
#define SLTI 0x0A
#define BEQ 0x04
#define BNE 0x05

#define J 0x02
#define JAL 0x03

#define HALT 0x3f
#define ZERO 0x00000000

#define DISK_SIZE 1024
#define WRITE_ZERO 0
#define NUMBER_OVERFLOW 1
#define ADDRESS_OVERFLOW 2
#define MISALIGNMENT 3
#define MAX_INT 2147483647

extern unsigned int iDisk[DISK_SIZE/4];
extern unsigned int dDisk[DISK_SIZE/4];
extern unsigned int sp;
extern unsigned int iSize, dSize;
extern unsigned int reg[32];
extern unsigned int PC, cycle;
extern unsigned int PC_init;
extern bool pcChanged;
extern bool error_halt;
extern FILE *snapshot, *report;
typedef struct tlb{
    bool valid;
    int ref;
    int VPN;
    int PPN;
}TLB;
typedef struct pageTable{
    bool valid;
    int PPN;
}PageTable;
typedef struct cache{
    bool valid;
    int ref;
    int block_addr;
    int VPN;
    int PPN;
}Cache;
typedef struct mem{
    bool valid;
    int ref;
    int VPN;
}Mem;


extern TLB iTLB[1030], dTLB[1030];
extern PageTable iPT[1030], dPT[1030];
extern Cache iCache[1030], dCache[1030];
extern Mem iMem[1030], dMem[1030];


extern int iTLBn, dTLBn, iPTn, dPTn;
extern int iCachen, dCachen, iMemn, dMemn;
extern int iTLBHit, iTLBMiss, iPTHit, iPTMiss, iCacheHit, iCacheMiss;
extern int dTLBHit, dTLBMiss, dPTHit, dPTMiss, dCacheHit, dCacheMiss;

extern int iMemSize, dMemSize;
extern int iPageSize, dPageSize;
extern int iCacheSize, iBlockSize, iSetAssoc;
extern int dCacheSize, dBlockSize, dSetAssoc;

extern int load_store_times;
#endif


