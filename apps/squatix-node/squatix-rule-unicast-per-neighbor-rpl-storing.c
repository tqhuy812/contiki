#include "contiki.h"
#include "squatix-node.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/packetbuf.h"
#include "net/rpl/rpl-conf.h"
#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

// #if SQUATIX_UNICAST_SENDER_BASED && SQUATIX_COLLISION_FREE_HASH
// #define UNICAST_SLOT_SHARED_FLAG    ((SQUATIX_UNICAST_PERIOD < (SQUATIX_MAX_HASH + 1)) ? LINK_OPTION_SHARED : 0)
// #else
#define UNICAST_SLOT_SHARED_FLAG      LINK_OPTION_SHARED
// #endif

static uint16_t slotframe_handle = 0;
static uint16_t channel_offset = 0;
static struct tsch_slotframe *sf_unicast;

/*---------------------------------------------------------------------------*/
static uint16_t
get_node_timeslot(const linkaddr_t *addr)
{
  if(addr != NULL && SQUATIX_UNICAST_PERIOD > 0) {
    PRINTF("IIIIIII: %d", SQUATIX_LINKADDR_HASH(addr) % SQUATIX_UNICAST_PERIOD);
    return SQUATIX_LINKADDR_HASH(addr) % SQUATIX_UNICAST_PERIOD;
  } else {
    return 0xffff;
  }
}
/*---------------------------------------------------------------------------*/
// static int
// neighbor_has_uc_link(const linkaddr_t *linkaddr)
// {
  //PRINTF("parent_has_uc_link");
  // if(linkaddr != NULL && !linkaddr_cmp(linkaddr, &linkaddr_null)) {
    // PRINTF("parent_has_uc_link1");
    // if((squatix_parent_knows_us || !SQUATIX_UNICAST_SENDER_BASED)
    //    && linkaddr_cmp(&squatix_parent_linkaddr, linkaddr)) {
    //   PRINTF("parent_has_uc_link2");
    //   return 1;
    // }
    // if(nbr_table_get_from_lladdr(nbr_routes, (linkaddr_t *)linkaddr) != NULL) {
    //   PRINTF("othernodes_have_uc_link");
    //   return 1;
    // }
//     return 1;
//   }
//   return 0;
// }
/*---------------------------------------------------------------------------*/
static void
add_uc_link(const linkaddr_t *linkaddr)
{
  PRINTF("timeslot_NODE1:");
  if(linkaddr != NULL) {
    uint16_t timeslot = get_node_timeslot(linkaddr);
    PRINTF("timeslot_NODE: %u",timeslot);
    //uint8_t link_options = SQUATIX_UNICAST_SENDER_BASED ? LINK_OPTION_RX : LINK_OPTION_TX | UNICAST_SLOT_SHARED_FLAG;
    uint8_t link_options = LINK_OPTION_RX | LINK_OPTION_TX | UNICAST_SLOT_SHARED_FLAG;
    //PRINTF("WWWWWWWWWWWWWWWW");
    // if(timeslot == get_node_timeslot(&linkaddr_node_addr)) {
    //   // /* This is also our timeslot, add necessary flags */
    //   //PRINTF("EEEEEEEEEEEEEEEEEE");
    //   link_options |= SQUATIX_UNICAST_SENDER_BASED ? LINK_OPTION_TX | UNICAST_SLOT_SHARED_FLAG: LINK_OPTION_RX;
    // }



    /* Add/update link */
    tsch_schedule_add_link(sf_unicast, link_options, LINK_TYPE_NORMAL, &tsch_broadcast_address,
          timeslot, channel_offset);

  }
}
/*---------------------------------------------------------------------------*/
static void
remove_uc_link(const linkaddr_t *linkaddr)
{
  uint16_t timeslot;
  struct tsch_link *l;
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if(linkaddr == NULL) {
    return;
  }

  timeslot = get_node_timeslot(linkaddr);
  l = tsch_schedule_get_link_by_timeslot(sf_unicast, timeslot);
  if(l == NULL) {
    return;
  }
  /* Does our current parent need this timeslot? */
  if(timeslot == get_node_timeslot(&squatix_parent_linkaddr)) {
    /* Yes, this timeslot is being used, return */
    return;
  }
  /* Does any other child need this timeslot?
   * (lookup all route next hops) */
  nbr_table_item_t *item = nbr_table_head(nbr_routes);
  while(item != NULL) {
    linkaddr_t *addr = nbr_table_get_lladdr(nbr_routes, item);
    if(timeslot == get_node_timeslot(addr)) {
      /* Yes, this timeslot is being used, return */
      return;
    }
    item = nbr_table_next(nbr_routes, item);
  }

  /* Do we need this timeslot? */
  if((timeslot == get_node_timeslot(&linkaddr_node_addr)) || (timeslot == get_node_timeslot(dest))) {
  // if(timeslot == get_node_timeslot(&linkaddr_node_addr)) {   
    // /* This is our link, keep it but update the link options */
    //uint8_t link_options = SQUATIX_UNICAST_SENDER_BASED ? LINK_OPTION_TX | UNICAST_SLOT_SHARED_FLAG: LINK_OPTION_RX;
    uint8_t link_options = LINK_OPTION_TX | UNICAST_SLOT_SHARED_FLAG | LINK_OPTION_RX;
    tsch_schedule_add_link(sf_unicast, link_options, LINK_TYPE_NORMAL, &tsch_broadcast_address,
              timeslot, channel_offset);
  } else {
    // /* Remove link */
    tsch_schedule_remove_link(sf_unicast, l);
  }
}
/*---------------------------------------------------------------------------*/
static void
child_added(const linkaddr_t *linkaddr)
{
  add_uc_link(linkaddr);
}
/*---------------------------------------------------------------------------*/
static void
child_removed(const linkaddr_t *linkaddr)
{
  remove_uc_link(linkaddr);
}
/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot)
{
  /* Select data packets we have a unicast link to */
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_DATAFRAME){
     //&& neighbor_has_uc_link(dest)) {
    if(slotframe != NULL) {
      *slotframe = slotframe_handle;
    }
    if(timeslot != NULL) {
      if(get_node_timeslot(dest)!=0){
      // *timeslot = SQUATIX_UNICAST_SENDER_BASED ? get_node_timeslot(&linkaddr_node_addr) : get_node_timeslot(dest);
      // *timeslot = get_node_timeslot(dest);
        *timeslot = get_node_timeslot(dest);
      }
      else *timeslot = get_node_timeslot(&linkaddr_node_addr);
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
new_time_source(const struct tsch_neighbor *old, const struct tsch_neighbor *new)
{
  if(new != old) {
    const linkaddr_t *old_addr = old != NULL ? &old->addr : NULL;
    const linkaddr_t *new_addr = new != NULL ? &new->addr : NULL;
    if(new_addr != NULL) {
      linkaddr_copy(&squatix_parent_linkaddr, new_addr);
    } else {
      linkaddr_copy(&squatix_parent_linkaddr, &linkaddr_null);
    }
    remove_uc_link(old_addr);
    add_uc_link(new_addr);
  }
}
/*---------------------------------------------------------------------------*/
static void
init(uint16_t sf_handle)
{
  slotframe_handle = sf_handle;
  channel_offset = sf_handle;
  //const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  /* Slotframe for unicast transmissions */
  sf_unicast = tsch_schedule_add_slotframe(slotframe_handle, SQUATIX_UNICAST_PERIOD);
  uint16_t timeslot = get_node_timeslot(&linkaddr_node_addr);
  // uint16_t timeslot = get_node_timeslot(dest); //edit
  //PRINTF("yyyyyyyyy: %u ",get_node_timeslot(dest));
  tsch_schedule_add_link(sf_unicast,
            //SQUATIX_UNICAST_SENDER_BASED ? LINK_OPTION_TX | UNICAST_SLOT_SHARED_FLAG: LINK_OPTION_RX,
            LINK_OPTION_TX | UNICAST_SLOT_SHARED_FLAG | LINK_OPTION_RX,
            LINK_TYPE_NORMAL, &tsch_broadcast_address,
            timeslot, channel_offset);
  // tsch_schedule_add_link(sf_unicast,
  //           //SQUATIX_UNICAST_SENDER_BASED ? LINK_OPTION_TX | UNICAST_SLOT_SHARED_FLAG: LINK_OPTION_RX,
  //           LINK_OPTION_TX | UNICAST_SLOT_SHARED_FLAG | LINK_OPTION_RX,
  //           LINK_TYPE_NORMAL, &tsch_broadcast_address,
  //           get_node_timeslot(dest), channel_offset);


}
/*---------------------------------------------------------------------------*/
struct squatix_rule unicast_per_neighbor_rpl_storing = {
  init,
  new_time_source,
  select_packet,
  child_added,
  child_removed,
};