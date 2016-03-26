//==============================================================================
/// <reference path="./jquery.d.ts"/>
//==============================================================================
class web_socket_t 
{
  socket: WebSocket;
  host: string;
  is_connected: boolean;

  constructor(host: string)
  {
    console.log("constructor: web_socket_t");

    this.host = host;
    this.is_connected = false;

    Signal.bind("doSend", this.doSend, this);

    var timerConnect = setInterval(() =>{
      this.createWebSocket();
    },
    3000);
  }

  public createWebSocket() 
  {
    if ((this.socket) && (this.is_connected))
      return;

    this.socket = new WebSocket(this.host);
    this.socket.onopen = (evt) => { this.onOpen(evt) };
    this.socket.onclose = (evt) => { this.onClose(evt) };
    this.socket.onmessage = (evt) => { this.onMessage(evt) };
    this.socket.onerror = (evt) => { this.onError(evt) };
  }

  doSend(message: any) 
  {
    console.log("doSend: " + message);

    message = JSON.stringify(message);

    this.socket.send(message);
  }

  onOpen(evt: any) 
  {
    console.log("onOpen");

    this.is_connected = true;

    $('#connection_status').removeClass('label-danger');
    $('#connection_status').removeClass('label-warning');
    $('#connection_status').addClass('label-success');
    $("#connection_status").text('Connection: open');
  }

  onClose(evt: any) 
  {
    console.log("onClose");

    this.is_connected = false;

    $('#connection_status').removeClass('label-danger');
    $('#connection_status').removeClass('label-success');
    $('#connection_status').addClass('label-warning');
    $("#connection_status").text('Connection: close');
  }

  onError(evt: any) 
  {
    console.log(evt.data);

    this.is_connected = false;

    $('#connection_status').removeClass('label-success');
    $('#connection_status').removeClass('label-warning');
    $('#connection_status').addClass('label-danger');
    $("#connection_status").text('Connection: error');
  }

  onMessage(evt: any) 
  {
    console.log("onMessage: " + evt.data);

    // Signal.emit('onMessage', evt.data);
  }
}
//==============================================================================
