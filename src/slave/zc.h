// zc.h
// zero crossing detection

#ifndef _ZC_H_
#define _ZC_H_

#include <inttypes.h>

typedef struct {
    uint16_t dur;     // cycles between two ZCs (ideal @ mains frequency)
    uint16_t old_dur; // for calibration
    uint8_t deg_dur;  // cycles b/w two firing angles (ideal @ mains freq.)
    uint8_t angle;    // current dmx firing angle (counted backwards!)
} zc_t;

// initialise subsystems used in zc detection
inline void zc_init (void);

// push old value of zc duration towards new value
inline uint16_t zc_calibrate (uint16_t old, uint16_t new);

// 
inline void degree_duration_counter_init (uint8_t deg_dur);

//
inline void degree_duration_counter_stop (void);

//
inline void zc_duration_counter_init (void);

#endif /* _ZC_H_ */
