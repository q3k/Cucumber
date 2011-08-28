#ifndef __APIC_H__
#define __APIC_H__

#include "types.h"

// To write to the IMCR (interrupt mode configuration register), you need two
// IO ports: APIC_IMCR_CTRL, and APIC_IMCR_DATA. First you output APIC_IMCR_SEL
// to _CTRL, and then you can write to _DATA. The possible values are:
//  o 0x00 - Default, bypass the APIC
//  o 0x01 - Pass NMI and 8259A interrupts through the APIC

#define APIC_IMCR_CTRL 0x22
#define APIC_IMCR_DATA 0x23

#define APIC_IMCR_SEL 0x70

void apic_enable_lapic(void);

// The memory mapped structure of the LAPIC
typedef struct {
    struct { u32 __reserved[4]; } __reserved01; // 0x0000
    struct { u32 __reserved[4]; } __reserved02; // 0x0010
    struct {
        u32 __reserved1 : 24,
                     ID :  4,
            __reserved2 :  4;
        u32 __reserved3[3];
    } ID;                                     // 0x0020
    const struct {
        u32     Version :  8,
            __reserved1 :  8,
                 MaxLVT :  8,
            __reserved2 :  8;
        u32 __reserved[3];
    } Version;                                // 0x0030
    struct { u32 __reserved[4]; } __reserved03; // 0x0040
    struct { u32 __reserved[4]; } __reserved04; // 0x0050
    struct { u32 __reserved[4]; } __reserved05; // 0x0060
    struct { u32 __reserved[4]; } __reserved06; // 0x0070
    struct {
        u32    Priority :  8,
            __reserved1 : 24;
        u32 __reserved2[3];
    } TPR;                                    // 0x0080
    const struct {
        u32    Priority :  8,
            __reserved1 : 24;
        u32 __reserved2[3];
    } APR;                                    // 0x0090
    const struct {
        u32    Priority :  8,
            __reserved1 : 24;
        u32 __reserved2[3];
    } PPR;                                    // 0x00A0
    struct {
        u32 EOI;
        u32 __reserved[3];
    } EOI;                                    // 0x00B0
    struct { u32 __reserved[4]; } __reserved07; // 0x00C0
    struct {
        u32 __reserved1 : 24,
            Destination :  8;
        u32 __reserved2[3];
    } LDR;                                    // 0x00D0
    struct {
        u32 __reserved1 : 28,
                  Model :  4;
        u32 __reserved2[3];
    } DFR;                                    // 0x00E0
    struct {
        u32 SpuriousVector :  8,
               APICEnabled :  1,
                  FocusCPU :  1,
               __reserved1 : 22;
        u32 __reserved2[3];
    } SVR;                                    // 0x00F0
    struct {
        u32 Bitfield;
        u32 __reserved[3];
    } ISR[8];                                 // 0x0100
    struct {
        u32 Bitfield;
        u32 __reserved[3];
    } TMR[8];                                 // 0x0180
    union {
        struct {
            u32 SendCSError            :  1,
                ReceiveCSError         :  1,
                SendAcceptError        :  1,
                ReceiveAcceptError     :  1,
                __reserved1            :  1,
                SendIllegalVector      :  1,
                ReceiveIllegalVector   :  1,
                IllegalRegisterAddress :  1,
                __reserved2            : 24;
            u32 __reserved3[3];
        } ErrorBits;
        struct {
            u32 Errors;
            u32 __reserved[3];
        } AllErrors;
    } ESR;                                    // 0x0280
    struct { u32 __reserved[4]; } __reserved08; // 0x0290
    struct { u32 __reserved[4]; } __reserved09; // 0x02A0
    struct { u32 __reserved[4]; } __reserved10; // 0x02B0
    struct { u32 __reserved[4]; } __reserved11; // 0x02C0
    struct { u32 __reserved[4]; } __reserved12; // 0x02D0
    struct { u32 __reserved[4]; } __reserved13; // 0x02E0
    struct { u32 __reserved[4]; } __reserved14; // 0x02F0
    struct {
        u32      Vector :  8,
           DeliveryMode :  3,
        DestinationMode :  1,
         DeliveryStatus :  1,
            __reserved1 :  1,
                  level :  1,
                trigger :  1,
            __reserved2 :  2,
              shorthand :  2,
            __reserved3 : 12;
        u32 __reserved4[3];
    } ICR1;                                   // 0x0300
    struct {
        u32         __reserved1 : 24,
            PhysicalDestination :  4,
                    __reserved2 :  4;
        u32         __reserved3 : 24,
             VirtualDestination :  8;
        u32 __reserved[3];
    } ICR2;                                   // 0x0310
    struct {
        u32      Vector :  8,
            __reserved1 :  4,
         DeliveryStatus :  1,
            __reserved2 :  3,
                   Mask :  1,
              TimerMode :  1,
            __reserved3 : 12;
        u32 __reserved4[3];
    } LVTTimer;                               // 0x0320
    struct {
        u32      Vector :  8,
           DeliveryMode :  3,
            __reserved1 :  1,
         DeliveryStatus :  1,
            __reserved2 :  3,
                   Mask :  1,
            __reserved3 : 15;
        u32 __reserved4[3];
    } LVTThermal;                             // 0x0330
    struct {
        u32      Vector :  8,
           DeliveryMode :  3,
            __reserved1 :  1,
         DeliveryStatus :  1,
            __reserved2 :  3,
                   Mask :  1,
            __reserved3 : 15;
        u32 __reserved4[3];
    } LVTPerformanceCounter;                  // 0x0340
    struct {
        u32      Vector :  8,
           DeliveryMode :  3,
            __reserved1 :  1,
         DeliveryStatus :  1,
               Polarity :  1,
              RemoteIRR :  1,
                Trigger :  1,
                   Mask :  1,
            __reserved2 : 15;
        u32 __reserved3[3];
    } LVTLINT0;                               // 0x0350
    struct {
        u32      Vector :  8,
           DeliveryMode :  3,
            __reserved1 :  1,
         DeliveryStatus :  1,
               Polarity :  1,
              RemoteIRR :  1,
                Trigger :  1,
                   Mask :  1,
            __reserved2 : 15;
        u32 __reserved3[3];
    } LVTLINT1;                               // 0x0360
    struct {
        u32      Vector :  8,
            __reserved1 :  4,
         DeliveryStatus :  1,
            __reserved2 :  3,
                   Mask :  1,
            __reserved3 : 15;
        u32 __reserved4[3];
    } LVTError;                               // 0x0370
    struct {
        u32 InitialCount;
        u32 __reserved[3];
    } TimerICR;                               // 0x0380
    struct {
        u32 CurrentCount;
        u32 __reserved[3];
    } TimerCCR;                               // 0x0390
    struct { u32 __reserved[4]; } __reserved15; // 0x03A0
    struct { u32 __reserved[4]; } __reserved16; // 0x03B0
    struct { u32 __reserved[4]; } __reserved17; // 0x03C0
    struct { u32 __reserved[4]; } __reserved18; // 0x03D0
    struct {
        u32     Divisor : 4,
            __reserved1 : 28;
        u32 __reserved2[3];
    } TimerDCR;                               // 0x3E0
    struct { u32 __reserved[4]; } __reserved19; // 0x3F0
    // phew.    
} __attribute__((packed)) T_APIC_LAPIC;

#endif
