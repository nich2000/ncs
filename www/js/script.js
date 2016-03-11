//=============================================================================
// http://getskeleton.com/
// http://getbootstrap.com/
//=============================================================================
var 
  ws
  wsUri = 'ws://' + location.hostname + ':5800',
  wsConnected = false;

var
  dataTable;

var
  dataCounter = 0;

function init() 
{
  console.log("init");

  dataTable = $("#remote_data");

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

  $('#connection_status').removeClass('label-danger');
  $('#connection_status').removeClass('label-warning');
  $('#connection_status').addClass('label-success');
  $("#connection_status").text('Connection: open');
}

function onClose(evt) 
{
  wsConnected = false;
  console.log("onClose");

  $('#connection_status').removeClass('label-danger');
  $('#connection_status').removeClass('label-success');
  $('#connection_status').addClass('label-warning');
  $("#connection_status").text('Connection: close');
}

function onMessage(evt) 
{
  var data;
  data = JSON.parse(evt.data);

  if(data.length == 0)
    return;

  if(data[0].length == 0)
    return;

  if(data[0][0] == "CMD")
  {
    $("#last_cmd").text("Command: " + data[0][1]);
  }
  else
  {
    dataCounter++;
    $("#data_counter").text(dataCounter);

    for(var i = 0; i < data.length; i++) 
    {
      var line = $("#" + data[i][0]);
      if(line.length == 0)
      {
        line = $("<tr></tr>");
        line.attr("id", data[i][0]);
        dataTable.append(line);
      }
      
      var key = $("#key_" + data[i][0]);
      if(key.length == 0)
      {
        key = $("<td style='height:10px'></td>");
        key.attr("id", "key_" + data[i][0]);
        line.append(key);
      }
      key.text(data[i][0]);

      var value = $("#value_" + data[i][0]);
      if(value.length == 0)
      {
        value = $("<td style='height:10px'></td>");
        value.attr("id", "value_" + data[i][0]);
        line.append(value);
      }
      value.text(data[i][1]);
    }
  }
}

function onError(evt) 
{
  wsConnected = false;
  console.log(evt.data);

  $('#connection_status').removeClass('label-success');
  $('#connection_status').removeClass('label-warning');
  $('#connection_status').addClass('label-danger');
  $("#connection_status").text('Connection: error');
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
