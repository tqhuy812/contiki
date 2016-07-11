#include "net/netstack.h"
#include "contiki-conf.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-private.h"
#include "net/rpl/rpl-private.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/ip/uip-debug.h"
#include "lib/random.h"
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


// #define DEBUG DEBUG_NONE
#define DEBUG DEBUG_PRINT

#include "net/ip/uip-debug.h"

#include "net/ip/ip64-addr.h"     // include to call function ip64_addr_is_ipv4_mapped_addr(addr)



// static char full_ipaddr[256];


//#define CONFIG_VIA_BUTTON PLATFORM_HAS_BUTTON
//#if CONFIG_VIA_BUTTON
//#include "button-sensor.h"
//#endif /* CONFIG_VIA_BUTTON */


////////////////////////////////////////////////////
// #if PLATFORM_HAS_BUTTON
// #include "dev/button-sensor.h"
// #endif

/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
// extern resource_t
  // res_hello,
//   res_mirror,
//   res_chunks,
//   res_separate,
//   res_push,
//   res_event,
//   res_sub,
//   res_b1_sep_b2;
// #if PLATFORM_HAS_LEDS
// extern resource_t res_leds, res_toggle;
// #endif
// #if PLATFORM_HAS_LIGHT
// #include "dev/light-sensor.h"
// extern resource_t res_light;
// #endif

              /* Start of comment: stop add resource file info to save ROM */

              // #if PLATFORM_HAS_BATTERY
              // #include "dev/battery-sensor.h"
              // extern resource_t res_battery;
              // #endif
              // #if PLATFORM_HAS_TEMPERATURE
              // #include "dev/temperature-sensor.h"
              // extern resource_t res_temperature;
              // #endif

              /* End of comment */

/*
extern resource_t res_battery;
#endif
#if PLATFORM_HAS_RADIO
#include "dev/radio-sensor.h"
extern resource_t res_radio;
#endif
#if PLATFORM_HAS_SHT11
#include "dev/sht11/sht11-sensor.h"
extern resource_t res_sht11;
#endif
*/
///////////////////////////////////////////////////

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "RPL Node");
AUTOSTART_PROCESSES(&node_process);

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
//       uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
//       PRINTA("\n");
//     }
//   }
  /*---------------------------------------------------------------------------*/
  /* Our default route */
//   PRINTA("- Default route:\n");
//   default_route = uip_ds6_defrt_lookup(uip_ds6_defrt_choose());
//   if(default_route != NULL) {
//     PRINTA("-- ");
//     uip_debug_ipaddr_print(&default_route->ipaddr);;
//     PRINTA(" (lifetime: %lu seconds)\n", (unsigned long)default_route->lifetime.interval);
//   } else {
//     PRINTA("-- None\n");
//   }

//   /* Our routing entries */
//   PRINTA("- Routing entries (%u in total):\n", uip_ds6_route_num_routes());
//   route = uip_ds6_route_head();
//   while(route != NULL) {
//     PRINTA("-- ");
//     uip_debug_ipaddr_print(&route->ipaddr);
//     PRINTA(" via ");
//     uip_debug_ipaddr_print(uip_ds6_route_nexthop(route));
//     PRINTA(" (lifetime: %lu seconds)\n", (unsigned long)route->state.lifetime);
//     route = uip_ds6_route_next(route); 
//   }
  
//   PRINTA("----------------------\n");
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

  // #if WITH_ORCHESTRA
  //   orchestra_init();
  // #endif /* WITH_ORCHESTRA */


///////////////////////////////////////////////////////////
 // rest_init_engine();
 // rest_activate_resource(&res_hello, "test/hello");
//  rest_activate_resource(&res_push, "test/push");
  // Activate more resources below
// #if PLATFORM_HAS_LEDS
//   rest_activate_resource(&res_leds, "actuators/leds"); 
//   rest_activate_resource(&res_toggle, "actuators/toggle");
// #endif
// #if PLATFORM_HAS_LIGHT
//   rest_activate_resource(&res_light, "sensors/light"); 
//   SENSORS_ACTIVATE(light_sensor);  
// #endif

  // End of activate resources
///////////////////////////////////////////////////////////


  // PRINTF("NODE_TEST1");
  /* Print out routing tables every minute */
  etimer_set(&et, CLOCK_SECOND * 20);
  // temp_string = uip_ipaddr_printf(get_default_router_ipaddr())  ;
  while(1) {
    // print_network_status();
    printf("NODDDDDEEE");
    printf("%s",uip_ipaddr_printf(get_default_router_ipaddr()));
    PROCESS_YIELD_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
