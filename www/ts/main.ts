//=============================================================================
/// <reference path="./jquery.d.ts"/>
//=============================================================================
var exec: exec_t;
var clients: clients_t;
var ws: web_socket_t;

// var data_table_first: data_table_t;
// var data_table_second: data_table_t;
// var clientsTable: clients_table_t;
// var map: map_t;
//=============================================================================

//=============================================================================
function init() {
  console.log("init");

  exec = new exec_t();

  clients = new clients_t();

  ws = new web_socket_t("ws://" + location.hostname + ":5800");

// map = new map_t('canvas');
// map.test_draw();
}
//=============================================================================
$(window).load(function() {
  // console.log("load");
  $('body').height($(window).height());
  init();
});
//=============================================================================
$(window).resize(function() {
  // console.log("resize");
  $('body').height($(window).height());
});
//=============================================================================
