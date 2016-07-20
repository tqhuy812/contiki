/*
 * Routing table resource 
 */

#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "net/ip/ip64-addr.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "simple-udp.h"





static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_routing_info,
         "title=\"Hello world: ?len=0..\";rt=\"Text\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);


/*---------FUNCTION TO CREATE ROUTING TABLE----------------*/
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
static rpl_rank_t get_node_rank(void)
{
  rpl_instance_t *default_instance = rpl_get_default_instance();
  rpl_parent_t *parent = nbr_table_head(rpl_parents);
  while (parent!=NULL){
    if (parent == default_instance->current_dag->preferred_parent){
      printf("AAA");
      return rpl_rank_via_parent(parent);
    }
    printf("BBB");
    parent = nbr_table_next(rpl_parents, parent);
  }
  return INFINITE_RANK;
}
/*---------------------------------------------------------------------------*/
/*-----Return ipaddr of the child node-----*/
// static
// void
// // // uip_ipaddr_t*
// get_child_node_ipaddr(void)  /*======OK======*/
// {
//   uip_ds6_route_t *route;
//   uint8_t i=0;
  
//   route = uip_ds6_route_head();
//   while(route != NULL) {
//     memcpy(&child_node_ipaddr[i],&route->ipaddr,sizeof(uip_ipaddr_t));
//     route = uip_ds6_route_next(route);
//     i++;
//   }
//   // // return child_node_ipaddr;
// }
/*-------------------------------------------------------*/
/*-----Return ipaddr of the neighbor node-----*/
// static 
// // // uip_ipaddr_t*
// void
// get_nbr_node_ipaddr(void)   /*======OK======*/
// {
//   uip_ds6_route_t *route;

//   route = uip_ds6_route_head();
//   uint8_t i=0;
//   while(route != NULL) {
//     memcpy(&nbr_node_ipaddr[i],uip_ds6_route_nexthop(route),sizeof(uip_ipaddr_t));
//     route = uip_ds6_route_next(route);
//     i++;
//   }
//   // // return nbr_node_ipaddr;
// }
//===================================================

#define CHUNKS_TOTAL    2050

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  
  // char *message = uip_ipaddr_printf(get_default_router_ipaddr());
    int length = REST_MAX_CHUNK_SIZE + 10;
  char *message = malloc(length);

  // snprintf((char *)buffer,length,"{\"Parent\":\"%s\"}",message);
  // snprintf((char *)buffer,length,"{\"parentAddr\":\"Node\"}");
  // snprintf((char *)buffer,length,"{\"Rank\":%u}",get_node_rank());

  // snprintf(message,length,"{\"Parent\":\"%s\"},",uip_ipaddr_printf(get_default_router_ipaddr()));
  snprintf(message,2,"{");
  snprintf(message+strlen((char *)message),length,"\"Parent\":\"%s\"",uip_ipaddr_printf(get_default_router_ipaddr()));
  snprintf(message+strlen((char *)message),2,",");
  snprintf(message+strlen((char *)message),length,"\"Rank\":%u",get_node_rank());
  snprintf(message+strlen((char *)message),2,"}");
  snprintf((char *)buffer,strlen((char *)message),message);


  REST.set_header_content_type(response, REST.type.APPLICATION_JSON); /* text/plain is the default, hence this option could be omitted. */
  // REST.set_header_etag(response, (uint8_t *)&length, 1);
  // REST.set_response_payload(response, buffer, length);
  REST.set_response_payload(response, buffer, strlen((char *)message));
  free(message);
}
