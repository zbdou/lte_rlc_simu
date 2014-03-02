/* zbdou
   3/6/2014

   the main simulation file for RLC UM mode.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "rlc.h"
#include "log.h"

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

typedef struct {
	dllist_node_t node;			/* must be the first. */
	
	u32 sequence_no;			/* sequence number (derived from rlc seq.) */
	u32 packet_size;			/* in bytes */

	
	/* statistics */
	u32 tx_begin_timestamp;
	u32 tx_end_timestamp;
	u32 rx_deliver_timestamp;
	u32 jitter;
} packet_t;

/* packet list used at the sink */
dllist_node_t g_sink_packet_list;

/* the sink
   set the rlc um rx -> deliv_sdr function pointer to this sink function
 */
void sink(struct rlc_entity_um_rx *umrx, rlc_sdu_t* sdu)
{
	/* FIXME:
	   input is the rlc_sdu_t* sdu, but we want packet_t */

	/*
	  1. update this packet’s end to end delay (received time = the current simulation time)
	  2. put this packet to the received packet list @g_sink_packet_list
	*/
}


/* simulation events */
void simu_begin_event(void *timer, u32 arg1, u32 arg2)
{
	/* FIXME:
	   arg1? arg2?
	*/

	/*
	  1. initialization the simulation parameters
	  2. set the simulation mode (UM mode or AM mode)
	  3. init the RLC entity (tx entity / rx entity)
	  4. set the sink function of the rx entity
	  5. generate the tx begin event at time 0?
	  6. put the packet tx begin event into the event timer queue  
	 */
}

void simu_end_event(void *timer, u32 arg1, u32 arg2)
{
	/* FIXME:
	   arg1? arg2?
	*/
	/*
	  1. output the simulation results {
	       for the sink, calculate the packet loss ratio per second
		   for the sink, calculate the goodput
		   end to end delay averaged by second
		   calculate the jitter
		   output the results
	  }
	  2. set the simulation termination flag to true to quit the main loop	  
	 */
}

void pkt_rx_end_event(void *timer, u32 arg1, u32 arg2)
{
	/*
	  if (BER is on && BER(packet) == discard) {
	     discard this packet
	  } else
	     hand this packet to the RLC RX entity	  
	 */
}

void pkt_tx_begin_event(void *timer, u32 arg1, u32 arg2)
{
	/*
	  1. get a @packet from the RLC UM entity TX
	  2. set this @packet's tx_begin_timestamp = current simu time
	  3. calculate the tx delay
	  4. generate the packet tx end event for this packet
	  5. put this packet tx end event to the timer queue
	 */
}

void pkt_tx_end_event(void *timer, u32 arg1, u32 arg2)
{
	/*
	  1. set this @packet's tx end timestamp = current simu time
	  2. calcuate the tx interval based on the traffic demand
	  3. generate the next packet tx begin event
	       time = cur simu time + packet tx interval
	  4. put the next packet tx begin event to the timer queue
	  5. update the throughput at the @transmitter side
	  6. calcuate *this* packet's propagation delay and generate
	     the corresponding packet rx end event
	  7. put the packet rx end event to the timer queue
	     (time = cur simu time + prop delay)
	 */
}

/* AM mode: TX, t-PollRx */
/* AM mode: RX, t-StatusProhibit */
/* AM mode: RX, t-Reordering */
/* UM mode: RX, t-Reordering */


typedef enum {
	RLC_AM,
	RLC_UM,
} rlc_mode_t;

/* light speed in km */
#define C 3e5

/* mac header size in bytes */
#define MAC_HEADER_SIZE 0

/* phy header size in bytes */
#define PHY_HEADER_SIZE 0

/* convert byte to bits */
#define OCTET 8

int main (int argc, char *argv[])
{
	/*
	  1. acquire all simulation parameters or use the default values
	  2. validation check of parameters
	  3. generate the simulation begin event @time = 0
	  4. add this event to the timer queue
	  5. while (simulatio not finished) {
	        advance the simu time by 1 time unit (ms)
	     }
	 */

	/* 1 */
	/* @traffic */
	u32 packet_size = 100;		/* in bytes */
	
	/* @radio link */
	u32 link_distance = 0;
	u32 prop_delay = 270;		/* ms */
	u32 link_bandwidth = 1024;	/* kbps */
	u32 per = 10;				/* packet error ratio, x/00, i.e. = (x/10)% */

	/* @derived */
	u32 tx_delay = 0;			/* ms */
	
	/* @transmitter */
	/* @receiver */
	
	
	
	
	return 0;
}
