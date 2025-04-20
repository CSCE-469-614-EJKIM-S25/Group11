#ifndef SHIP_REPL_H
#define SHIP_REPL_H

#include "repl_policies.h" 

#define SHCT_ENTRIES (1u << 14) // 16kb unsigned entries
#define MAX_RRPV 3

class SHiPReplPolicy : public ReplPolicy {
struct lineMeta {
    uint32_t sign; //signature 
    bool outcome; 
};
    uint8_t* shct;  // sign. 
    uint8_t* rrpv;  
    uint32_t numLines;
    lineMeta* meta;

    uint32_t pendSign;   //sign. of the incoming line 
    uint8_t pendInsertRrpv; // what RRPV that line should start with
    
public:
    
SHiPReplPolicy(uint32_t numLines): numLines(numLines), pendSign(0), pendInsertRrpv(3) {
    rrpv = gm_calloc<uint8_t>(numLines);
    meta = gm_calloc<lineMeta>(numLines);
    shct = gm_calloc<uint8_t>(SHCT_ENTRIES);

    for (uint32_t i = 0; i < numLines; ++i) 
        rrpv[i] = MAX_RRPV;
};

~SHiPReplPolicy() {
    gm_free(rrpv);
    gm_free(meta);
    gm_free(shct);
};

void update(uint32_t id, const MemReq* req) { 
    if (rrpv[id] != 0)
        rrpv[id] = 0;
    if (meta[id].outcome == false) // if the line was orginal false update it to reused
        meta[id].outcome = true;
}

void replaced(uint32_t id) override {
    uint32_t oldSign = meta[id].sign;
    if (oldSign) {
        if (meta[id].outcome) {  // line was reused
            if (shct[oldSign] < 3) 
                ++shct[oldSign]; // useful 
        } else {  // bad line
            if (shct[oldSign] > 0)       
                --shct[oldSign]; // not useful
        }
    }
    // seed the fresh new inserted line
    rrpv[id] = pendInsertRrpv;
    meta[id].sign = pendSign;
    meta[id].outcome = false;
}

template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
        pendSign = signature(req->lineAddr); // hash the signature from mem addr
        uint8_t counter = shct[pendSign] & 0x3;
        pendInsertRrpv = (counter == 0 ? MAX_RRPV : 2);

        // old srrip, look for rrpv==3; else age & retry
        while (true) {
            for (auto it = cands.begin(); it != cands.end(); it.inc())
                if (rrpv[*it] == MAX_RRPV) 
                    return *it;
            for (auto it = cands.begin(); it != cands.end(); it.inc())
                if (rrpv[*it] < MAX_RRPV) 
                    ++rrpv[*it];
        }
    }

    DECL_RANK_BINDINGS;

private:
    // signature = bits 14…27 of the byte address (16 KB region) 
    static inline uint32_t signature(Address addr) {
        return static_cast<uint32_t>(addr >> 14) & (SHCT_ENTRIES - 1);
    }
    
};

#endif // SHIP_REPL_H