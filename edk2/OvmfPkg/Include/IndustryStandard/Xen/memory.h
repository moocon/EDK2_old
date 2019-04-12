/******************************************************************************
 * memory.h
 * 
 * Memory reservation and information.
 * 
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2005, Keir Fraser <keir@xensource.com>
 */

#ifndef __XEN_PUBLIC_MEMORY_H__
#define __XEN_PUBLIC_MEMORY_H__

#include "xen.h"

/* Source mapping space. */
/* ` enum phys_map_space { */
#define XENMAPSPACE_shared_info  0 /* shared info page */
#define XENMAPSPACE_grant_table  1 /* grant table page */
#define XENMAPSPACE_gmfn         2 /* GMFN */
#define XENMAPSPACE_gmfn_range   3 /* GMFN range, XENMEM_add_to_physmap only. */
#define XENMAPSPACE_gmfn_foreign 4 /* GMFN from another dom,
                                    * XENMEM_add_to_physmap_batch only. */
/* ` } */

/*
 * Sets the GPFN at which a particular page appears in the specified guest's
 * pseudophysical address space.
 * arg == addr of xen_add_to_physmap_t.
 */
#define XENMEM_add_to_physmap      7
struct xen_add_to_physmap {
    /* Which domain to change the mapping for. */
    domid_t domid;

    /* Number of pages to go through for gmfn_range */
    UINT16    size;

    UINT32 space; /* => enum phys_map_space */

#define XENMAPIDX_grant_table_status 0x80000000

    /* Index into space being mapped. */
    xen_ulong_t idx;

    /* GPFN in domid where the source mapping page should appear. */
    xen_pfn_t     gpfn;
};
typedef struct xen_add_to_physmap xen_add_to_physmap_t;
DEFINE_XEN_GUEST_HANDLE(xen_add_to_physmap_t);

/*
 * Unmaps the page appearing at a particular GPFN from the specified guest's
 * pseudophysical address space.
 * arg == addr of xen_remove_from_physmap_t.
 */
#define XENMEM_remove_from_physmap      15
struct xen_remove_from_physmap {
    /* Which domain to change the mapping for. */
    domid_t domid;

    /* GPFN of the current mapping of the page. */
    xen_pfn_t     gpfn;
};
typedef struct xen_remove_from_physmap xen_remove_from_physmap_t;
DEFINE_XEN_GUEST_HANDLE(xen_remove_from_physmap_t);

#endif /* __XEN_PUBLIC_MEMORY_H__ */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
