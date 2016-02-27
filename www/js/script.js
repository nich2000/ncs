//=============================================================================
// http://getskeleton.com/
// http://getbootstrap.com/
//=============================================================================
var 
  ws,
  wsUri = 'ws://' + location.hostname + ':5800';
  wsConnected = false;

function init() 
{
  console.log("init");

  var timerConnect = setInterval(function() 
  {
    createWebSocket();
  }, 
  5000);
}

function createWebSocket() 
{
  if(wsConnected)
    return;

  console.log("createWebSocket");
  ws = new WebSocket(wsUri);
  ws.onopen = function(evt) 
  {
    onOpen(evt)
  };
  ws.onclose = function(evt) 
  {
    onClose(evt)
  };
  ws.onmessage = function(evt) 
  {
    onMessage(evt)
  };
  ws.onerror = function(evt) 
  {
    onError(evt)
  };
}

function onOpen(evt) 
{
  wsConnected = true;
  console.log("onOpen");
  $("#connection_status").text('connected');
}

function onClose(evt) 
{
  wsConnected = false;
  console.log("onClose");
  $("#connection_status").text('disconnected');
}

function onMessage(evt) 
{
  console.log(evt.data);
}

function onError(evt) 
{
  console.log(evt.data);
}

function doSend(message) 
{
  console.log("doSend: " + message);
  ws.send(message);
}

window.addEventListener("load", init, false);
