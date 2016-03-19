//=============================================================================
//=============================================================================
var ws: web_socket_t;
var dataTable: data_table_t;
var clientsTable: clients_table_t;
var exec: exec_t;
//=============================================================================
function init() 
{
  console.log("init");
  exec = new exec_t();
  ws = new web_socket_t("ws://" + location.hostname + ":5800");
  clientsTable = new clients_table_t("remote_clients", 2);
  dataTable = new data_table_t("remote_data", 2);
}
//=============================================================================
window.addEventListener("load", init, false);
//=============================================================================
