/*
 * Copyright (c) 2015, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *         Orchestra header file
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#ifndef __SQUATIX_H__
#define __SQUATIX_H__

#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-conf.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "squatix-br-conf.h"

/* The structure of an Orchestra rule */
struct squatix_rule {
  void (* init)(uint16_t slotframe_handle);
  void (* new_time_source)(const struct tsch_neighbor *old, const struct tsch_neighbor *new);
  int  (* select_packet)(uint16_t *slotframe, uint16_t *timeslot);
  void (* child_added)(const linkaddr_t *addr);
  void (* child_removed)(const linkaddr_t *addr);
};


struct squatix_rule default_common;
struct squatix_rule unicast_per_neighbor_rpl_storing;
// struct squatix_rule unicast_per_neighbor_rpl_ns;
// struct squatix_rule eb_per_time_source;


extern linkaddr_t squatix_parent_linkaddr;
extern int squatix_parent_knows_us;

/* Call from application to start Orchestra */
void squatix_init(void);
/* Callbacks requied for Orchestra to operate */
/* Set with #define TSCH_CALLBACK_PACKET_READY squatix_callback_packet_ready */
void squatix_callback_packet_ready(void);
/* Set with #define TSCH_CALLBACK_NEW_TIME_SOURCE squatix_callback_new_time_source */
void squatix_callback_new_time_source(const struct tsch_neighbor *old, const struct tsch_neighbor *new);
/* Set with #define NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK squatix_callback_child_added */
void squatix_callback_child_added(const linkaddr_t *addr);
/* Set with #define NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK squatix_callback_child_removed */
void squatix_callback_child_removed(const linkaddr_t *addr);

#endif /* __SQUATIX_H__ */
