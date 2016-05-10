//==============================================================================
/// <reference path="./jquery.d.ts"/>
//==============================================================================
var use_clients = 1;
var use_map = 1;
var use_ws = 1;
//==============================================================================
var clients: clients_t = undefined;
var ws: web_socket_t = undefined;
var map: map_t = undefined;
var profiler: any = undefined;
//==============================================================================
function init(): void {
  profiler = $("#profiler");

  console.log("init");

  if(use_clients)
    clients = new clients_t();
  else
    console.log("clients not enabled");

  if(use_map) {
    map = new map_t("canvas_map");
    // map.test_mode = true;
  }
  else
    console.log("map not enabled");

  if(use_ws)
    ws = new web_socket_t("ws://" + location.hostname + ":5800");
  else
    console.log("ws not enabled");

  console.log("init success");
}
//==============================================================================
function test(): void {
  // $.get("command?cmd=ws_activate&par=1&par=next",
  //   function( data ) {
  //     alert("Data Loaded: " + data);
  //   });
}
//==============================================================================
$(window).load(function() {
  // console.log("load");
  $("body").height($(window).height());
  init();
});
//==============================================================================
$(window).resize(function() {
  // console.log("resize");
  $("body").height($(window).height());
});
//==============================================================================
