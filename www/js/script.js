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

  // var bytes = new Uint8Array(evt.data);
  // var array = toNumbersArray(bytes);
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

function asInt(value) 
{
    var result = [
        (value & 0x000000ff),
        (value & 0x0000ff00) >> 8,
        (value & 0x00ff0000) >> 16,
        (value & 0xff000000) >> 24
    ];

    return result;
}

function toInt(value) 
{
    var result = 0;

    for (var i = 0; i < 4; i++) // 4 bytes for integer
    { 
        result += value[i] << (i * 8);
    }

    return result;
}

function asDouble(value) 
{
  var result = [];
  var binary = new Float64Array([value]);
  var array = new Uint8Array(binary.buffer);

  for (var i = 0; i < array.length; i++) 
  {
      result.push(array[i]);
  }

  return result;
}

function toDouble(value) 
{
  var array = new Uint8Array(value);
  return new Float64Array(array.buffer)[0];
}

function toNumbersArray(value) 
{
  var buffer = [];
  
  for (var i = 0; i < value.length; i++) 
  {
    buffer[i] = value[i];
  }

  return buffer;
}

function toNumberToByte(value) 
{  
  var arr = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 'A', 'B', 'C', 'D', 'E', 'F'];
  return arr[value >> 4] + '' + arr[num & 0xF];
}

window.addEventListener("load", init, false);
