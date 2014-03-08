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

#define FINISHED 1
#define NOT_FINISHED 0
int g_is_finished = NOT_FINISHED;

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

typedef struct {
	u32 packet_size;			/* in bytes */
} traffic_t;

typedef struct {
	u32 link_distance;			/* kilometers */
	u32 prop_delay;				/* in ms */
	u32 link_bandwidth;			/* in kbps */
	u32 per;					/* pakcet error ratio, (per/10)% */
} radio_link_t;

typedef struct {
	u32 t_Reodering;			/* in us */
	u32 UM_Window_Size;
	u32 sn_FieldLength;
} um_mode_paras_t;


typedef struct {
	u32 t_Reodering;
	u32 t_StatusProhibit;
	u32 t_PollRetransmit;

	/* FIXME, other.... */
} am_mode_paras_t;

typedef struct {
	traffic_t t;
	radio_link_t rl;
	u32 tx_delay;				/* derived */

	union {
		um_mode_paras_t ump;
		am_mode_paras_t amp;
	} rlc_paras;
	
	/* @transmitter */
	union {
		rlc_entity_um_t um_tx;
		rlc_entity_am_t am_tx;
	} rlc_tx;

	/* @receiver */
	union {
		rlc_entity_um_t um_rx;
		rlc_entity_am_t am_rx;
	} rlc_rx;
	
} simu_paras_t;

/* convert ms to us */
#define MS2US(xms) (1e3*xms)

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
void simu_begin_event(void *timer, void* arg1, void* arg2)
{
	/* FIXME:
	   arg1? arg2?
	*/
	simu_paras_t *pspt = (simu_paras_t*) arg1;

	ZLOG_DEBUG("pspt. t_Reodering = %d\n", pspt->rlc_paras.ump.t_Reodering);
	

	/*
	  1. initialization the simulation parameters
	  2. set the simulation mode (UM mode or AM mode)
	  3. init the RLC entity (tx entity / rx entity)
	  4. set the sink function of the rx entity
	  5. generate the tx begin event at time 0?
	  6. put the packet tx begin event into the event timer queue  
	 */
	g_is_finished = FINISHED;
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
	g_is_finished = FINISHED;
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


int main (int argc, char *argv[])
{
	/*
	  1. acquire all simulation parameters or use the default values
	  2. generate the simulation begin event @time = 0
	  3. add this event to the timer queue
	  4. while (simulatio not finished) {
	        advance the simu time by 1 time unit (ms)
	     }
	 */
	/* 1. */
	simu_paras_t spt;
	spt.t.packet_size = 100;	/* 100 bytes */
	spt.rl.link_distance = 0;
	spt.rl.prop_delay = MS2US(270);	/* 270 ms */
	spt.rl.link_bandwidth = 1024;
	spt.rl.per = 10;

	spt.tx_delay = ( (1e3 * OCTET *
						   (spt.t.packet_size +
							MAC_HEADER_SIZE +
							PHY_HEADER_SIZE))
						  / spt.rl.link_bandwidth );


	/* rlc params */
	spt.rlc_paras.ump.t_Reodering = MS2US(5); /* 5 ms */
	spt.rlc_paras.ump.UM_Window_Size = 512;
	spt.rlc_paras.ump.sn_FieldLength = 10;
	
	
	/* @transmitter */
	/* @receiver */
	/* leave these to be done at the simulatioin begin event */

	/* init log */
	zlog_default = openzlog(ZLOG_STDOUT);
	ZLOG_DEBUG("pkt size = %d, prop delay = %d, link bandwidth = %d," \
			   "tx delay = %d\n", spt.t.packet_size, spt.rl.prop_delay,
			   spt.rl.link_bandwidth, spt.tx_delay);

	/* 2. */
	/* FIXME: simu time unit: 1us! */
	ptimer_t simu_begin_timer = {
		.duration = 0,
		.onexpired_func = simu_begin_event,
		.param[0] = (void*) &spt,
		.param[1] = 0,
	};

	/* 3. */
	rlc_init();
	rlc_timer_start(&simu_begin_timer);

	/* 4. */
	while (!g_is_finished) {
		rlc_timer_push(1);		/* 1 time unit */
	}

	/* FIXME: fastalloc.c, always think the addr is 32 bits! wrong! */

	ZLOG_DEBUG("size_t = %ld\n", sizeof(size_t));
	
	return 0;
}


