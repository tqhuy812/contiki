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

#define DEBUG DEBUG_NONE
// #define DEBUG DEBUG_PRINT
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


extern resource_t
  // res_routing_table,
  res_routing_info;
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

rest_init_engine();

rest_activate_resource(&res_routing_info, "test/routing info");


/*-------------- Loop to send routing table routinely------------------------*/
  etimer_set(&et, CLOCK_SECOND * 15); //Change to *30 to observe the Routing Table more frequently
  while(1) {
    PROCESS_YIELD_UNTIL(etimer_expired(&et) ); 
    etimer_reset(&et);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
