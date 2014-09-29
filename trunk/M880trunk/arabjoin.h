#ifndef arajoin_h
#define arajoin_h

typedef struct {
    unsigned short isolated;
    unsigned short final;
    unsigned short medial;
    unsigned short initial;
} arab_t;

extern const arab_t zwj; /* zero-width joiner */
extern const arab_t lamaleph[]; /* lam-aleph ligatures */
extern const arab_t jointable[]; /* lookup table for arabic joining */
typedef int bool;
#define true 1
#define false 0
#define MAX_PATH 260

#endif
