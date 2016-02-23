var wsUri = "ws://127.0.0.1:5800";
var output;

// var input = document.createElement("input");
// input.type = "text";
// input.value = "qwerty";

// var button = document.createElement("button");
// button.innerText = "send";
// button.onclick = function () 
// {
//   doSend(input.value);
// }

function init()
{
  output = document.getElementById("output");

  // document.body.appendChild(button);
  // document.body.appendChild(input);

  createWebSocket();
}

function createWebSocket()
{
  websocket = new WebSocket(wsUri);
  websocket.onopen = function(evt) { onOpen(evt) };
  websocket.onclose = function(evt) { onClose(evt) };
  websocket.onmessage = function(evt) { onMessage(evt) };
  websocket.onerror = function(evt) { onError(evt) };
}

function onOpen(evt)
{
  console.log("onOpen");
  writeToScreen("onOpen");
  doSend("WS_CONNECTED");
}

function onClose(evt)
{
  console.log("onClose");
  writeToScreen("onClose");
}

function onMessage(evt)
{
  console.log(evt.data);
  writeToScreen('<span style="color: blue;">onMessage: ' + evt.data + '</span>');
}

function onError(evt)
{
  console.log(evt.data);
  writeToScreen('<span style="color: red;">onError: ' + evt.data + '</span>');
}

function doSend(message)
{
  console.log("doSend: " + message);
  writeToScreen("doSend: " + message);
  websocket.send(message);
}

function writeToScreen(message)
{
  var pre = document.createElement("p");
  pre.style.wordWrap = "break-word";
  pre.innerHTML = message;
  output.appendChild(pre);
}

window.addEventListener("load", init, false);
