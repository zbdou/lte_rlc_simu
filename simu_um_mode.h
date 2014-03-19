#ifndef _SIMU_UM_MODE_
#define _SIMU_UM_MODE_

#include "list.h"
#include "rlc.h"
#include "log.h"
#include "fastalloc.h"


#define FINISHED 1
#define NOT_FINISHED 0



/* simulation input parameters */
/* @transmitter */
/* @receiver */
/* @radio link */
/* @traffic */
/* @derived */


/* simulation events */
typedef enum {
	SIMU_BEGIN,
	PKT_TX_BEGIN,
	PKT_TX_END,
	PKT_RX_END,
	RLC_T_REORDERING,
	RLC_T_POLL_RX,
	RLC_T_STATUS_PROHIBIT,
	SIMU_END,
} event_type_t;

typedef enum {
	ONGOING,
	RX_OK,
	RX_ERR,
} packet_status_t;

typedef struct {
	dllist_node_t node;			/* must be the first. */
	
	u32 sequence_no;			/* sequence number (derived from rlc seq.) */
	u32 packet_size;			/* in bytes */

	u8* mac_pdu;				/* pointer to the mac pdu */
	u32 mac_pdu_size;

	packet_status_t ptt;			/* the status of the packet */
	
	/* statistics */
	u32 tx_begin_timestamp;
	u32 tx_end_timestamp;
	u32 rx_deliver_timestamp;
	u32 jitter;
} packet_t;




#endif	/* _SIMU_UM_MODE_ */
