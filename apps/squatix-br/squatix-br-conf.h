#ifndef __SQUATIX_CONF_H__
#define __SQUATIX_CONF_H__

#ifdef SQUATIX_CONF_RULES
#define SQUATIX_RULES SQUATIX_CONF_RULES
#else /* SQUATIX_CONF_RULES */
/* A default configuration with:
 * - a sender-based slotframe for EB transmission
 * - a sender-based or receiver-based slotframe for unicast to RPL parents and children
 * - a common shared slotframe for any other traffic (mostly broadcast)
 *  */
#define SQUATIX_RULES { &unicast_per_neighbor_rpl_storing, &default_common }
// #define SQUATIX_RULES { &default_common }
/* Example configuration for RPL non-storing mode: */
/* #define SQUATIX_RULES { &eb_per_time_source, &unicast_per_neighbor_rpl_ns, &default_common } */

#endif /* SQUATIX_CONF_RULES */

/* Length of the various slotframes. Tune to balance network capacity,
 * contention, energy, latency. */
#if 0
#ifdef SQUATIX_CONF_EBSF_PERIOD
#define SQUATIX_EBSF_PERIOD                     SQUATIX_CONF_EBSF_PERIOD
#else /* SQUATIX_CONF_EBSF_PERIOD */
#define SQUATIX_EBSF_PERIOD                     7
#endif /* SQUATIX_CONF_EBSF_PERIOD */
#endif

#ifdef SQUATIX_CONF_COMMON_SHARED_PERIOD
#define SQUATIX_COMMON_SHARED_PERIOD            SQUATIX_CONF_COMMON_SHARED_PERIOD
#else /* SQUATIX_CONF_COMMON_SHARED_PERIOD */
#define SQUATIX_COMMON_SHARED_PERIOD            7
#endif /* SQUATIX_CONF_COMMON_SHARED_PERIOD */

#ifdef SQUATIX_CONF_UNICAST_PERIOD
#define SQUATIX_UNICAST_PERIOD                  SQUATIX_CONF_UNICAST_PERIOD
#else /* SQUATIX_CONF_UNICAST_PERIOD */
#define SQUATIX_UNICAST_PERIOD                  7
#endif /* SQUATIX_CONF_UNICAST_PERIOD */

/* Is the per-neighbor unicast slotframe sender-based (if not, it is receiver-based).
 * Note: sender-based works only with RPL storing mode as it relies on DAO and
 * routing entries to keep track of children and parents. */
#ifdef SQUATIX_CONF_UNICAST_SENDER_BASED
#define SQUATIX_UNICAST_SENDER_BASED            SQUATIX_CONF_UNICAST_SENDER_BASED
#else // /* SQUATIX_CONF_UNICAST_SENDER_BASED */
#define SQUATIX_UNICAST_SENDER_BASED            1
#endif // /* SQUATIX_CONF_UNICAST_SENDER_BASED */

/* The hash function used to assign timeslot to a given node (based on its link-layer address) */
#ifdef SQUATIX_CONF_LINKADDR_HASH
#define SQUATIX_LINKADDR_HASH                   SQUATIX_CONF_LINKADDR_HASH
#else /* SQUATIX_CONF_LINKADDR_HASH */
#define SQUATIX_LINKADDR_HASH(addr)             ((addr != NULL) ? (addr)->u8[LINKADDR_SIZE - 1] : -1)
#endif /* SQUATIX_CONF_LINKADDR_HASH */

/* The maximum hash */
#ifdef SQUATIX_CONF_MAX_HASH
#define SQUATIX_MAX_HASH                        SQUATIX_CONF_MAX_HASH
#else /* SQUATIX_CONF_MAX_HASH */
#define SQUATIX_MAX_HASH                        0x7fff
#endif /* SQUATIX_CONF_MAX_HASH */

/* Is the "hash" function collision-free? (e.g. it maps to unique node-ids) */
#ifdef SQUATIX_CONF_COLLISION_FREE_HASH
#define SQUATIX_COLLISION_FREE_HASH             SQUATIX_CONF_COLLISION_FREE_HASH
#else /* SQUATIX_CONF_COLLISION_FREE_HASH */
#define SQUATIX_COLLISION_FREE_HASH             0 /* Set to 1 if SQUATIX_LINKADDR_HASH returns unique hashes */
#endif /* SQUATIX_CONF_COLLISION_FREE_HASH */

#endif /* __SQUATIX_CONF_H__ */
