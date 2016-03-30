//=============================================================================
/// <reference path="./jquery.d.ts"/>
//=============================================================================
var ws: web_socket_t;
// var data_table_first: data_table_t;
// var data_table_second: data_table_t;
// var clientsTable: clients_table_t;
// var map: map_t;
var clients: clients_t;
var exec: exec_t;
//=============================================================================
function init() {
  console.log("init");

  exec = new exec_t();

  ws = new web_socket_t("ws://" + location.hostname + ":5800");

  clients = new clients_t();

  // clientsTable = new clients_table_t("remote_clients", $(window), 2);

  // data_table_first = new data_table_t("remote_data_first", $(window), 2);
  // data_table_second = new data_table_t("remote_data_second", $(window), 2);

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
