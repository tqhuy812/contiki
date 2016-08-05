#include "net/netstack.h"
#include "contiki-conf.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-private.h"
#include "net/rpl/rpl-private.h"

#include "net/ip/uip-debug.h"

#include "net/ipv6/uip-ds6-route.h"

// Adding this library to edit the number of maximum routes in a node
// #include "net/ipv6/uip-ds6.h"
// End of Adding this library to edit the number of maximum routes in a node

///////Adding this file to test add_link function//////
#if WITH_SQUATIX
#include "squatix-node.h"
#endif
///////End of Adding this file to test add_link function//////

#include "net/rpl/rpl.h"

#include "node-id.h"
#include "leds.h"
#include "net/ip/uiplib.h"
#include "net/ip/uip-udp-packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


////////////////////////////////////////////////
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
///////////////////////////////////////////////

// #if WITH_ORCHESTRA
// #include "orchestra-node.h"
// #endif /* WITH_ORCHESTRA */


#define DEBUG DEBUG_NONE
// #define DEBUG DEBUG_PRINT

#include "net/ip/uip-debug.h"

#include "net/ip/ip64-addr.h"     // include to call function ip64_addr_is_ipv4_mapped_addr(addr)



// static char full_ipaddr[256];

/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
extern resource_t
  res_cell,
  res_routing_info;
  // res_hello,

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "RPL Node");
AUTOSTART_PROCESSES(&node_process);


/*---------------------------------------------------------------------------*/
static void
net_init(uip_ipaddr_t *br_prefix)
{
  uip_ipaddr_t global_ipaddr;

  if(br_prefix) { /* We are RPL root. Will be set automatically
                     as TSCH pan coordinator via the tsch-rpl module */
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
PROCESS_THREAD(node_process, ev, data)
{
  static struct etimer et;
  // static char *temp_string;
  PROCESS_BEGIN();

  static int is_coordinator = 0;
  static enum { role_6ln, role_6dr, role_6dr_sec } node_role;
  

  node_role = role_6ln;
  
  /* Set node with ID == 1 as coordinator, convenient in Cooja. */


  if(node_id == 1) {
    if(LLSEC802154_ENABLED) {
      node_role = role_6dr_sec;
    } else {
      node_role = role_6dr;
    }
  } else {
    node_role = role_6ln;
  }

  printf("Init: node starting with role %s\n",
      node_role == role_6ln ? "6ln" : (node_role == role_6dr) ? "6dr" : "6dr-sec");

  tsch_set_pan_secured(LLSEC802154_ENABLED && (node_role == role_6dr_sec));
  is_coordinator = node_role > role_6ln;

  if(is_coordinator) {
    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    net_init(&prefix);
  } else {
    net_init(NULL);
  }

  
  //Add TSCH schedule here
  #if WITH_SQUATIX
    squatix_init();
  #endif /* WITH_SQUATIX */




///////////////////////////////////////////////////////////
 rest_init_engine();
  rest_activate_resource(&res_routing_info, "test/routing-info");
  rest_activate_resource(&res_cell, "test/cell");

  etimer_set(&et, CLOCK_SECOND * 60);
  while(1) {
    // squatix_init();
    printf("SCHEEEEEDULE: ");
    tsch_schedule_print();
    PROCESS_YIELD_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
