bt_conn_cb_area : SUBALIGN(4) 
{ 
    _bt_conn_cb_list_start = .; 
    KEEP(*(SORT_BY_NAME(._bt_conn_cb.static.*))); 
    _bt_conn_cb_list_end = .; 
} > ROM

#ifdef BT_CONN
bt_l2cap_fixed_chan_area : SUBALIGN(4) 
{ 
    _bt_l2cap_fixed_chan_list_start = .; 
    KEEP(*(SORT_BY_NAME(._bt_l2cap_fixed_chan.static.*))); 
    _bt_l2cap_fixed_chan_list_end = .; 
} > ROM

bt_gatt_service_static_area : SUBALIGN(4) 
{ 
    _bt_gatt_service_static_list_start = .; 
    KEEP(*(SORT_BY_NAME(._bt_gatt_service_static.static.*))); 
    _bt_gatt_service_static_list_end = .; 
} > ROM
#endif

#ifdef BT_IAS
bt_ias_cb_area : SUBALIGN(4) 
{ 
        _bt_ias_cb_list_start = .; 
        KEEP(*(SORT_BY_NAME(._bt_ias_cb.static.*))); 
        _bt_ias_cb_list_end = .; 
} > ROM
#endif


#ifdef BT_MESH 
bt_mesh_subnet_cb_area : SUBALIGN(4) 
{ 
        _bt_mesh_subnet_cb_list_start = .; 
        KEEP(*(SORT_BY_NAME(._bt_mesh_subnet_cb.static.*))); 
        _bt_mesh_subnet_cb_list_end = .; 
} > ROM

bt_mesh_app_key_cb_area : SUBALIGN(4) 
{ 
        _bt_mesh_app_key_cb_list_start = .; 
        KEEP(*(SORT_BY_NAME(._bt_mesh_app_key_cb.static.*))); 
        _bt_mesh_app_key_cb_list_end = .; 
} > ROM

bt_mesh_hb_cb_area : SUBALIGN(4) 
{ 
        _bt_mesh_hb_cb_list_start = .; 
        KEEP(*(SORT_BY_NAME(._bt_mesh_hb_cb.static.*))); 
        _bt_mesh_hb_cb_list_end = .; 
} > ROM

bt_mesh_friend_cb_area : SUBALIGN(4) 
{ 
        _bt_mesh_friend_cb_list_start = .; 
        KEEP(*(SORT_BY_NAME(._bt_mesh_friend_cb.static.*))); 
        _bt_mesh_friend_cb_list_end = .; 
} > ROM


bt_mesh_proxy_cb_area : SUBALIGN(4) 
{ 
        _bt_mesh_proxy_cb_list_start = .; 
        KEEP(*(SORT_BY_NAME(._bt_mesh_proxy_cb.static.*))); 
        _bt_mesh_proxy_cb_list_end = .; 
} > ROM

#endif
 