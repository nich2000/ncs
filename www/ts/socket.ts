//==============================================================================
/// <reference path="./jquery.d.ts"/>
//==============================================================================
class web_socket_t {
  //----------------------------------------------------------------------------
  private _socket: WebSocket;
  private _host: string;
  private _is_connected: boolean;
  private _session_id: string;
  //----------------------------------------------------------------------------
  constructor(host: string) {
    console.log("constructor: web_socket_t");

    this._host = host;
    this._is_connected = false;

    Signal.bind("doSend", this.doSend, this);

    var timerConnect = setInterval(() => {
      this.createWebSocket();
    },
      3000);
  }
  //----------------------------------------------------------------------------
  private createWebSocket() {
    if ((this._socket) && (this._is_connected))
      return;

    this._socket = new WebSocket(this._host);
    this._socket.onopen = (evt) => { this.onOpen(evt) };
    this._socket.onclose = (evt) => { this.onClose(evt) };
    this._socket.onmessage = (evt) => { this.onMessage(evt) };
    this._socket.onerror = (evt) => { this.onError(evt) };
  }
  //----------------------------------------------------------------------------
  private doSend(message: any) {
    console.log("doSend: " + message);

    message.splice(1, 0, ["par", this.session_id]);

    $("#last_send_cmd").text("Command: " + message);

    message = JSON.stringify(message);

    this._socket.send(message);
  }
  //----------------------------------------------------------------------------
  private onOpen(evt: any) {
    console.log("onOpen");

    this._is_connected = true;
    this.session_id = String(new Date().getTime());

    $('#connection_status').removeClass('label-danger');
    $('#connection_status').removeClass('label-warning');
    $('#connection_status').addClass('label-success');
    element.set_text("connection_status", 'Connection: open');

    Signal.emit("doSend", [["cmd", "ws_register"]]);
  }
  //----------------------------------------------------------------------------
  private onClose(evt: any) {
    console.log("onClose");

    this._is_connected = false;

    $('#connection_status').removeClass('label-danger');
    $('#connection_status').removeClass('label-success');
    $('#connection_status').addClass('label-warning');
    element.set_text("connection_status", 'Connection: close');
  }
  //----------------------------------------------------------------------------
  private onError(evt: any) {
    console.log(evt.data);

    this._is_connected = false;

    $('#connection_status').removeClass('label-success');
    $('#connection_status').removeClass('label-warning');
    $('#connection_status').addClass('label-danger');
    element.set_text("connection_status", 'Connection: error');
  }
  //----------------------------------------------------------------------------
  private onMessage(evt: any) {
    // console.log("onMessage: " + evt.data);

    try{
      Signal.emit('onMessage', evt.data);
    }
    catch(e){
      console.log("onMessage: " + e.data);
    }
  }
  //----------------------------------------------------------------------------
  public get is_connected() : boolean {
    return this._is_connected;
  }
  //----------------------------------------------------------------------------
  public set is_connected(v : boolean) {
    this._is_connected = v;
  }
  //----------------------------------------------------------------------------
  public get session_id() : string {
    return this._session_id;
  }
  //----------------------------------------------------------------------------
  public set session_id(v : string) {
    this._session_id = v;
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
