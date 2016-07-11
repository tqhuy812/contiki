#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/rpl/rpl.h"
#include "simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/netstack.h"
#include "dev/slip.h"
//#include "button-sensor.h"

////////////////////////////////////////////////
#include "rest-engine.h"
#include "net/ip/ip64-addr.h"     // include to call function ip64_addr_is_ipv4_mapped_addr(addr)
///////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// #define DEBUG DEBUG_NONE
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#if WITH_SQUATIX
#include "squatix-br.h"
#endif /* WITH_SQUATIX */

#if WITH_ORCHESTRA
#include "orchestra-br.h"
#endif /* WITH_ORCHESTRA */

static uip_ipaddr_t prefix;
static uint8_t prefix_set;

static  uip_ipaddr_t child_node_ipaddr[UIP_CONF_MAX_ROUTES];
static uip_ipaddr_t nbr_node_ipaddr[NBR_TABLE_CONF_MAX_NEIGHBORS];


// extern resource_t
  // res_routing_table,
  // res_routing_table;
  // res_hello;

PROCESS(border_router_process, "Border router process");
AUTOSTART_PROCESSES(&border_router_process);
/*---------------------------------------------------------------------------*/
// static void
// print_local_addresses(void)
// {
//   int i;
//   uint8_t state;
//   PRINTA("Server IPv6 addresses:\n");
//   for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
//     state = uip_ds6_if.addr_list[i].state;
//     if(uip_ds6_if.addr_list[i].isused &&
//        (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
//       PRINTA(" ");
//       uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
//       PRINTA("\n");
//     }
//   }
// }
/*---------------------------------------------------------------------------*/
static 
char*
uip_ipaddr_printf(const uip_ipaddr_t *addr)
{
#if NETSTACK_CONF_WITH_IPV6
  uint16_t a;
  unsigned int i;
  int f;
  char full_ipaddr[512];
  memset(full_ipaddr,0,sizeof(full_ipaddr));
  char temp[256];
  // memset(temp,0,sizeof(temp));

#endif /* NETSTACK_CONF_WITH_IPV6 */

  if(addr == NULL) {
    // PRINTA("(NULL IP addr)");
    // strcpy(temp,full_ipaddr);
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
          //PRINTA("::");
          strcpy(temp,full_ipaddr);
          sprintf(full_ipaddr,"%s::",temp);
        } /* END OF (f++ == 0) */
      } /* END OF (a == 0 && f >= 0) */
      else {
        
          if(f > 0) {
            f = -1;
          } 
          else 
            if(i > 0) {
              // PRINTA(":");
              strcpy(temp,full_ipaddr);
              sprintf(full_ipaddr,"%s:",temp);
            }
              strcpy(temp,full_ipaddr);
              sprintf(full_ipaddr,"%s%x",temp,a);
        } /* END OF else */

    }
  }
return full_ipaddr;
#else /* NETSTACK_CONF_WITH_IPV6 */
  // PRINTA("%u.%u.%u.%u", addr->u8[0], addr->u8[1], addr->u8[2], addr->u8[3]);
  sprintf(full_ipaddr,"%u.%u.%u.%u", addr->u8[0], addr->u8[1], addr->u8[2], addr->u8[3]);
#endif /* NETSTACK_CONF_WITH_IPV6 */
}
/*-----Return ipaddr of the parent node (default router) of a node-----*/ //====OK====
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
/*-----Return ipaddr of the child node-----*/
static
void
// // uip_ipaddr_t*
get_child_node_ipaddr(void)  /*======OK======*/
{
  uip_ds6_route_t *route;
  uint8_t i=0;
  
  route = uip_ds6_route_head();
  while(route != NULL) {
    // temp_child_node_ipaddr = &route->ipaddr;
    memcpy(&child_node_ipaddr[i],&route->ipaddr,sizeof(uip_ipaddr_t));

    route = uip_ds6_route_next(route);
    i++;
  }
  // // return child_node_ipaddr;
}
/*-------------------------------------------------------*/
/*-----Return ipaddr of the neighbor node-----*/
static 
// // uip_ipaddr_t*
void
get_nbr_node_ipaddr(void)   /*======OK======*/
{
  uip_ds6_route_t *route;

  route = uip_ds6_route_head();
  uint8_t i=0;
  while(route != NULL) {
    memcpy(&nbr_node_ipaddr[i],uip_ds6_route_nexthop(route),sizeof(uip_ipaddr_t));

    route = uip_ds6_route_next(route);
    i++;
  }
  // // return nbr_node_ipaddr;
}
//===================================================

void
request_prefix(void)
{
  /* mess up uip_buf with a dirty request... */
  uip_buf[0] = '?';
  uip_buf[1] = 'P';
  uip_len = 2;
  slip_send();
  uip_clear_buf(); //empty the uip_buf, similar to the code uip_len = 0
}
/*---------------------------------------------------------------------------*/
void
set_prefix_64(uip_ipaddr_t *prefix_64)
{
  rpl_dag_t *dag;
  uip_ipaddr_t ipaddr;
  memcpy(&prefix, prefix_64, 16);
  memcpy(&ipaddr, prefix_64, 16);
  prefix_set = 1;
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
 uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

 dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &ipaddr);
 if(dag != NULL) {
  rpl_set_prefix(dag, &prefix, 64);
  PRINTF("created a new RPL dag\n");
 }
}
/*---------------------------------------------------------------------------*/
// static void
// print_network_status(void)
// {
//   int i;
//   uint8_t state;
//   uip_ds6_defrt_t *default_route;
//   uip_ds6_route_t *route;


//   PRINTA("--- Network status ---\n");
  
//   /* Our IPv6 addresses */
//   PRINTA("- Server IPv6 addresses:\n");
//   for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
//     state = uip_ds6_if.addr_list[i].state;
//     if(uip_ds6_if.addr_list[i].isused &&
//        (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
//       PRINTA("-- ");
//       uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr); //print both fe and fd for border-router.c, print fe first for node.c
//       PRINTA("\n");
//     }
//   }
  
//   /* Our default route */
//   PRINTA("- Default route:\n");
//   default_route = uip_ds6_defrt_lookup(uip_ds6_defrt_choose());
//   if(default_route != NULL) {
//     PRINTA("-- ");
//     uip_debug_ipaddr_print(&default_route->ipaddr); // print fe-local addr
//     PRINTA(" (lifetime: %lu seconds)\n", (unsigned long)default_route->lifetime.interval);
//   } else {
//     PRINTA("-- None\n");
//   }

//   /* Our routing entries */
//   PRINTA("- Routing entries (%u in total):\n", uip_ds6_route_num_routes());
//   route = uip_ds6_route_head();
//   while(route != NULL) {
//     PRINTA("-- ");
//     uip_debug_ipaddr_print(&route->ipaddr); // print fd-global addr
//     PRINTA(" via ");
//     uip_debug_ipaddr_print(uip_ds6_route_nexthop(route)); //print fe-local addr
//     PRINTA(" (lifetime: %lu seconds)\n", (unsigned long)route->state.lifetime);
//     route = uip_ds6_route_next(route); 
//   }
  
//   PRINTA("----------------------\n");
// }
/*---------------------------------------------------------------------------*/
static void
net_init(uip_ipaddr_t *br_prefix)
{
  uip_ipaddr_t global_ipaddr;

  if(br_prefix) { /* We are RPL root. Will be set automatically
                     as TSCH pan coordinator via the tsch-rpl module */
    //If an RDC layer is used, turn it off but still keep the radio on at the root
    NETSTACK_RDC.off(1);
    memcpy(&global_ipaddr, br_prefix, 16);
    uip_ds6_set_addr_iid(&global_ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&global_ipaddr, 0, ADDR_AUTOCONF);
    rpl_set_root(RPL_DEFAULT_INSTANCE, &global_ipaddr);
    rpl_set_prefix(rpl_get_any_dag(), br_prefix, 64);
    rpl_repair_root(RPL_DEFAULT_INSTANCE);
  }

  NETSTACK_MAC.on();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(border_router_process, ev, data)
{
  static struct etimer et;

  
  PROCESS_BEGIN();
  

  static int i=0;


  prefix_set = 0;

  NETSTACK_MAC.off(0); //turn off both the MAC and radio signal
  
  PROCESS_PAUSE();

  PRINTF("RPL-Border router started\n");

  /* Request prefix until it has been received */
  while(!prefix_set) {
    etimer_set(&et, CLOCK_SECOND);
    request_prefix();
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  //Add TSCH schedule here


  net_init(&prefix);

  #if WITH_SQUATIX
  squatix_init();
  #endif /* WITH_SQUATIX */

  // PRINTF("uIP buffer: %u\n", UIP_BUFSIZE);
  // PRINTF("LL header: %u\n", UIP_LLH_LEN);
  // PRINTF("IP+UDP header: %u\n", UIP_IPUDPH_LEN);
  // PRINTF("REST max chunk: %u\n", REST_MAX_CHUNK_SIZE);

// rest_init_engine();

// rest_activate_resource(&res_hello, "test/hello");
// rest_activate_resource(&res_routing_table, "Routing table");
// PRINTF("BEFORRRRRRRRE");

/*-------------- Loop to send routing table routinely------------------------*/
  etimer_set(&et, CLOCK_SECOND * 15); //Change to *30 to observe the Routing Table more frequently
  


  while(1) {
    // print_network_status();

    PROCESS_YIELD_UNTIL(etimer_expired(&et) ); 
                        // || ev == UIP_DS6_NOTIFICATION_DEFRT_ADD 
                        // || ev == UIP_DS6_NOTIFICATION_DEFRT_RM  
                        // || ev == UIP_DS6_NOTIFICATION_ROUTE_ADD 
                        // || ev == UIP_DS6_NOTIFICATION_ROUTE_RM);
    // Add sending_table_function here (e.g res_event.trigger();)

    printf("KKKKKK");
    // memset(child_node_ipaddr,0,sizeof(child_node_ipaddr));
    // get_child_node_ipaddr();
    get_nbr_node_ipaddr();
    // for(i=0;i!=UIP_CONF_MAX_ROUTES;i++){
    for(i=0;i!=NBR_TABLE_CONF_MAX_NEIGHBORS;i++){
      // uip_debug_ipaddr_print(&child_node_ipaddr[i]);
      // printf("%s",uip_ipaddr_printf(&child_node_ipaddr[i]));
      printf("%s",uip_ipaddr_printf(&nbr_node_ipaddr[i]));
    }

    // res_routing_table.trigger();

    etimer_reset(&et);
  }
/*-------------- Loop to send routing table routinely------------------------*/


  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
