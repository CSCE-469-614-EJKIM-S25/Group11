#ifndef RRIP_REPL_H_
#define RRIP_REPL_H_

#include "repl_policies.h"

// Static RRIP
class SRRIPReplPolicy : public ReplPolicy {
    protected:
        // add class member variables here
        // uint32_t assoc; // default of rrpvMax = (2 << assoc) - 1
        uint64_t* array;
        uint32_t numLines;
        const uint64_t MAX_RRPV;
        const uint64_t INIT_RRPV; // The replaced block's initial value

    public:
        // add member methods here, refer to repl_policies.h
        explicit SRRIPReplPolicy(uint32_t numLines, uint32_t rrpvMax) : MAX_RRPV(rrpvMax), INIT_RRPV(rrpvMax - 1) {
            array = gm_calloc<uint64_t>(numLines);
            
            // Initializing every array to the rrpvMax to be replaced (set invalid in other words)
            for(uint64_t i = 0; i < numLines; i++){
                array[i] = rrpvMax;
            }
        }

        ~SRRIPReplPolicy(){
            gm_free(array);
        }

        void update(uint32_t id, const MemReq* req) {
            // if this array block has not been replaced
            if(array[id] != INIT_RRPV)
                array[id] = 0; //RRPV goes to 0 since it is likely to be re-referenced later
        }

        // (v) replace block and set RRPV to ‘(2^n)-2’
        void replaced(uint32_t id) {
            array[id] = INIT_RRPV;
        }

        template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
            // (i) search for first ‘(2^n)-1’ from left
            // (ii) if ‘(2^n)-1’ found go to step (v) or replaced() function
            while(true){ // don't stop incrementing until '(2^n)-1' is found
                for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
                    uint64_t rrpv = array[*ci];
                    if(rrpv == MAX_RRPV) 
                        return *ci;
                }
                
                // (iii) increment all RRPVs if (2^n)-1 is not found
                // (iv) goto step (i)               
                for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
                    uint64_t rrpv = array[*ci];
                    if(rrpv < MAX_RRPV) 
                        array[*ci]++;
                }
            }
        }

        DECL_RANK_BINDINGS;
};
#endif // RRIP_REPL_H_