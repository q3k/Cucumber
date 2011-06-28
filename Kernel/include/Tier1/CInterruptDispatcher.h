#ifndef __CINTERRUPT_DISPATCHER__
#define __CINTERRUPT_DISPATCHER__

#include "types.h"
#include "preprocessor_hacks.h"
extern "C" {
    #include "Tier0/interrupts.h"
    #include "Tier0/panic.h"
    #include "Tier0/kstdio.h"
};

// I am going to programmer hell for this

//CInterruptDispatcher StaticDispatcher Declaration
#define CID_SDIS_DEC(n) static void d_Interrupt##n(T_ISR_REGISTERS_ERR R);

//CInterruptDispatcher StaticDispatcher Implementation
#define CID_SDIS_IMP(n) void CInterruptDispatcher::d_Interrupt##n( \
                                                    T_ISR_REGISTERS_ERR R) { \
    if (m_Dispatchers[n] != 0) \
        m_Dispatchers[n]->Dispatch(&R); \
}

//CInterruptDispatcher StaticDispatcher Set
#define CID_SDID_SET(n) m_DispatcherFunctions[n] = (void*)d_Interrupt##n;

//CInterruptsDispatcher StaticDispatcher Function
#define CID_SDIS_FUNC(n) d_Interrupt##n

namespace cb {
    // A class to.. dispatch interrupts?
    class CInterruptDispatcher {
        protected:
            // Interrupt number
            u8 m_Interrupt;
            
            // Enabled?
            bool m_bEnabled;
            
            // Private enable function - call it from the constructor, another
            // public method or whatever
            void Enable(void);
            
            // Same, but disables
            void Disable(void);
        private:     
            // Internal stuff for translating static calls into member functions
            static CInterruptDispatcher *m_Dispatchers[256];
            static bool m_bInitializedStatic;
            static void InitializeStatic(void);
            
            // All the static dispatchers for all the interrupts
            PPHAX_DO256(CID_SDIS_DEC);
        public:
            // Default constructor - doesn't do shit
            CInterruptDispatcher(void);
            
            // The main point - a virtual dispatch function.
            // This receives either a T_ISR_REGISTERS_ERR or a T_ISR_REGISTERS
            // depending on the interrupt - we'll let the deriver decide
            virtual void Dispatch(void *Registers);
            
            // The usual getters...
            u8 GetInterrupt(void);
            bool GetEnabled(void);
            
            //Static stuff...
            static void *m_DispatcherFunctions[256];
    };
};

#endif
