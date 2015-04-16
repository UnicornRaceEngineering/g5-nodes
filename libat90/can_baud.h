/**
 * Thu Apr 16 19:52:41 2015
 * This file is machine generated and should not be altered by hand.
 */


#ifndef CAN_BAUD_H
#define CAN_BAUD_H


#if CAN_BAUDRATE == 204800

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

#endif /* CAN_BAUDRATE 204800*/

#if CAN_BAUDRATE == 125000

#define CAN_PRESCALER (8)
#define CAN_CLKS_PR_BIT (88.4736)
#define CAN_TBIT (11)
#define CAN_TSYNS (1)
#define CAN_TPRS (5)
#define CAN_TPH1 (3)
#define CAN_TPH2 (2)
#define CAN_SJW (1)
#define CAN_ERR_RATE (0.4736000000000047)
#define CANBT1_VALUE ((CAN_PRESCALER-1)<<BRP0)
#define CANBT2_VALUE (((CAN_TPRS-1)<<PRS0) | ((CAN_SJW-1)<<SJW0))
#define CANBT3_VALUE (((CAN_TPH1-1)<<PHS10) | ((CAN_TPH2-1)<<PHS20))

#endif /* CAN_BAUDRATE 125000*/

#if CAN_BAUDRATE == 250000

#define CAN_PRESCALER (4)
#define CAN_CLKS_PR_BIT (44.2368)
#define CAN_TBIT (11)
#define CAN_TSYNS (1)
#define CAN_TPRS (5)
#define CAN_TPH1 (3)
#define CAN_TPH2 (2)
#define CAN_SJW (1)
#define CAN_ERR_RATE (0.23680000000000234)
#define CANBT1_VALUE ((CAN_PRESCALER-1)<<BRP0)
#define CANBT2_VALUE (((CAN_TPRS-1)<<PRS0) | ((CAN_SJW-1)<<SJW0))
#define CANBT3_VALUE (((CAN_TPH1-1)<<PHS10) | ((CAN_TPH2-1)<<PHS20))

#endif /* CAN_BAUDRATE 250000*/

#if CAN_BAUDRATE == 500000

#define CAN_PRESCALER (2)
#define CAN_CLKS_PR_BIT (22.1184)
#define CAN_TBIT (11)
#define CAN_TSYNS (1)
#define CAN_TPRS (5)
#define CAN_TPH1 (3)
#define CAN_TPH2 (2)
#define CAN_SJW (1)
#define CAN_ERR_RATE (0.11840000000000117)
#define CANBT1_VALUE ((CAN_PRESCALER-1)<<BRP0)
#define CANBT2_VALUE (((CAN_TPRS-1)<<PRS0) | ((CAN_SJW-1)<<SJW0))
#define CANBT3_VALUE (((CAN_TPH1-1)<<PHS10) | ((CAN_TPH2-1)<<PHS20))

#endif /* CAN_BAUDRATE 500000*/

#if CAN_BAUDRATE == 1000000

#define CAN_PRESCALER (1)
#define CAN_CLKS_PR_BIT (11.0592)
#define CAN_TBIT (11)
#define CAN_TSYNS (1)
#define CAN_TPRS (5)
#define CAN_TPH1 (3)
#define CAN_TPH2 (2)
#define CAN_SJW (1)
#define CAN_ERR_RATE (0.059200000000000585)
#define CANBT1_VALUE ((CAN_PRESCALER-1)<<BRP0)
#define CANBT2_VALUE (((CAN_TPRS-1)<<PRS0) | ((CAN_SJW-1)<<SJW0))
#define CANBT3_VALUE (((CAN_TPH1-1)<<PHS10) | ((CAN_TPH2-1)<<PHS20))

#endif /* CAN_BAUDRATE 1000000*/


#endif /* CAN_BAUD_H */
