/*
 * Routing table resource 
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "rest-engine.h"
#include "simple-udp.h"
#include "er-coap.h"
#include "net/mac/tsch/tsch-schedule.h"
 #include "net/mac/tsch/tsch-private.h"
#include "net/ipv6/uip-ds6-route.h"
// #define DEBUG DEBUG_NONE
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "net/ip/uip.h"
#include "net/ip/ip64-addr.h"


#define UIP_DS6_LOCAL_PREFIX 0xfe80

static struct tsch_link *link;
static struct tsch_slotframe *sf_allts;
static struct tsch_slotframe *sf_assignedts;
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



/*====Parse IP addr============================================*/
//The code below belongs to Rosetta's Code

static unsigned int _parseDecimal ( const char** pchCursor )
{
   unsigned int nVal = 0;
   char chNow;
   while ( chNow = **pchCursor, chNow >= '0' && chNow <= '9' )
   {
      //shift digit in
      nVal *= 10;
      nVal += chNow - '0';
 
      ++*pchCursor;
   }
   return nVal;
}
 
static unsigned int _parseHex ( const char** pchCursor )
{
   unsigned int nVal = 0;
   char chNow;
   while ( chNow = **pchCursor & 0x5f, //(collapses case, but mutilates digits)
         (chNow >= ('0'&0x5f) && chNow <= ('9'&0x5f)) || 
         (chNow >= 'A' && chNow <= 'F') 
         )
   {
      unsigned char nybbleValue;
      chNow -= 0x10; //scootch digital values down; hex now offset by x31
      nybbleValue = ( chNow > 9 ? chNow - (0x31-0x0a) : chNow );
      //shift nybble in
      nVal <<= 4;
      nVal += nybbleValue;
 
      ++*pchCursor;
   }
   return nVal;
}
 
//Parse a textual IPv4 or IPv6 address, optionally with port, into a binary
//array (for the address, in network order), and an optionally provided port.
//Also, indicate which of those forms (4 or 6) was parsed.  Return true on
//success.  ppszText must be a nul-terminated ASCII string.  It will be
//updated to point to the character which terminated parsing (so you can carry
//on with other things.  abyAddr must be 16 bytes.  You can provide NULL for
//abyAddr, nPort, bIsIPv6, if you are not interested in any of those
//informations.  If we request port, but there is no port part, then nPort will
//be set to 0.  There may be no whitespace leading or internal (though this may
//be used to terminate a successful parse.
//Note:  the binary address and integer port are in network order.
int ParseIPv4OrIPv6 ( const char** ppszText, 
      unsigned char* abyAddr, int* pnPort, int* pbIsIPv6 )
{
   unsigned char* abyAddrLocal;
   unsigned char abyDummyAddr[16];
 
   //find first colon, dot, and open bracket
   const char* pchColon = strchr ( *ppszText, ':' );
   const char* pchDot = strchr ( *ppszText, '.' );
   const char* pchOpenBracket = strchr ( *ppszText, '[' );
   const char* pchCloseBracket = NULL;
 
 
   //we'll consider this to (probably) be IPv6 if we find an open
   //bracket, or an absence of dots, or if there is a colon, and it
   //precedes any dots that may or may not be there
   int bIsIPv6local = NULL != pchOpenBracket || NULL == pchDot ||
         ( NULL != pchColon && ( NULL == pchDot || pchColon < pchDot ) );
   //OK, now do a little further sanity check our initial guess...
   if ( bIsIPv6local )
   {
      //if open bracket, then must have close bracket that follows somewhere
      pchCloseBracket = strchr ( *ppszText, ']' );
      if ( NULL != pchOpenBracket && ( NULL == pchCloseBracket ||
            pchCloseBracket < pchOpenBracket ) )
         return 0;
   }
   else  //probably ipv4
   {
      //dots must exist, and precede any colons
      if ( NULL == pchDot || ( NULL != pchColon && pchColon < pchDot ) )
         return 0;
   }
 
   //we figured out this much so far....
   if ( NULL != pbIsIPv6 )
      *pbIsIPv6 = bIsIPv6local;
 
   //especially for IPv6 (where we will be decompressing and validating)
   //we really need to have a working buffer even if the caller didn't
   //care about the results.
   abyAddrLocal = abyAddr; //prefer to use the caller's
   if ( NULL == abyAddrLocal )   //but use a dummy if we must
      abyAddrLocal = abyDummyAddr;
 
   //OK, there should be no correctly formed strings which are miscategorized,
   //and now any format errors will be found out as we continue parsing
   //according to plan.
   if ( ! bIsIPv6local )   //try to parse as IPv4
   {
      //4 dotted quad decimal; optional port if there is a colon
      //since there are just 4, and because the last one can be terminated
      //differently, I'm just going to unroll any potential loop.
      unsigned char* pbyAddrCursor = abyAddrLocal;
      unsigned int nVal;
      const char* pszTextBefore = *ppszText;
      nVal =_parseDecimal ( ppszText );         //get first val
      if ( '.' != **ppszText || nVal > 255 || pszTextBefore == *ppszText ) //must be in range and followed by dot and nonempty
         return 0;
      *(pbyAddrCursor++) = (unsigned char) nVal;   //stick it in addr
      ++(*ppszText); //past the dot
 
      pszTextBefore = *ppszText;
      nVal =_parseDecimal ( ppszText );         //get second val
      if ( '.' != **ppszText || nVal > 255 || pszTextBefore == *ppszText )
         return 0;
      *(pbyAddrCursor++) = (unsigned char) nVal;
      ++(*ppszText); //past the dot
 
      pszTextBefore = *ppszText;
      nVal =_parseDecimal ( ppszText );         //get third val
      if ( '.' != **ppszText || nVal > 255 || pszTextBefore == *ppszText )
         return 0;
      *(pbyAddrCursor++) = (unsigned char) nVal;
      ++(*ppszText); //past the dot
 
      pszTextBefore = *ppszText;
      nVal =_parseDecimal ( ppszText );         //get fourth val
      if ( nVal > 255 || pszTextBefore == *ppszText ) //(we can terminate this one in several ways)
         return 0;
      *(pbyAddrCursor++) = (unsigned char) nVal;
 
      if ( ':' == **ppszText && NULL != pnPort )   //have port part, and we want it
      {
         unsigned short usPortNetwork; //save value in network order
         ++(*ppszText); //past the colon
         pszTextBefore = *ppszText;
         nVal =_parseDecimal ( ppszText );
         if ( nVal > 65535 || pszTextBefore == *ppszText )
            return 0;
         ((unsigned char*)&usPortNetwork)[0] = ( nVal & 0xff00 ) >> 8;
         ((unsigned char*)&usPortNetwork)[1] = ( nVal & 0xff );
         *pnPort = usPortNetwork;
         return 1;
      }
      else  //finished just with ip address
      {
         if ( NULL != pnPort )
            *pnPort = 0;   //indicate we have no port part
         return 1;
      }
   }
   else  //try to parse as IPv6
   {
      unsigned char* pbyAddrCursor;
      unsigned char* pbyZerosLoc;
      int bIPv4Detected;
      int nIdx;
      //up to 8 16-bit hex quantities, separated by colons, with at most one
      //empty quantity, acting as a stretchy run of zeroes.  optional port
      //if there are brackets followed by colon and decimal port number.
      //A further form allows an ipv4 dotted quad instead of the last two
      //16-bit quantities, but only if in the ipv4 space ::ffff:x:x .
      if ( NULL != pchOpenBracket ) //start past the open bracket, if it exists
         *ppszText = pchOpenBracket + 1;
      pbyAddrCursor = abyAddrLocal;
      pbyZerosLoc = NULL;  //if we find a 'zero compression' location
      bIPv4Detected = 0;
      for ( nIdx = 0; nIdx < 8; ++nIdx )  //we've got up to 8 of these, so we will use a loop
      {
         const char* pszTextBefore = *ppszText;
         unsigned nVal =_parseHex ( ppszText );    //get value; these are hex
         if ( pszTextBefore == *ppszText )   //if empty, we are zero compressing; note the loc
         {
            if ( NULL != pbyZerosLoc ) //there can be only one!
            {
               //unless it's a terminal empty field, then this is OK, it just means we're done with the host part
               if ( pbyZerosLoc == pbyAddrCursor )
               {
                  --nIdx;
                  break;
               }
               return 0;   //otherwise, it's a format error
            }
            if ( ':' != **ppszText )   //empty field can only be via :
               return 0;
            if ( 0 == nIdx )  //leading zero compression requires an extra peek, and adjustment
            {
               ++(*ppszText);
               if ( ':' != **ppszText )
                  return 0;
            }
 
            pbyZerosLoc = pbyAddrCursor;
            ++(*ppszText);
         }
         else
         {
            if ( '.' == **ppszText )   //special case of ipv4 convenience notation
            {
               //who knows how to parse ipv4?  we do!
               const char* pszTextlocal = pszTextBefore; //back it up
               unsigned char abyAddrlocal[16];
               int bIsIPv6local;
               int bParseResultlocal = ParseIPv4OrIPv6 ( &pszTextlocal, abyAddrlocal, NULL, &bIsIPv6local );
               *ppszText = pszTextlocal;  //success or fail, remember the terminating char
               if ( ! bParseResultlocal || bIsIPv6local )   //must parse and must be ipv4
                  return 0;
               //transfer addrlocal into the present location
               *(pbyAddrCursor++) = abyAddrlocal[0];
               *(pbyAddrCursor++) = abyAddrlocal[1];
               *(pbyAddrCursor++) = abyAddrlocal[2];
               *(pbyAddrCursor++) = abyAddrlocal[3];
               ++nIdx;  //pretend like we took another short, since the ipv4 effectively is two shorts
               bIPv4Detected = 1;   //remember how we got here for further validation later
               break;   //totally done with address
            }
 
            if ( nVal > 65535 )  //must be 16 bit quantity
               return 0;
            *(pbyAddrCursor++) = nVal >> 8;     //transfer in network order
            *(pbyAddrCursor++) = nVal & 0xff;
            if ( ':' == **ppszText )   //typical case inside; carry on
            {
               ++(*ppszText);
            }
            else  //some other terminating character; done with this parsing parts
            {
               break;
            }
         }
      }
 
      //handle any zero compression we found
      if ( NULL != pbyZerosLoc )
      {
         int nHead = (int)( pbyZerosLoc - abyAddrLocal );   //how much before zero compression
         int nTail = nIdx * 2 - (int)( pbyZerosLoc - abyAddrLocal ); //how much after zero compression
         int nZeros = 16 - nTail - nHead;    //how much zeros
         memmove ( &abyAddrLocal[16-nTail], pbyZerosLoc, nTail ); //scootch stuff down
         memset ( pbyZerosLoc, 0, nZeros );     //clear the compressed zeros
      }
 
      //validation of ipv4 subspace ::ffff:x.x
      if ( bIPv4Detected )
      {
         static const unsigned char abyPfx[] = { 0,0, 0,0, 0,0, 0,0, 0,0, 0xff,0xff };
         if ( 0 != memcmp ( abyAddrLocal, abyPfx, sizeof(abyPfx) ) )
            return 0;
      }
 
      //close bracket
      if ( NULL != pchOpenBracket )
      {
         if ( ']' != **ppszText )
            return 0;
         ++(*ppszText);
      }
 
      if ( ':' == **ppszText && NULL != pnPort )   //have port part, and we want it
      {
         const char* pszTextBefore;
         unsigned int nVal;
         unsigned short usPortNetwork; //save value in network order
         ++(*ppszText); //past the colon
         pszTextBefore = *ppszText;
         pszTextBefore = *ppszText;
         nVal =_parseDecimal ( ppszText );
         if ( nVal > 65535 || pszTextBefore == *ppszText )
            return 0;
         ((unsigned char*)&usPortNetwork)[0] = ( nVal & 0xff00 ) >> 8;
         ((unsigned char*)&usPortNetwork)[1] = ( nVal & 0xff );
         *pnPort = usPortNetwork;
         return 1;
      }
      else  //finished just with ip address
      {
         if ( NULL != pnPort )
            *pnPort = 0;   //indicate we have no port part
         return 1;
      }
   }
 
}
 
 
//simple version if we want don't care about knowing how much we ate
// int ParseIPv4OrIPv6_2 ( const char* pszText, 
//       unsigned char* abyAddr, int* pnPort, int* pbIsIPv6 )
// {
//    const char* pszTextLocal = pszText;
//    return ParseIPv4OrIPv6 ( &pszTextLocal, abyAddr, pnPort, pbIsIPv6);
// }


unsigned short htons ( unsigned short us )
{
   return ( ((unsigned char*)&us)[0] << 8 ) + ((unsigned char*)&us)[1];
}
 
// void dumpbin ( unsigned char* pbyBin, int nLen )
// {
//    int i;
//    for ( i = 0; i < nLen; ++i )
//    {
//       printf( "%02x", pbyBin[i] );
//    }
// }
 

static 
uip_ipaddr_t
build_local_ipaddr( const char* pszTest ){
    uip_ipaddr_t addr;
    int i,nLen;
   unsigned char abyAddr[16];
   int bIsIPv6;
   int nPort;
   int bSuccess;
   uint8_t u8[16];
   const char* pszTextCursor = pszTest;
   ParseIPv4OrIPv6 ( &pszTextCursor, abyAddr, &nPort, &bIsIPv6 );
   nLen = bIsIPv6 ? 16 : 4;
   for ( i = 0; i < nLen; ++i )
   {
    u8[i]=abyAddr[i];
   }
   uip_ip6addr_u8(&addr,u8[0],u8[1],u8[2],u8[3],u8[4],u8[5],u8[6],u8[7],u8[8],u8[9],u8[10],u8[11],u8[12],u8[13],u8[14],u8[15]);
   return addr;
}


static void
res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  // static uint16_t slotframe_handle = 0;
  uip_ds6_defrt_t *default_route;
  uip_ds6_route_t *route;

  uip_ipaddr_t local_ipaddr;

  uip_ipaddr_t *nexthop_local_ipaddr;
  uip_ipaddr_t ipaddr_tsch;
  uip_lladdr_t *lladdr_tsch;
  
  char cell_owner_local_ipaddr[40];
  uint16_t channel_offset=0;
  uint16_t timeslot=0;
  uint16_t slotmode=0;
  uint8_t link_option = LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED;
  int a=0,b=0,c=0,isNotMatched=0;
  int i=0;

  const uint8_t *payload;
  REST.get_request_payload(request,&payload);
  // printf("AAA%s",(char *)payload);

  


  
  char *cell_info = strtok((char *)payload,"=,;");  

  sf_assignedts = tsch_schedule_get_slotframe_by_handle(1);


/*==================================================*/

  while(cell_info !=NULL){
    if((i%4)==0) {

      sprintf(cell_owner_local_ipaddr,"%x%s",UIP_DS6_LOCAL_PREFIX,cell_info);
      local_ipaddr = build_local_ipaddr(cell_owner_local_ipaddr);

      default_route = uip_ds6_defrt_lookup(uip_ds6_defrt_choose());
      for(a=0;a<16;a++){
        if(default_route->ipaddr.u8[a]!=local_ipaddr.u8[a]){
          isNotMatched=1;
          break;
        }
      }
      if(!isNotMatched){
        printf("XXXXX");
        uip_ipaddr_copy(&ipaddr_tsch,&default_route->ipaddr);
      }else{
        printf("YYYYY");
        route = uip_ds6_route_head();
        while(route != NULL) {

            for(b=0;b<16;b++){
              isNotMatched=0;
              // printf("PPP%d",uip_ds6_route_nexthop(route)->u8[b]);
              // printf("NNN%d",local_ipaddr.u8[b]);
              if (uip_ds6_route_nexthop(route)->u8[b]!=local_ipaddr.u8[b]){
                isNotMatched=1;
                break;
              }
            }
          if(isNotMatched){
            printf("FFFFF");
            route = uip_ds6_route_next(route);
          }else{
            if(route==NULL){
              printf("KKKKK");
            }else{
              printf("DDDDD%s",uip_ipaddr_printf(uip_ds6_route_nexthop(route)));
              uip_ipaddr_copy(&ipaddr_tsch,uip_ds6_route_nexthop(route));
              break;
            }
        } 
      }   /* end of while */

     }

    lladdr_tsch = uip_ds6_nbr_lladdr_from_ipaddr(&ipaddr_tsch);
    printf("RRRRR");
    net_debug_lladdr_print(lladdr_tsch);
    }
    else if ((i%4)==1){
            timeslot = atoi(cell_info);
          }else if((i%4)==2){
                  channel_offset = atoi(cell_info);
                } else {
                  slotmode = atoi(cell_info);
                  if (slotmode==2){
                    link_option = LINK_OPTION_TX;
                  } else {
                      link_option = (slotmode==3) ? LINK_OPTION_RX : LINK_OPTION_SHARED;
                  }
                    new_link = tsch_schedule_add_link(sf_assignedts,
                    // LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED,
                    link_option,
                    LINK_TYPE_NORMAL,
                    lladdr_tsch,
                    timeslot,
                    0);   
                    // channel_offset);
                }
    i++;
    cell_info = strtok(NULL,"=,;");
    isNotMatched=0;
  }

/*==================================================*/

  sf_allts = tsch_schedule_get_slotframe_by_handle(0);
  // link = list_head(sf_allts->links_list);
  // link = list_item_next(link);
  // while(link!=NULL){
    // tsch_schedule_remove_link(sf_allts,link);
    // link = list_item_next(link);
    // printf("JJJJJJJJ %d",c);
    // c++;
  // }

  for(c=1;c<5;c++){
    tsch_schedule_remove_link_by_timeslot(sf_allts,c);
  }


  printf("ASNNNN: %x.%lx",current_asn.ms1b,current_asn.ls4b);
  if (new_link!=NULL){
    #define TSCH_CONF_WITH_LINK_SELECTOR 1


  if(payload!=NULL){
    REST.set_response_status(response, REST.status.CHANGED);
  }
  }else REST.set_response_status(response,REST.status.NOT_MODIFIED);
}




 

