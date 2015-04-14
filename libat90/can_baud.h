/**
 * Tue Apr 14 21:23:15 2015
 * This file is machine generated and should not be altered by hand.
 */


#ifndef CAN_BAUD_H
#define CAN_BAUD_H

#if F_CPU == 11059200

#define CAN_BAUDRATE (204800)
#define CAN_PRESCALER (6)
#define CAN_CLKS_PR_BIT (54.0)
#define CAN_TBIT (9)
#define CAN_TSYNS (1)
#define CAN_TPRS (4)
#define CAN_TPH1 (2)
#define CAN_TPH2 (2)
#define CAN_SJW (1)
#define CAN_ERR_RATE (0.0)
#define CANBT1_VALUE ((CAN_PRESCALER-1)<<BRP0)
#define CANBT2_VALUE (((CAN_TPRS-1)<<PRS0) | ((CAN_SJW-1)<<SJW0))
#define CANBT3_VALUE (((CAN_TPH1-1)<<PHS10) | ((CAN_TPH2-1)<<PHS20))

#endif /* #if F_CPU == 1105920 */

#endif /* CAN_BAUD_H */
