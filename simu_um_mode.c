/* zbdou
   3/6/2014

   the main simulation file for RLC UM mode.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "list.h"
#include "rlc.h"
#include "log.h"
#include "fastalloc.h"

#include "simu_um_mode.h"

#define MAC_UL_LCID_CCCH 0x01
#define MAC_DL_LCID_CCCH 0x02

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
	u32 t_Reordering;			/* in us */
	u32 UM_Window_Size;
	u32 sn_FieldLength;
} um_mode_paras_t;


typedef struct {
	u32 t_Reordering;
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

	fastalloc_t *g_mem_ptimer_t; /* as the timer event memory pool. */
	fastalloc_t *g_mem_packet_t; /* as the packet_t memory pool */
} simu_paras_t;

/* convert s to ms */
#define S2MS(xs) (1e3*xs)

/* convert ms to us */
#define MS2US(xms) (1e3*xms)


/* current simulation time, in us */
int g_time_elasped_in_us = 0;

int g_is_finished = NOT_FINISHED;

/* packet list used at the sink */
dllist_node_t g_sink_packet_list;


/* forward declaration */
void simu_end_event(void *timer, void* arg1, void* arg2);
void pkt_rx_end_event(void *timer, void* arg1, void* arg2);
void pkt_tx_begin_event(void *timer, void* arg1, void* arg2);
void pkt_tx_end_event(void *timer, void* arg1, void* arg2);

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
	ZLOG_DEBUG("SSSSSSSSSSSSSSSSSSSSk\n");
	packet_t *pktt = sdu->pktt;
	pktt->rx_deliver_timestamp = g_time_elasped_in_us;
	ZLOG_DEBUG("pktt->rx_deliver_timestamp = %d\n", g_time_elasped_in_us);
}

void mac_free_pdu(void *data, void *cookie)
{
	free(cookie);
}


void mac_free_sdu(void *data, void *cookie)
{
	free(data);
}

/* simulation events */
/*
  @arg1 in pointer to the simu_paras_t struct
  @arg2 in event type ( = SIMU_BEGIN)
 */

void simu_begin_event(void *timer, void* arg1, void* arg2)
{
	/* FIXME: mac_free_pdu, mac_free_sdu */

	/*
	  1. init the RLC entity (tx entity / rx entity)
	  2. set the sink function of the rx entity
	  3. generate the tx begin event at time 0?
	  4. put the packet tx begin event into the event timer queue  
	 */
	simu_paras_t *pspt = (simu_paras_t*) arg1;
	assert((event_type_t)arg2 == SIMU_BEGIN);
	
	/* 1. */
	/* @transmitter */
	rlc_um_init(&(pspt->rlc_tx.um_tx), pspt->rlc_paras.ump.sn_FieldLength, \
				pspt->rlc_paras.ump.UM_Window_Size, \
				pspt->rlc_paras.ump.t_Reordering, \
				mac_free_pdu, mac_free_sdu);

	/* @receiver
	   in @free_sdu should be set to NULL, as we re-use
	   the rlc sdu buffer through the entire simulation.
	 */
	rlc_um_init(&(pspt->rlc_rx.um_rx), pspt->rlc_paras.ump.sn_FieldLength, \
				pspt->rlc_paras.ump.UM_Window_Size, \
				pspt->rlc_paras.ump.t_Reordering, \
				mac_free_pdu, NULL);

	/* 2. */
	rlc_um_set_deliv_func(&pspt->rlc_rx.um_rx, sink);


	/* 3. */
	ptimer_t *pkt_tx_begin = (ptimer_t*)FASTALLOC(pspt->g_mem_ptimer_t);
	assert(pkt_tx_begin);
	pkt_tx_begin->duration = 0;
	pkt_tx_begin->onexpired_func = pkt_tx_begin_event;
	pkt_tx_begin->param[0] = (void*) pspt;
	pkt_tx_begin->param[1] = (void*) PKT_TX_BEGIN;

	/* 4. */
	rlc_timer_start(pkt_tx_begin);

	// g_is_finished = FINISHED;
}

/* FIXME:
   arg1? arg2?
*/
void simu_end_event(void *timer, void* arg1, void* arg2)
{
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


u8* rlc_create_sdu(u16 size)
{
	int i;
	u8* buf = (u8*) malloc(size);
	assert (buf);
	for(i = 0; i < size; i++)
		buf[i] = 0xee;
	
	return buf;
}

#pragma pack(1)

typedef struct mac_pdu_subhead
{
	u32 r:2;
	u32 e:1;
	u32 lc_id:5;
	u32 f:1;
	u32 l:15;
}mac_pdu_subhead_t;

#pragma pack()

void *mac_alloc_buf(u32 size)
{
	return malloc(size);
}

/*
  OUT, mac_pdu
  OUT, mac_pdu_size
  OUT, buf = cookie
 */
#define RESERVE_SPACE 100
int mac_build_pdu(rlc_entity_um_t* rlc_um, u8 **mac_pdu, u32 *mac_pdu_size, u8 **buf)
{
	rlc_entity_um_tx_t* rlc_umtx = NULL;
	u8 *pdu, *cookie;
	mac_pdu_subhead_t *mac_subhead = NULL;
	u32 headsize, offset = 0;
	u32 padding = 0;

	u32 final_rlc_pdu_size = 0, rlc_pdu_size = 0;

	*mac_pdu_size = 0;
	rlc_umtx = &(rlc_um->umtx);
	rlc_pdu_size = rlc_um_tx_estimate_pdu_size(rlc_umtx);
	
	if(rlc_pdu_size == 0) return 0;
	cookie = (u8*) mac_alloc_buf(rlc_pdu_size + 1 + RESERVE_SPACE);
	assert(cookie);

	final_rlc_pdu_size = rlc_um_tx_build_pdu(rlc_umtx, cookie + RESERVE_SPACE, rlc_pdu_size);
	if(final_rlc_pdu_size == 0) {
		free(cookie);
		return 0;
	}

	rlc_pdu_size = final_rlc_pdu_size;
	headsize = 1;
	assert(headsize < RESERVE_SPACE);
	pdu = cookie + RESERVE_SPACE - headsize;

	mac_subhead = (mac_pdu_subhead_t *) (pdu + offset);
	mac_subhead->r = 0;
	mac_subhead->e = (padding == 2);
	mac_subhead->lc_id = MAC_UL_LCID_CCCH;
	
	/* no padding */
	offset++;

	*mac_pdu = pdu;
	*mac_pdu_size = rlc_pdu_size + 1;
	*buf = cookie;
	
	return 1;
}


void macrlc_dump_buffer(u8 *buffer, u16 length)
{
	int i;
	
	for(i=0; i<length; i++)
	{
		if(i%16 == 0)
			printf("\n");
		else if(i%8 == 0)
			printf("-- ");
		printf("%02x ", buffer[i]);
	}
	printf("\n");
}

/*
  @arg1 in pointer to the simu_paras_t struct
  @arg2 in event type ( = PKT_TX_BEGIN)
 */
void pkt_tx_begin_event(void *timer, void* arg1, void* arg2)
{
	/*
	  1. get a @packet from the RLC UM entity TX
	  2. set this @packet's tx_begin_timestamp = current simu time
	  3. generate the packet tx end event for this packet
	  4. put this packet tx end event to the timer queue
	 */
	u8 *mac_pdu, *buffer, *new_mac_pdu;
	u32 mac_pdu_size;


	simu_paras_t *pspt = (simu_paras_t*) arg1;
	assert((event_type_t)arg2 == PKT_TX_BEGIN);

	/* 1. */
	/* 2. */
	/* the sdu_ptr is freed within the rlc_encode_sdu() */
	u8* sdu_ptr = rlc_create_sdu(pspt->t.packet_size);
	rlc_um_tx_sdu_enqueue(&(pspt->rlc_tx.um_tx.umtx), sdu_ptr, pspt->t.packet_size, NULL);

	/* call the mac_build_pdu variant */
	if( mac_build_pdu(&(pspt->rlc_tx.um_tx), &mac_pdu, &mac_pdu_size, &buffer) == 0 ) {
		ZLOG_ERR("failed to build MAC PDU\n");
		return;
	}

	/* debug, dump the mac pdu */
	ZLOG_DEBUG("Rx PDU length=%u.\n", mac_pdu_size);
	macrlc_dump_buffer(mac_pdu, mac_pdu_size);

	new_mac_pdu = malloc(mac_pdu_size);
	assert(new_mac_pdu);
	memcpy(new_mac_pdu, mac_pdu, mac_pdu_size);
	free(buffer);

	/* 3. & 4. */
	ptimer_t *pkt_tx_end = (ptimer_t*)FASTALLOC(pspt->g_mem_ptimer_t);
	assert(pkt_tx_end);

	pkt_tx_end->duration = pspt->tx_delay;	/* tx delay */
	pkt_tx_end->onexpired_func = pkt_tx_end_event;
	pkt_tx_end->param[0] = (void*) pspt;

	/* associate this mac pdu (rlc pdu) with the simulation packet info */
	packet_t *pktt = (packet_t*)FASTALLOC(pspt->g_mem_packet_t);
	assert(pktt);

	dllist_init(&(pktt->node));
	pktt->sequence_no = pspt->rlc_tx.um_tx.umtx.VT_US;
	pktt->mac_pdu = new_mac_pdu;
	pktt->mac_pdu_size = mac_pdu_size;
	pktt->ptt = ONGOING;
	pktt->packet_size = pspt->t.packet_size;
	pktt->tx_begin_timestamp = g_time_elasped_in_us;
	pktt->tx_end_timestamp = 0;	/* not valid, in us */
	pktt->rx_deliver_timestamp = 0; /* not valid, in us */
	pktt->jitter = 0;			/* not valid, in us */	
		
	pkt_tx_end->param[1] = (void*) pktt; 

	rlc_timer_start(pkt_tx_end);

	/*
	ZLOG_DEBUG("going to be finished!\n");
	g_is_finished = FINISHED;
	*/
	
	FASTFREE(pspt->g_mem_ptimer_t, timer);
}

/*
  @arg1 in pointer to the simu_paras_t struct
  @arg2 in pointer to the packet_t struct
 */
void pkt_tx_end_event(void *timer, void* arg1, void* arg2)
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
	simu_paras_t *pspt = (simu_paras_t*) arg1;
	packet_t *pktt = (packet_t*) arg2;
	assert(pktt);

	/* 1. update packet_t */
	pktt->tx_end_timestamp = g_time_elasped_in_us;

	/* 2. FIXME */
	u32 tx_interval = 1;		/* us */	

	/* 3. & 4. */
	ptimer_t *pkt_tx_begin = (ptimer_t*)FASTALLOC(pspt->g_mem_ptimer_t);
	assert(pkt_tx_begin);
	pkt_tx_begin->duration = tx_interval;
	pkt_tx_begin->onexpired_func = pkt_tx_begin_event;
	pkt_tx_begin->param[0] = (void*) pspt;
	pkt_tx_begin->param[1] = (void*) PKT_TX_BEGIN;
	
	rlc_timer_start(pkt_tx_begin);

	/* 5. tx throughput update, done at the @receiver side */
	
	/* 6. */
	ptimer_t *pkt_rx_end = (ptimer_t*)FASTALLOC(pspt->g_mem_ptimer_t);
	assert(pkt_rx_end);
	pkt_rx_end->duration = pspt->rl.prop_delay;
	pkt_rx_end->onexpired_func = pkt_rx_end_event;
	pkt_rx_end->param[0] = (void*) pspt;
	pkt_rx_end->param[1] = arg2; /* packet_t struct */
	
	rlc_timer_start(pkt_rx_end);
	/*
	ZLOG_DEBUG("going to be finished!\n");
	g_is_finished = FINISHED;
	*/
	
	FASTFREE(pspt->g_mem_ptimer_t, timer);
}

/* AM mode: TX, t-PollRx */
/* AM mode: RX, t-StatusProhibit */
/* AM mode: RX, t-Reordering */
/* UM mode: RX, t-Reordering */

int mac_process_pdu(rlc_entity_um_rx_t *umrx, u8 *mac_pdu, u32 pdu_len)
{
	u32 lc_id;
	u32 i, l = 0;
	u32 ret = 0;
	u32 n_sdu = 0;
	int left_len = pdu_len;
	u8 *mac_sdu = NULL;
	mac_pdu_subhead_t *subhead_ptr = (mac_pdu_subhead_t *)mac_pdu;
	mac_pdu_subhead_t subhead[32];

	/* parse subhead */
	while(left_len > 0)
	{
		if(subhead_ptr->e == 0)
		{
			subhead[n_sdu].lc_id = subhead_ptr->lc_id;
			subhead[n_sdu].l = left_len-1;
			mac_sdu = (u8 *)subhead_ptr + 1;			//this first SDU pointer
			n_sdu ++;
			left_len = 0;
			break;
		}
		else
		{
			/* this should not happen! */
			ZLOG_ERR("this should not happen!\n");
			assert(0);
		}
	}
	
	if(mac_sdu == NULL || n_sdu == 0 || left_len != 0)
	{
		ZLOG_ERR("mac_sdu=0x%p n_sdu=%u left_len=%d.\n", mac_sdu, n_sdu, left_len);
		return -1;
	}
	
	for(i=0; i<n_sdu; i++)
	{
		lc_id = subhead[i].lc_id;
		l = subhead[i].l;
		
		//dump MAC sub-header
		ZLOG_DEBUG("Rx MAC PDU sub-header: LCID=%u L=%u.\n", lc_id, l);
				
		/* process special LCID */
		switch(lc_id)
		{
			default:		//Identity of the logical channel
			{
				assert(l > 0);
				if(rlc_um_rx_process_pdu(umrx, mac_sdu, l, mac_pdu) != 0)
					ZLOG_WARN("rlc_um_rx_process_pdu() failure.\n");
			}
		}
		//to next SDU
		mac_sdu += l;
	}
	return ret;	
}

/*
  @arg1 in pointer to the simu_paras_t struct
  @arg2 in pointer to the packet_t struct
 */
void pkt_rx_end_event(void *timer, void* arg1, void* arg2)
{
	/*
	  if (BER is on && BER(packet) == discard) {
	     discard this packet
	  } else
	     hand this packet to the RLC RX entity	  
	 */
	simu_paras_t *pspt = (simu_paras_t*) arg1;
	packet_t *pktt = (packet_t*) arg2;
	assert(pktt);


	/* 1. update this packet's statistics */
	pktt->ptt = RX_OK;
	
	/* 2. call mac_process_pdu to handle this packet */
	/* 3. mac pdu is freed in the mac_process_pdu */
	pspt->rlc_rx.um_rx.umrx.pktt = pktt;
	mac_process_pdu(&(pspt->rlc_rx.um_rx.umrx), pktt->mac_pdu, pktt->mac_pdu_size);

	/*
	ZLOG_DEBUG("going to be finished!\n");
	g_is_finished = FINISHED;
	*/
	
	FASTFREE(pspt->g_mem_ptimer_t, timer);	
}


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
	spt.rlc_paras.ump.t_Reordering = MS2US(5); /* 5 ms */
	spt.rlc_paras.ump.UM_Window_Size = 512;	   /* window size */
	spt.rlc_paras.ump.sn_FieldLength = 10;	   /* sn length */

#define PTIMER_MEM_MAX 4096
	spt.g_mem_ptimer_t = fastalloc_create(sizeof(ptimer_t), PTIMER_MEM_MAX, 0, 100);
	assert(spt.g_mem_ptimer_t);

#define PACKET_MEM_MAX (4096*16*16)
	spt.g_mem_packet_t = fastalloc_create(sizeof(packet_t), PACKET_MEM_MAX, 0, 100);
	assert(spt.g_mem_packet_t);
	
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
		.param[1] = (void*) SIMU_BEGIN,
	};

	/* 3. */
	rlc_init();
	rlc_timer_start(&simu_begin_timer);

	/* 4. */
	int step_in_us = 1;
	// while (!g_is_finished && time_elasped_in_us <= MS2US(S2MS(1000))) {
	while (g_is_finished == NOT_FINISHED && g_time_elasped_in_us <= MS2US(S2MS(10)) ) {		
		rlc_timer_push(step_in_us);		/* us */
		g_time_elasped_in_us += step_in_us;
	}
	ZLOG_DEBUG("time_elasped_in_us = %d\n", g_time_elasped_in_us);

	return 0;
}
