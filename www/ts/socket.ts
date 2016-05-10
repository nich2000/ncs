//==============================================================================
/// <reference path="./jquery.d.ts"/>
//==============================================================================
class web_socket_t {
  //----------------------------------------------------------------------------
  private _socket: WebSocket = undefined;
  private _host: string = "";
  private _is_connected: boolean = false;
  private _session_id: string = "";
  private _counter: number = 0;
  private _connection_status: any = undefined;
  private _last_recv_cmd: any = undefined;
  private _last_send_cmd: any = undefined;
  private _data_counter: any = undefined;
  //----------------------------------------------------------------------------
  constructor(host: string) {
    console.log("constructor, web_socket_t");

    this._host = host;
    this._is_connected = false;

    this._connection_status = $("#connection_status");
    this._last_recv_cmd     = $("#last_recv_cmd");
    this._last_send_cmd     = $("#last_send_cmd");
    this._data_counter      = $("#data_counter");

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

    this._last_send_cmd.text("Send: " + message);

    message = JSON.stringify(message);

    this._socket.send(message);
  }
  //----------------------------------------------------------------------------
  private onOpen(evt: any) {
    console.log("onOpen");

    this._is_connected = true;
    this.session_id = String(new Date().getTime());

    this._connection_status.removeClass("label-danger");
    this._connection_status.removeClass("label-warning");
    this._connection_status.addClass("label-success");
    this._connection_status.text("Connection: open");

    Signal.emit("doSend", [["cmd", "ws_register"]]);
  }
  //----------------------------------------------------------------------------
  private onClose(evt: any) {
    console.log("onClose");

    this._is_connected = false;

    this._connection_status.removeClass("label-danger");
    this._connection_status.removeClass("label-success");
    this._connection_status.addClass("label-warning");
    this._connection_status.text("Connection: close");
  }
  //----------------------------------------------------------------------------
  private onError(evt: any) {
    console.log(evt.data);

    this._is_connected = false;

    this._connection_status.removeClass("label-success");
    this._connection_status.removeClass("label-warning");
    this._connection_status.addClass("label-danger");
    this._connection_status.text("Connection: error");
  }
  //----------------------------------------------------------------------------
  private onMessage(evt: any) {
    let data: any;
    data = JSON.parse(evt.data);

    if (data.length == 0)
      return;

    if (data[0].CMD != undefined) {
      let cmd: string = data[0].CMD;
      this._last_recv_cmd.text("Recv: " + cmd);

      data.shift();

      Signal.emit(cmd, data);
    }
    else {
      this._counter++;
      this._data_counter.text("Counter: " + this._counter);
      Signal.emit("add_data", data);
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
