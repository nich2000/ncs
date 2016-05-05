//=============================================================================
/// <reference path="./jquery.d.ts"/>
//=============================================================================
var clients: clients_t;
var ws: web_socket_t;
var map: map_t;
var profiler: any;
//=============================================================================
function init(): void {
  profiler = $("#profiler");

  console.log("init");

  clients = new clients_t();

  // map = new map_t("canvas_map");
  // map.test_mode = true;

  ws = new web_socket_t("ws://" + location.hostname + ":5800");

  console.log("init done");
}
//=============================================================================
function test(): void {
  // $.get("command?cmd=ws_activate&par=1&par=next",
  //   function( data ) {
  //     alert("Data Loaded: " + data);
  //   });
}
//=============================================================================
//=============================================================================
$(window).load(function() {
  // console.log("load");
  $("body").height($(window).height());
  // init();
});
//=============================================================================
$(window).resize(function() {
  // console.log("resize");
  $("body").height($(window).height());
});
//=============================================================================
