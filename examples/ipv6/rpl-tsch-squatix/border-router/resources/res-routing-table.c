/*
 * Routing table resource
 */

#include <stdlib.h>
#include "rest-engine.h"
#include "er-coap.h"
#include "er-coap-observe.h"
 #include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/rpl/rpl.h"

#include "net/ip/ip64-addr.h"     // include to call function ip64_addr_is_ipv4_mapped_addr(addr)
#define DEBUG 1
#include "net/ip/uip-debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
// static void res_periodic_handler(void);
// static void res_event_handler(void);
// static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
// static void res_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

// EVENT_RESOURCE(res_routing_table,
// PERIODIC_RESOURCE(res_routing_table,
RESOURCE(res_routing_table,
         "title=\"Routing table...\"",
         res_get_handler,
         NULL,
         NULL,
         // NULL,
         //res_put_handler,
         // 5 * CLOCK_SECOND,
         NULL);
         // res_periodic_handler);
         // res_event_handler);

/*---------FUNCTION TO CREATE ROUTING TABLE----------------*/

// Print the ipaddr into string
char *
uip_ipaddr_print(const uip_ipaddr_t *addr)
{
// #if NETSTACK_CONF_WITH_IPV6
  uint16_t a;
  unsigned int i;
  int f;

  static char full_ipaddr[sizeof(uip_ipaddr_t)];

// #endif /* NETSTACK_CONF_WITH_IPV6 */

  if(addr == NULL) {
    // PRINTA("(NULL IP addr)");
    strcpy(full_ipaddr, "NULL IP addr");
    return full_ipaddr;
  }
// #if NETSTACK_CONF_WITH_IPV6
  if(ip64_addr_is_ipv4_mapped_addr(addr)) {

    // PRINTA("::FFFF:%u.%u.%u.%u", addr->u8[12], addr->u8[13], addr->u8[14], addr->u8[15]);
    sprintf(full_ipaddr,"::FFFF:%u.%u.%u.%u", addr->u8[12], addr->u8[13], addr->u8[14], addr->u8[15]);
    return full_ipaddr;
  } else {
    for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
      a = (addr->u8[i] << 8) + addr->u8[i + 1];
      if(a == 0 && f >= 0) {
        if(f++ == 0) {
          //PRINTA("::");
          sprintf(full_ipaddr, "::");
        }
      } else {
          if(f > 0) {
            f = -1;
          } else if(i > 0) {
            // PRINTA(":");
            sprintf(full_ipaddr,":");
          }
          // PRINTA("%x", a);
          sprintf(full_ipaddr,"%u", a);
      }
    }
  }
  return full_ipaddr;
// #else /* NETSTACK_CONF_WITH_IPV6 */
  // PRINTA("%u.%u.%u.%u", addr->u8[0], addr->u8[1], addr->u8[2], addr->u8[3]);
// #endif /* NETSTACK_CONF_WITH_IPV6 */
}

/*-------------------------------------------------------*/




/*-----Return ipaddr of the parent node (default router) of a node-----*/
static uip_ipaddr_t *get_default_router_ipaddr(void)
{
  uip_ipaddr_t *parent_node_ipaddr;
  uip_ds6_defrt_t *default_route;

  default_route = uip_ds6_defrt_lookup(uip_ds6_defrt_choose());
  // if(default_route != NULL) {
    parent_node_ipaddr = &default_route->ipaddr;
    return parent_node_ipaddr;
    // return &default_route->ipaddr;
  // }
  // else return NULL;
}

/*-----Return ipaddr of the child node-----*/
// static
// uip_ipaddr_t
// get_child_node_ipaddr(void)
// {
//   uip_ds6_route_t *route;
// uip_ipaddr_t *child_node_ipaddr;
//   route = uip_ds6_route_head();
//   while(route != NULL) {
//     child_node_ipaddr = &route->ipaddr;
//     route = uip_ds6_route_next(route); 
//   }
// }

/*-----Return ipaddr of the neighbor node-----*/
// static uip_ipaddr_t
// get_nbr_node_ipaddr(void)
// {
//   uip_ds6_route_t *route;
//   route = uip_ds6_route_head();
//   while(route != NULL) {
//     nbr_node_ipaddr = uip_ds6_route_nexthop(route);
//     route = uip_ds6_route_next(route); 
//   }
// }

/*---------FUNCTION TO CREATE THE ROUTING TABLE----------------*/

// static int32_t event_counter = 0;

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  PRINTF("GET HANDLERRRRRRR");
  
  // REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
  // snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'Parent': %s}", uip_ipaddr_print((uip_ipaddr_t *)get_default_router_ipaddr));  
  // REST.set_response_payload(response, buffer, strlen((char *)buffer));
  // REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "EVENT %lu", event_counter));
    // REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    // snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%s", uip_ipaddr_print((uip_ipaddr_t *)get_default_router_ipaddr));
    // REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
  // REST.set_header_max_age(response, MAX_AGE);
  // REST.set_header_max_age(response, 10);

  // char *len = NULL;
  char *message = uip_ipaddr_print((uip_ipaddr_t *)get_default_router_ipaddr);
  snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%s", message);
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
}

// static void
// // res_periodic_handler(void)
// res_event_handler(void)
// {
//   PRINTF("GET HANDLERRRRRRR");
//   ++event_counter;
//   // if(ev == UIP_DS6_NOTIFICATION_DEFRT_ADD 
//   // || ev == UIP_DS6_NOTIFICATION_DEFRT_RM  
//   // || ev == UIP_DS6_NOTIFICATION_ROUTE_ADD 
//   // || ev == UIP_DS6_NOTIFICATION_ROUTE_RM) {
//       REST.notify_subscribers(&res_routing_table);
//   // }
// }