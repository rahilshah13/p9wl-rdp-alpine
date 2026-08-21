#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <wlr/types/wlr_xwayland.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include "types.h"

struct xwayland_surface_wrapper {
    struct wlr_xwayland_surface *xsurface;
    struct wlr_scene_tree *scene_tree;
    struct server *server;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener request_activate;
    struct wl_listener request_move;
    struct wl_listener request_resize;
};

static void xwayland_surface_handle_map(struct wl_listener *listener, void *data) {
    (void)data;
    struct xwayland_surface_wrapper *xw = wl_container_of(listener, xw, map);
    
    xw->scene_tree = wlr_scene_subsurface_tree_create(&xw->server->scene->tree, xw->xsurface->surface);
    xw->xsurface->surface->data = xw->scene_tree;

    wlr_log(WLR_INFO, "Xwayland surface mapped: title='%s', class='%s' (%dx%d)",
            xw->xsurface->title ? xw->xsurface->title : "",
            xw->xsurface->class ? xw->xsurface->class : "",
            xw->xsurface->width, xw->xsurface->height);

    wlr_xwayland_surface_activate(xw->xsurface, true);
    focus_keyboard_set(&xw->server->focus, xw->xsurface->surface, FOCUS_REASON_SURFACE_MAP);
}

static void xwayland_surface_handle_unmap(struct wl_listener *listener, void *data) {
    (void)data;
    struct xwayland_surface_wrapper *xw = wl_container_of(listener, xw, unmap);

    wlr_log(WLR_INFO, "Xwayland surface unmapped");

    if (xw->scene_tree) {
        wlr_scene_node_destroy(&xw->scene_tree->node);
        xw->scene_tree = NULL;
    }
}

static void xwayland_surface_handle_destroy(struct wl_listener *listener, void *data) {
    (void)data;
    struct xwayland_surface_wrapper *xw = wl_container_of(listener, xw, destroy);

    wl_list_remove(&xw->map.link);
    wl_list_remove(&xw->unmap.link);
    wl_list_remove(&xw->destroy.link);
    wl_list_remove(&xw->request_activate.link);
    wl_list_remove(&xw->request_move.link);
    wl_list_remove(&xw->request_resize.link);

    free(xw);
}

static void xwayland_surface_handle_request_activate(struct wl_listener *listener, void *data) {
    (void)data;
    struct xwayland_surface_wrapper *xw = wl_container_of(listener, xw, request_activate);
    wlr_xwayland_surface_activate(xw->xsurface, true);
    focus_keyboard_set(&xw->server->focus, xw->xsurface->surface, FOCUS_REASON_EXPLICIT);
}

static void xwayland_surface_handle_request_move(struct wl_listener *listener, void *data) {
    (void)data;
    struct xwayland_surface_wrapper *xw = wl_container_of(listener, xw, request_move);
    // Headless / single window environments can typically ignore raw client-side window dragging commands
    (void)xw;
}

static void xwayland_surface_handle_request_resize(struct wl_listener *listener, void *data) {
    struct wlr_xwayland_surface_resize_event *event = data;
    struct xwayland_surface_wrapper *xw = wl_container_of(listener, xw, request_resize);
    wlr_xwayland_surface_configure(xw->xsurface, xw->xsurface->x, xw->xsurface->y, event->width, event->height);
}

void handle_xwayland_surface(struct wl_listener *listener, void *data) {
    struct server *s = wl_container_of(listener, s, new_xwayland_surface);
    struct wlr_xwayland_surface *xsurface = data;

    struct xwayland_surface_wrapper *xw = calloc(1, sizeof(struct xwayland_surface_wrapper));
    if (!xw) {
        wlr_log(WLR_ERROR, "Allocation failed for Xwayland surface wrapper");
        return;
    }

    xw->xsurface = xsurface;
    xw->server = s;

    xw->map.notify = xwayland_surface_handle_map;
    wl_signal_add(&xsurface->events.map, &xw->map);

    xw->unmap.notify = xwayland_surface_handle_unmap;
    wl_signal_add(&xsurface->events.unmap, &xw->unmap);

    xw->destroy.notify = xwayland_surface_handle_destroy;
    wl_signal_add(&xsurface->events.destroy, &xw->destroy);

    xw->request_activate.notify = xwayland_surface_handle_request_activate;
    wl_signal_add(&xsurface->events.request_activate, &xw->request_activate);

    xw->request_move.notify = xwayland_surface_handle_request_move;
    wl_signal_add(&xsurface->events.request_move, &xw->request_move);

    xw->request_resize.notify = xwayland_surface_handle_request_resize;
    wl_signal_add(&xsurface->events.request_resize, &xw->request_resize);
}
