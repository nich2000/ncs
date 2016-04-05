//==============================================================================
//==============================================================================
class exec_t {
  //----------------------------------------------------------------------------
  constructor() {
    console.log("constructor: exec_t");

    Signal.bind("onMessage", this.doOnMessage, this);
  }
  //----------------------------------------------------------------------------
  public doOnMessage(message: any) {
    let data: any;
    data = JSON.parse(message);

    if (data.length == 0)
      return;

    if (data[0].CMD != undefined) {
      let cmd: string = data[0].CMD;
      $("#last_recv_cmd").text("Command: " + cmd);

      data.shift();

      Signal.emit(cmd, data);
    }
    else {
      Signal.emit("add_data", data);
    }
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
