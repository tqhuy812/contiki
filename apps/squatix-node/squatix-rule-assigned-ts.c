#include "contiki.h"
#include "squatix-node.h"
 #define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "net/packetbuf.h"
#include "net/ip/ip64-addr.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip.h"
static uint16_t slotframe_handle = 0;
static uint16_t channel_offset = 0;
struct tsch_slotframe *sf_assignedts;



static uip_ipaddr_t *get_default_router_ipaddr(void)
{
  uip_ipaddr_t *parent_node_ipaddr;
  uip_ds6_defrt_t *default_route;

  default_route = uip_ds6_defrt_lookup(uip_ds6_defrt_choose());
  if(default_route != NULL) {
    parent_node_ipaddr = &default_route->ipaddr;
    return parent_node_ipaddr;
  }
  else return NULL;
}
/*---------------------------------------------------------------------------*/
static 
char*
uip_ipaddr_printf(const uip_ipaddr_t *addr)
{
#if NETSTACK_CONF_WITH_IPV6
  uint16_t a;
  unsigned int i;
  int f;
  static char full_ipaddr[40];
  memset(full_ipaddr,0,sizeof(full_ipaddr));
  char temp[20];

#endif /* NETSTACK_CONF_WITH_IPV6 */

  if(addr == NULL) {
    sprintf(full_ipaddr, "NULL IP addr");
    return full_ipaddr;
  }
#if NETSTACK_CONF_WITH_IPV6
  if(ip64_addr_is_ipv4_mapped_addr(addr)) {
    // PRINTA("::FFFF:%u.%u.%u.%u", addr->u8[12], addr->u8[13], addr->u8[14], addr->u8[15]);
    sprintf(full_ipaddr,"::FFFF:%u.%u.%u.%u", addr->u8[12], addr->u8[13], addr->u8[14], addr->u8[15]);
  } /* END OF (ip64_addr_is_ipv4_mapped_addr(addr)) */

  else {
    for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
      a = (addr->u8[i] << 8) + addr->u8[i + 1];
     if(a == 0 && f >= 0) {
        if(f++ == 0) {
          continue; //Add this line to omit "::" in the addr in the payload
          //PRINTA("::");
          strcpy(temp,full_ipaddr);
          sprintf(full_ipaddr,"%s::",temp);
        } /* END OF (f++ == 0) */
      } /* END OF (a == 0 && f >= 0) */
      else {
        if (a != UIP_DS6_DEFAULT_PREFIX){ //Add this line to omit the prefix "fe80" in the addr in the payload
          if(f > 0) {
            f = -1;
          } 
          else {
            if(i > 0) {
              // PRINTA(":");
              strcpy(temp,full_ipaddr);
              sprintf(full_ipaddr,"%s:",temp);
            }
          }
          strcpy(temp,full_ipaddr);
          sprintf(full_ipaddr,"%s%x",temp,a);
        } //Add this line to omit the prefix "fe80" in the addr in the payload
      } /* END OF else */
        
    }
  }
return full_ipaddr;
#else /* NETSTACK_CONF_WITH_IPV6 */
  // PRINTA("%u.%u.%u.%u", addr->u8[0], addr->u8[1], addr->u8[2], addr->u8[3]);
  sprintf(full_ipaddr,"%u.%u.%u.%u", addr->u8[0], addr->u8[1], addr->u8[2], addr->u8[3]);
#endif /* NETSTACK_CONF_WITH_IPV6 */
}
/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot)
{
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  const linkaddr_t *src = packetbuf_addr(PACKETBUF_ADDR_SENDER);
  uip_ipaddr_t *test_ipaddr = uip_ds6_nbr_ipaddr_from_lladdr(dest);
  // const linkaddr_t *lladdr = rpl_get_parent_lladdr(parent); //========================
  if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_DATAFRAME   ){
     // && !linkaddr_cmp(dest, &linkaddr_null)) {
    if(slotframe != NULL) {
    *slotframe = slotframe_handle;
  struct tsch_slotframe *sf= tsch_schedule_get_slotframe_by_handle(*slotframe);
  struct tsch_link *link = list_head(sf->links_list);
    while(link!=NULL){
      if((link->link_options==LINK_OPTION_TX) && linkaddr_cmp(dest,&link->addr)) {
        *timeslot = link->timeslot;
      }
      link = list_item_next(link);
    }
    return 1;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(uint16_t sf_handle)
{
  slotframe_handle = sf_handle;
  sf_assignedts = tsch_schedule_add_slotframe(slotframe_handle, SQUATIX_COMMON_SHARED_PERIOD);
}
/*---------------------------------------------------------------------------*/
struct squatix_rule assigned_ts = {
  init,
  NULL,
  select_packet,
  // NULL,
  NULL,
  NULL,
};
