#include "contiki.h"
#include "squatix-br.h"
 #define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

static uint16_t slotframe_handle = 0;
static uint16_t channel_offset = 0;
// struct tsch_slotframe *sf_common;

#if SQUATIX_EBSF_PERIOD > 0
/* There is a slotframe for EBs, use this slotframe for non-EB traffic only */
#define SQUATIX_COMMON_SHARED_TYPE              LINK_TYPE_NORMAL
#else
/* There is no slotframe for EBs, use this slotframe both EB and non-EB traffic */
// #define SQUATIX_COMMON_SHARED_TYPE              LINK_TYPE_ADVERTISING_ONLY
#define SQUATIX_COMMON_SHARED_TYPE              LINK_TYPE_ADVERTISING
#endif

/*---------------------------------------------------------------------------*/
// static uint16_t
// get_node_timeslot(const linkaddr_t *addr)
// {
//   if(addr != NULL && SQUATIX_UNICAST_PERIOD > 0) {
//     return SQUATIX_LINKADDR_HASH(addr) % SQUATIX_UNICAST_PERIOD;
//   } else {
//     return 0xffff;
//   }
// }
/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot)
{
  // /* We are the default slotframe, select anything */
  // if((slotframe != NULL)) {
  //   if (slotframe != tsch_schedule_get_slotframe_by_handle(1)) {
  //       *slotframe = slotframe_handle;
  //       *timeslot = 0;
  //       PRINTF("HANDLEEEEEEEE33333333");      

  //   }
  //   else{
  //     if (tsch_schedule_get_link_by_handle(sf_common)==NULL){
  //       *slotframe = 0;
  //       *timeslot = get_node_timeslot(&linkaddr_node_addr);
  //       PRINTF("HANDLEEEEEEEE11111");
  //     }
  //     else {
  //       *slotframe = slotframe_handle;
  //       *timeslot = 0;
  //       PRINTF("HANDLEEEEEEEE22222");
  //     }
  //   }
  // }
  // // if(timeslot != NULL) {
  // //   *timeslot = 0;
  // //   PRINTF("HANDLEEEEEEEE22222");
  // // }
  // // else{ 
  // //   *timeslot = get_node_timeslot(&linkaddr_node_addr);
  // //   PRINTF("HANDLEEEEEEEE33333333");
  // // }
  // return 1;
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
  // channel_offset = slotframe_handle;
  /* Default slotframe: for broadcast or unicast to neighbors we
   * do not have a link to */
  struct tsch_slotframe *sf_common = tsch_schedule_add_slotframe(slotframe_handle, SQUATIX_COMMON_SHARED_PERIOD);
  // sf_common = tsch_schedule_add_slotframe(slotframe_handle, SQUATIX_COMMON_SHARED_PERIOD);
  // uint16_t timeslot;
  // timeslot = 0 ? 0 : get_node_timeslot(&linkaddr_node_addr);
  tsch_schedule_add_link(sf_common,
      LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED,
      SQUATIX_COMMON_SHARED_TYPE, &tsch_broadcast_address,
      0, channel_offset);
}
/*---------------------------------------------------------------------------*/
struct squatix_rule default_common = {
  init,
  NULL,
  select_packet,
  NULL,
  NULL,
};
