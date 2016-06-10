#include "contiki.h"
#include "squatix-br.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/packetbuf.h"
#include "lib/random.h"

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

static uint16_t slotframe_handle = 0;
static uint16_t channel_offset = 0;

static struct tsch_slotframe *sf_node;

// Because of the One-SF design, use the common shared cell for both EB and non-EB traffic \
// (inteded for RPL packets but not CoAP packets)
#define SQUATIX_ONE_SLOTFRAME_TYPE              LINK_TYPE_ADVERTISING_ONLY

/////////End of declaration/////////////////

/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot)
{
  /* We are the default slotframe, select anything */
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
//  channel_offset = sf_handle;
  /* Default slotframe: for broadcast or unicast to neighbors we
   * do not have a link to */
//  uint16_t *timeslot;
  sf_node = tsch_schedule_add_slotframe(slotframe_handle, SQUATIX_ONE_SLOTFRAME_PERIOD);
  tsch_schedule_add_link(sf_node,
      LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED,
      SQUATIX_ONE_SLOTFRAME_TYPE, &tsch_broadcast_address,
      0, 0);
}

/*---------------------------------------------------------------------------*/
struct squatix_rule one_slotframe = {
  init,
  NULL,
  select_packet,
  NULL,
  NULL,
};

