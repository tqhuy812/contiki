/*
 * Routing table resource 
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "rest-engine.h"
#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
// #include "net/ip/ip64-addr.h"
// #include "net/ip/uip.h"
// #include "net/ipv6/uip-ds6.h"
// #include "net/rpl/rpl.h"
// #include "net/rpl/rpl-private.h"
#include "simple-udp.h"
#include "er-coap.h"
#include "net/mac/tsch/tsch-schedule.h"
 #include "net/mac/tsch/tsch-private.h"
 
// #define DEBUG DEBUG_NONE
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

static struct tsch_link *link;
static struct tsch_slotframe *sf;
static struct tsch_link *new_link;

// static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_cell,
         "title=\"Cell:\"",
         // res_get_handler,
         NULL,
         res_post_handler,
         NULL,
         NULL);

static void
res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  // static uint16_t slotframe_handle = 0;
  uint16_t channel_offset = 0;
  uint16_t timeslot=0;
  uint16_t slotmode = 0;
  uint8_t link_option = LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED;

  int i=0;
  int length=45;
  char *cell_info=malloc(length);
  // char *info=malloc(10);
  const uint8_t *payload;
  REST.get_request_payload(request,&payload);


  cell_info = strtok((char *)payload,",;");
  sf = tsch_schedule_get_slotframe_by_handle(0);
  link = list_head(sf->links_list);
  // while(link!=NULL){
  //   tsch_schedule_remove_link(sf,link);
  //   link = list_item_next(link);
  // }

  // struct asn_t asn=current_asn;
  printf("ASNNNN: %x.%lx",current_asn.ms1b,current_asn.ls4b);

  while(cell_info!=NULL){
    if (i==0){
      timeslot = atoi(strdup(cell_info));
    } else{
      cell_info = strtok(NULL,",;");
      timeslot = atoi(strdup(cell_info));
  }
    cell_info = strtok(NULL,",;");
    channel_offset = atoi(strdup(cell_info));
    cell_info = strtok(NULL,",;");
    slotmode = atoi(strdup(cell_info));

    if (slotmode==2){
      link_option = LINK_OPTION_TX;
    } else {
      link_option = (slotmode==3) ? LINK_OPTION_RX : LINK_OPTION_SHARED;
    }


    new_link = tsch_schedule_add_link(sf,
    LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED,
    // link_option,
    // LINK_TYPE_NORMAL,
    LINK_TYPE_ADVERTISING,
    &tsch_broadcast_address,
    timeslot, channel_offset);      
    i++;
  }

  


  if (new_link!=NULL){
    #define TSCH_CONF_WITH_LINK_SELECTOR 1
    REST.set_response_status(response, REST.status.CHANGED);
  }else REST.set_response_status(response,REST.status.NOT_MODIFIED);
}
