#include "contiki.h"
#include "squatix-br.h"
 #define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

static uint16_t slotframe_handle = 0;
static uint16_t channel_offset = 0;
struct tsch_slotframe *sf_allts;

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot)
{
  if(slotframe != NULL) {
    *slotframe = slotframe_handle;
  }
  if(timeslot != NULL) {
    *timeslot = 0;
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
init(uint16_t sf_handle)
{
  slotframe_handle = sf_handle;
  int timeslot=0;
  // channel_offset = slotframe_handle;
  /* Default slotframe: for broadcast or unicast to neighbors we
   * do not have a link to */
  struct tsch_slotframe *sf_allts = tsch_schedule_add_slotframe(slotframe_handle, SQUATIX_COMMON_SHARED_PERIOD);
  // sf_common = tsch_schedule_add_slotframe(slotframe_handle, SQUATIX_COMMON_SHARED_PERIOD);
  // uint16_t timeslot;
  // timeslot = 0 ? 0 : get_node_timeslot(&linkaddr_node_addr);
  for (timeslot=0;timeslot<SQUATIX_COMMON_SHARED_PERIOD;timeslot++){
    tsch_schedule_add_link(sf_allts,
      LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED,
      LINK_TYPE_ADVERTISING, &tsch_broadcast_address,
      timeslot, channel_offset);
  
  }
}
/*---------------------------------------------------------------------------*/
struct squatix_rule all_ts = {
  init,
  NULL,
  // select_packet,
  NULL,
  NULL,
  NULL,
};
