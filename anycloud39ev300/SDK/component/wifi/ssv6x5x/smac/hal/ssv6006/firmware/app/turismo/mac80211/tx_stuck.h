#ifndef _TX_STUCK_H_
#define _TX_STUCK_H_


void ResetTxStuckCnt(void);
int StartTxStuckTimer(void);
int StopTxStuckTimer(void);
int tx_stuck_init(void);

#endif /* _TX_STUCK_H_ */

